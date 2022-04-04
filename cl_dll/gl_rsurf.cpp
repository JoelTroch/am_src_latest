//
// written by BUzer for HL: Paranoia modification
//
//		2006

#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "com_model.h"
#include "glmanager.h"
#include "gl_texloader.h"

#include "studio_util.h"
#include "r_studioint.h"
#include "ref_params.h"
#include "gl_renderer.h"

int cubemaprendering = 0;
extern int norm_cubemap_id;

int HasDynamicLights(); // if TRUE, lightmaps should be drawn separately
static int hasdynlights;

// debug info collection
static int wpolytotal;
static int wpolybumped;
static int wpolynormal;
static int wpolyspecular;
static int wpolyspecular_hi;


msurface_t *specularchain_low = NULL;
msurface_t *specularchain_high = NULL;
msurface_t *specularchain_both = NULL;
msurface_t *skychain = NULL;
extern int no_sky_visible;
extern cl_entity_t *currententity;

// for special bump passes on hardware with <4 TU
msurface_t* lightmapchains[64];
int needs_special_bump_passes = 0;

int needs_second_pass = 0;


//int IsEntityTransparent(cl_entity_t *e);



//================================
// ARRAYS AND VBOS MANAGEMENT
//
//================================

#define OFFSET(type, variable) ((const void*)&(((type*)NULL)->variable))

struct BrushVertex
{
	vec3_t	pos;
	float	texcoord[2];
	float	lightmaptexcoord[2];
};

struct BrushFace
{
	int start_vertex;
	vec3_t	normal;
	vec3_t	s_tangent;
	vec3_t	t_tangent;
};

GLuint	buffer = 0;
BrushVertex* buffer_data = NULL;
BrushFace* faces_extradata = NULL;

int use_vertex_array = 0; // 1 - VAR, 2 - VBO


void FreeBuffer()
{
	if (buffer)
	{
		gEngfuncs.Con_DPrintf("Clearing VBO...");
		gl.glDeleteBuffersARB(1, &buffer);
		gEngfuncs.Con_DPrintf("Ok\n");
		buffer = 0;
	}

	if (buffer_data)
	{
		gEngfuncs.Con_DPrintf("Deleting vertex array\n");
		delete[] buffer_data;
		buffer_data = NULL;
	}

	if (faces_extradata)
	{
		gEngfuncs.Con_DPrintf("Deleting faces extra data\n");
		delete[] faces_extradata;
		faces_extradata = NULL;
	}
}


void GenerateVertexArray()
{
	// TODO: remember mapname!!

	gEngfuncs.Con_DPrintf("GenerateVertexArray()\n");
	FreeBuffer();

	// clear all previous errors
	GLenum err;
	do {err = gl.glGetError();} while (err != GL_NO_ERROR);

	// calculate number of used faces and vertexes
	int numfaces = 0;
	int numverts = 0;
	model_t *world = gEngfuncs.GetEntityByIndex(0)->model;
	msurface_t* surfaces = world->surfaces;
	for (int i = 0; i < world->numsurfaces; i++)
	{
		if (!(surfaces[i].flags & (SURF_DRAWSKY|SURF_DRAWTURB)))
		{
			glpoly_t *p = surfaces[i].polys;
			if (p->numverts > 0)
			{
				numfaces++;
				numverts += p->numverts;
			}
		}
	}

	gEngfuncs.Con_DPrintf("%d world polygons visible\n", numfaces);
	gEngfuncs.Con_DPrintf("%d vertexes in all world polygons\n", numverts);

	// create vertex array
	int curvert = 0;
	int curface = 0;
	buffer_data = new BrushVertex[numverts];
	faces_extradata = new BrushFace[numfaces];
	for (int i = 0; i < world->numsurfaces; i++)
	{
		if (!(surfaces[i].flags & (SURF_DRAWSKY|SURF_DRAWTURB)))
		{
			glpoly_t *p = surfaces[i].polys;
			if (p->numverts > 0)
			{
				float *v = p->verts[0];

			/*	int packed = (curvert << 16);
				packed |= (p->flags & 0xFFFF);
				p->flags = packed;

				packed = (curvert & 0xFFFF0000);
				packed |= (surfaces[i].flags & 0xFFFF);
				surfaces[i].flags = packed;*/

				// hack: pack extradata index in unused bits of poly->flags
				int packed = (curface << 16);
				packed |= (p->flags & 0xFFFF);
				p->flags = packed;

				BrushFace* ext = &faces_extradata[curface];
				ext->start_vertex = curvert;

				// store tangent space
				VectorCopy(surfaces[i].texinfo->vecs[0], ext->s_tangent);
				VectorCopy(surfaces[i].texinfo->vecs[1], ext->t_tangent);
				VectorNormalize(ext->s_tangent);
				VectorNormalize(ext->t_tangent);
				VectorCopy(surfaces[i].plane->normal, ext->normal);
				if (surfaces[i].flags & SURF_PLANEBACK)
				{
					VectorInverse(ext->normal);
				}

				for (int j=0; j<p->numverts; j++, v+= VERTEXSIZE)
				{
					buffer_data[curvert].pos[0] = v[0];
					buffer_data[curvert].pos[1] = v[1];
					buffer_data[curvert].pos[2] = v[2];
					buffer_data[curvert].texcoord[0] = v[3];
					buffer_data[curvert].texcoord[1] = v[4];
					buffer_data[curvert].lightmaptexcoord[0] = v[5];
					buffer_data[curvert].lightmaptexcoord[1] = v[6];
					curvert++;
				}
				curface++;
			}
		}
	}

	gEngfuncs.Con_DPrintf("Created Vertex Array for world polys\n");

	if (gl.ARB_VBO_supported)
	{
		gl.glGenBuffersARB(1, &buffer);
		if (buffer)
		{
			gl.glBindBufferARB(GL_ARRAY_BUFFER_ARB, buffer);
			gl.glBufferDataARB(GL_ARRAY_BUFFER_ARB, numverts*sizeof(BrushVertex), buffer_data, GL_STATIC_DRAW_ARB);
			if (gl.glGetError() != GL_OUT_OF_MEMORY)
			{
				// VBO created succesfully!!!
				gl.glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0); // remove binding
				gEngfuncs.Con_DPrintf("Created VBO for world polys\n");
			}
			else
			{
				gl.glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0); // remove binding
				gl.glDeleteBuffersARB(1, &buffer);
				buffer = 0;
				gEngfuncs.Con_Printf("Error: glBufferDataARB failed to upload data to VBO\n");
			}
		}
		else
			gEngfuncs.Con_Printf("Error: glGenBuffersARB failed to generate buffer index\n");	
	}
}

void EnableVertexArray()
{
	if (!buffer_data)
		return;

	if (buffer)
	{
		gl.glBindBufferARB(GL_ARRAY_BUFFER_ARB, buffer);

		// initialize pointer for VBO
		gl.glVertexPointer( 3, GL_FLOAT, sizeof(BrushVertex), OFFSET(BrushVertex, pos) );
		use_vertex_array = 2;
	}
	else
	{
		// initialize pointer for vertex array
		gl.glVertexPointer( 3, GL_FLOAT, sizeof(BrushVertex), &buffer_data[0].pos );
		use_vertex_array = 1;
	}

	gl.glEnableClientState(GL_VERTEX_ARRAY);	
}

void SetupOffset()
{

}

void DisableVertexArray()
{
	if (!use_vertex_array)
		return;

	if (use_vertex_array == 2)
		gl.glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

	gl.glDisableClientState(GL_VERTEX_ARRAY);

	SetTexPointer(0, TC_OFF);
	SetTexPointer(1, TC_OFF);
	SetTexPointer(2, TC_OFF);
	SetTexPointer(3, TC_OFF);
	use_vertex_array = 0;
}



//================================
// HELPER FUNCTIONS, STATE CACHES, ETC...
//
//================================


//===============
// R_TextureAnimation
// Returns the proper texture for a given time and base texture
//===============
texture_t *R_TextureAnimation (texture_t *base)
{
	int		reletive;
	int		count;

	if (currententity->curstate.frame)
	{
		if (base->alternate_anims)
			base = base->alternate_anims;
	}
	
	if ((base->name[0] != '+') || (!base->anim_total))
		return base;

	reletive = (int)(gEngfuncs.GetClientTime()*10) % base->anim_total;

	count = 0;	
	while (base->anim_min > reletive || base->anim_max <= reletive)
	{
		base = base->anim_next;
		if (!base) {
			gEngfuncs.Con_Printf("R_TextureAnimation: broken cycle"); break;}
		if (++count > 100) {
			gEngfuncs.Con_Printf("R_TextureAnimation: infinite cycle"); break;}
	}

	return base;
}

//===============
// 2D texture binding cache
//===============
static GLuint currentbinds[16];

void Bind2DTexture(GLenum texture, GLuint id)
{
	int idx = texture - GL_TEXTURE0_ARB;
	if (currentbinds[idx] != id)
	{
		gl.glActiveTextureARB( texture );
		gl.glBindTexture(GL_TEXTURE_2D, id);
		currentbinds[idx] = id;
	}
}

//===============
// TexEnv states cache
//
// only for 2D textures.
// First and second passes are pretty messy - polygon soup with different states.
//===============
static int envstates[4];

void SetTexEnv_Internal(int env)
{
	switch(env)
	{
	case ENVSTATE_OFF:
		gl.glDisable(GL_TEXTURE_2D);
	//	gl.glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1);
		break;

	case ENVSTATE_REPLACE:
		gl.glEnable(GL_TEXTURE_2D);
	//	gl.glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
	//	gl.glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_REPLACE);
	//	gl.glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_TEXTURE);
	//	gl.glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		break;

	case ENVSTATE_MUL_CONST:
		gl.glEnable(GL_TEXTURE_2D);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_MODULATE);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_CONSTANT_ARB);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_TEXTURE);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1);
		break;

	case ENVSTATE_MUL_PREV_CONST:
		gl.glEnable(GL_TEXTURE_2D);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_MODULATE);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_PREVIOUS_ARB);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_PRIMARY_COLOR_ARB);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1);
		break;

	case ENVSTATE_MUL:
		gl.glEnable(GL_TEXTURE_2D);
	/*	gl.glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_MODULATE);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_PREVIOUS_ARB);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_TEXTURE);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1);/**/
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		break;

	case ENVSTATE_MUL_X2:
		gl.glEnable(GL_TEXTURE_2D);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_MODULATE);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_PREVIOUS_ARB);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_TEXTURE);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 2);
		break;

	case ENVSTATE_ADD:
		gl.glEnable(GL_TEXTURE_2D);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_ADD);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_PREVIOUS_ARB);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_TEXTURE);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1);
		break;

	case ENVSTATE_DOT:
		gl.glEnable(GL_TEXTURE_2D);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_DOT3_RGBA_ARB);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_PREVIOUS_ARB);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_TEXTURE);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1);
		break;

	case ENVSTATE_DOT_CONST:
		gl.glEnable(GL_TEXTURE_2D);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_DOT3_RGBA_ARB);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_CONSTANT_ARB);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_TEXTURE);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1);
		break;

	case ENVSTATE_PREVCOLOR_CURALPHA:
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_REPLACE);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_PREVIOUS_ARB);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1);

		gl.glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, GL_REPLACE);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB, GL_TEXTURE);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_ARB, GL_SRC_ALPHA);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_ALPHA_SCALE, 1);

	/*	gl.glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_REPLACE);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_TEXTURE);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1);

		gl.glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, GL_REPLACE);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB, GL_PREVIOUS_ARB);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_ARB, GL_SRC_ALPHA);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_ALPHA_SCALE, 1);*/
		break;
	}
}
	
void SetTexEnvs(int env0, int env1, int env2, int env3)
{
	if (envstates[0] != env0) {
		gl.glActiveTextureARB( GL_TEXTURE0_ARB );
		SetTexEnv_Internal(env0);
		envstates[0] = env0;
	}
	if (envstates[1] != env1) {
		gl.glActiveTextureARB( GL_TEXTURE1_ARB );
		SetTexEnv_Internal(env1);
		envstates[1] = env1;
	}
	if (gl.MAX_TU_supported < 3) return;
	if (envstates[2] != env2) {
		gl.glActiveTextureARB( GL_TEXTURE2_ARB );
		SetTexEnv_Internal(env2);
		envstates[2] = env2;
	}
	if (gl.MAX_TU_supported < 4) return;
	if (envstates[3] != env3) {
		gl.glActiveTextureARB( GL_TEXTURE3_ARB );
		SetTexEnv_Internal(env3);
		envstates[3] = env3;
	}
}

//===============
// Texture coords array pointer cache
//===============
static int texpointer[4];

void SetTexPointer(int unitnum, int tc)
{
	if (unitnum >= gl.MAX_TU_supported)
		return;

	if (texpointer[unitnum] == tc)
		return;

	gl.glClientActiveTextureARB(unitnum + GL_TEXTURE0_ARB);
	
	switch(tc)
	{
	case TC_OFF:
		gl.glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		break;

	case TC_TEXTURE:
		if (use_vertex_array == 2) gl.glTexCoordPointer(2, GL_FLOAT, sizeof(BrushVertex), OFFSET(BrushVertex, texcoord));
		else gl.glTexCoordPointer(2, GL_FLOAT, sizeof(BrushVertex), &buffer_data[0].texcoord);
		gl.glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		break;

	case TC_LIGHTMAP:
		if (use_vertex_array == 2) gl.glTexCoordPointer(2, GL_FLOAT, sizeof(BrushVertex), OFFSET(BrushVertex, lightmaptexcoord));
		else gl.glTexCoordPointer(2, GL_FLOAT, sizeof(BrushVertex), &buffer_data[0].lightmaptexcoord);
		gl.glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		break;

	case TC_VERTEX_POSITION:
		if (use_vertex_array == 2) gl.glTexCoordPointer(3, GL_FLOAT, sizeof(BrushVertex), OFFSET(BrushVertex, pos));
		else gl.glTexCoordPointer(3, GL_FLOAT, sizeof(BrushVertex), &buffer_data[0].pos);
		gl.glEnableClientState(GL_TEXTURE_COORD_ARRAY);		
	}

	texpointer[unitnum] = tc;
}

//===============
// Resets all caches
//===============
void ResetCache()
{
	for(int i = 0; i < 16; i++)
		currentbinds[i] = 0;

	for (int i = 0; i < 4; i++)
	{
		envstates[i] = ENVSTATE_NOSTATE;
		texpointer[i] = TC_NOSTATE;
	}
}


//
// Makes code more clear..
//
void SetTextureMatrix(int unit, int xscale, int yscale)
{
	gl.glActiveTextureARB(unit + GL_TEXTURE0_ARB);
	gl.glMatrixMode(GL_TEXTURE);
	gl.glLoadIdentity();
	if (xscale && yscale)
		gl.glScalef(xscale, yscale, 1.0);
	
	gl.glMatrixMode(GL_MODELVIEW);
}

ExtTextureData* GetExtTexData (texture_t *tex)
{
	if (tex->anim_min == -666) return (ExtTextureData*)tex->anim_next;
	return NULL;
}

void AddPolyToSpecularChain(msurface_t *s)
{
	ExtTextureData *pExtData = GetExtTexData (s->texinfo->texture);
	if (pExtData->gl_glossmap_id)
	{
		if (pExtData->gl_extra_glossmap_id)
		{
			s->texturechain = specularchain_both;
			specularchain_both = s;
		}
		else
		{
			s->texturechain = specularchain_low;
			specularchain_low = s;
		}
	}
	else
	{
		s->texturechain = specularchain_high;
		specularchain_high = s;
	}
}

void AddPolyToTextureChain(msurface_t *s)
{
	needs_second_pass = 1;
	texture_t *tex = s->texinfo->texture;
	s->texturechain = tex->texturechain;
	tex->texturechain = s;
}

void AddPolyToSkyChain(msurface_t *s)
{
	s->texturechain = skychain;
	skychain = s;
}

 // used for additional bump passes, if hardware cant draw bump lighting in one pass
void AddPolyToLightmapChain(msurface_t *s)
{
	needs_special_bump_passes = 1;
	s->texturechain = lightmapchains[s->lightmaptexturenum];
	lightmapchains[s->lightmaptexturenum] = s;
}



//================================================
// WPOLY RENDERING
//
//================================================


//================================
// Called from main renderer for each brush entity (including world)
// Allows us to clear lists, initialize counters, etc..
//================================
void PrepareFirstPass()
{
	gl.glDisable(GL_BLEND);
	gl.glDepthMask(GL_TRUE);
	gl.glDepthFunc(GL_LEQUAL);
	specularchain_low = NULL;
	specularchain_both = NULL;
	specularchain_high = NULL;
	skychain = NULL;
	needs_special_bump_passes = 0;
	needs_second_pass = 0;
	hasdynlights = HasDynamicLights();
	for (int i = 0; i < 64; i++)
		lightmapchains[i] = NULL;

	ResetCache();
	SetTexPointer(0, TC_LIGHTMAP);
	SetTexPointer(1, TC_TEXTURE);
	SetTexPointer(3, TC_LIGHTMAP);
	// in first pass, unit 2 can be either with lightmap, either detail texture

	// reset texture chains
	model_t* world = gEngfuncs.GetEntityByIndex(0)->model;
	texture_t** tex = (texture_t**)world->textures;
	for (int i = 0; i < world->numtextures; i++)
	{
		if (tex[i])
			tex[i]->texturechain = NULL;
	}
}


//================================
// Draws poly from vertex array or VBO
//================================
void DrawPolyFromArray (glpoly_t *p)
{
	int facedataindex = (p->flags >> 16) & 0xFFFF;
	BrushFace* ext = &faces_extradata[facedataindex];
	gl.glDrawArrays(GL_POLYGON, ext->start_vertex, p->numverts);	
}

void DrawPolyFromArray (msurface_t *s)
{
	DrawPolyFromArray(s->polys);
}


//============================
// First pass for poly. (Called from main renderer)
// poly added to appropriate lists for next passes, if requed
// (specular, lightmap, texture)
//============================
void DrawNormalPoly (msurface_t *s);
void DrawBumpedPoly (msurface_t *s);
#include "water.h"

void DrawPolyFirstPass (msurface_t *s)
{
	if (s->flags & SURF_DRAWTURB && !cubemaprendering) // water
	{
		gWater.AddToChain(s);
		return;
	}

	if (s->flags & SURF_DRAWSKY)
	{
		AddPolyToSkyChain(s);
		return;
	}

	if (s->flags & SURF_BUMPREADY && cv_bump->value)
		DrawBumpedPoly(s);
	else
		DrawNormalPoly(s);

	wpolytotal++;
}


void DrawNormalPoly (msurface_t *s)
{
	texture_t *t = R_TextureAnimation(s->texinfo->texture);
	wpolynormal++;

	// bind lightmap - it will be drawn anyway
	Bind2DTexture( GL_TEXTURE0_ARB, s->lightmaptexturenum + g_lightmap_baselight );

	// draw only lightmap if scene contains dynamic lights
	if (hasdynlights)
	{
		if (currententity->curstate.rendermode != kRenderNormal)
		{
			// drawing "solid" textures in multiple passes is a bit tricky -
			// need to render only lightmap, but with holes at that places,
			// where original texture has holes
		//	Bind2DTexture( GL_TEXTURE1_ARB, t->gl_texturenum);
		//	SetTexEnvs( ENVSTATE_REPLACE, ENVSTATE_PREVCOLOR_CURALPHA );

			// YES, I KNOW, this is completely stupid to switch states when
			// rendering only one polygon!
			// I should render whole entity two times, instead of
			// rendering two times each poly, but i'm too lazy
			gl.glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
			SetTexEnvs(ENVSTATE_REPLACE, ENVSTATE_REPLACE);
			Bind2DTexture( GL_TEXTURE1_ARB, t->gl_texturenum);
			DrawPolyFromArray(s);
			gl.glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
			gl.glDepthFunc(GL_EQUAL);
			SetTexEnvs(ENVSTATE_REPLACE);
			DrawPolyFromArray(s);
			gl.glDepthFunc(GL_LEQUAL);
			AddPolyToTextureChain(s);
			return;
		}
		else
		{
			SetTexEnvs( ENVSTATE_REPLACE );
		}
		DrawPolyFromArray(s);

		// TEST
	//	if (currententity->curstate.rendermode == kRenderNormal)
		AddPolyToTextureChain(s);
		return;
	}

	ExtTextureData *pExtData = GetExtTexData (t);
	if (pExtData && pExtData->gl_detailtex_id && cv_detailtex->value)
	{
		// face has a detail texture
		if (gl.MAX_TU_supported == 2)
		{
			// draw only lightmap, texture and detail will be added by second pass
			SetTexEnvs( ENVSTATE_REPLACE );
			DrawPolyFromArray(s);
			AddPolyToTextureChain(s);
			return;
		}

		SetTexEnvs(ENVSTATE_REPLACE, ENVSTATE_MUL_X2, ENVSTATE_MUL_X2);

		SetTexPointer( 2, TC_TEXTURE );
		SetTextureMatrix( 2, pExtData->detail_xscale, pExtData->detail_yscale );
		Bind2DTexture( GL_TEXTURE1_ARB, t->gl_texturenum);
		Bind2DTexture( GL_TEXTURE2_ARB, pExtData->gl_detailtex_id);		
	}
	else
	{
		//no detail tex, just multiply lightmap and texture
		SetTexEnvs(ENVSTATE_REPLACE, ENVSTATE_MUL_X2);
		Bind2DTexture( GL_TEXTURE1_ARB, t->gl_texturenum);
	}

	DrawPolyFromArray(s);

	if (s->flags & SURF_SPECULAR)
		AddPolyToSpecularChain(s);
}


void DrawBumpedPoly (msurface_t *s)
{
	texture_t	*t = s->texinfo->texture;
	wpolybumped++;

	ExtTextureData *pExtData = GetExtTexData (t);
	Bind2DTexture( GL_TEXTURE0_ARB, s->lightmaptexturenum + g_lightmap_lightvecs);
	Bind2DTexture( GL_TEXTURE1_ARB, pExtData->gl_normalmap_id);

	if (gl.MAX_TU_supported >= 4)
	{
		SetTexEnvs(ENVSTATE_REPLACE, ENVSTATE_DOT, ENVSTATE_MUL, ENVSTATE_ADD);
		SetTexPointer( 2, TC_LIGHTMAP );
		SetTextureMatrix( 2, 0, 0 );
		Bind2DTexture( GL_TEXTURE2_ARB, s->lightmaptexturenum + g_lightmap_addlight);
		Bind2DTexture( GL_TEXTURE3_ARB, s->lightmaptexturenum + g_lightmap_baselight);
		AddPolyToTextureChain(s);
	}
	else if (gl.MAX_TU_supported == 3)
	{
		SetTexEnvs(ENVSTATE_REPLACE, ENVSTATE_DOT, ENVSTATE_MUL);
		SetTexPointer( 2, TC_LIGHTMAP );
		SetTextureMatrix( 2, 0, 0 );
		Bind2DTexture( GL_TEXTURE2_ARB, s->lightmaptexturenum + g_lightmap_addlight);
		AddPolyToLightmapChain(s);
	}
	else if (gl.MAX_TU_supported == 2)
	{
		SetTexEnvs(ENVSTATE_REPLACE, ENVSTATE_DOT);
		AddPolyToLightmapChain(s);		
	}

	DrawPolyFromArray(s);
}



//============================
// Renders additional bump passes for hardware with <4 TU
// Called from main renderer before dynamic light will be added.
// Polys added to texture chain for second pass
//============================
void RenderAdditionalBumpPass()
{
	// reset texture matrix at 3rd TU after first pass
	SetTextureMatrix( 2, 0, 0 );

	if (!needs_special_bump_passes)
		return;

	gl.glEnable(GL_BLEND);
	gl.glDepthMask(GL_FALSE);
	gl.glDepthFunc(GL_EQUAL);
	SetTexEnvs( ENVSTATE_REPLACE );
	SetTexPointer(0, TC_LIGHTMAP);

	if (gl.MAX_TU_supported == 2)
	{
		// multiply by diffuse light
		gl.glBlendFunc(GL_ZERO, GL_SRC_COLOR);
		for (int i = 0; i < 64; i++)
		{
			msurface_t *surf = lightmapchains[i];
			if (surf)
			{
				Bind2DTexture( GL_TEXTURE0_ARB, i + g_lightmap_addlight);
				while (surf)
				{
					DrawPolyFromArray(surf);
					surf = surf->texturechain;
				}
			}
		}
	}

	// add ambient light
	gl.glBlendFunc(GL_ONE, GL_ONE);
	for (int i = 0; i < 64; i++)
	{
		msurface_t *surf = lightmapchains[i];
		if (surf)
		{
			Bind2DTexture( GL_TEXTURE0_ARB, i + g_lightmap_baselight);
			while (surf)
			{
				DrawPolyFromArray(surf);
				msurface_t *next = surf->texturechain;
				AddPolyToTextureChain(surf);
				surf = next;
			}
		}
	}
}


//============================
// Renders texture chains.
// Called from main renderer after all lighting rendered.
// Specular polys added to specular chain.
//============================
void RenderSecondPass()
{
	if (!needs_second_pass)
		return;

	gl.glEnable(GL_BLEND);
	gl.glDepthMask(GL_FALSE);
	gl.glDepthFunc(GL_EQUAL);
	gl.glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);

	SetTexPointer(0, TC_TEXTURE);
	SetTexPointer(1, TC_TEXTURE);

	model_t* world = gEngfuncs.GetEntityByIndex(0)->model;
	texture_t** tex = (texture_t**)world->textures;
	for (int i = 0; i < world->numtextures; i++)
	{
		if (!tex[i])
			continue;

		msurface_t *s = tex[i]->texturechain;
		if (s)
		{
			Bind2DTexture( GL_TEXTURE0_ARB, tex[i]->gl_texturenum );
			while(s)
			{
				ExtTextureData *pExtData = GetExtTexData (tex[i]);
				if (pExtData && pExtData->gl_detailtex_id && cv_detailtex->value)
				{
					// has detail texture
					SetTexEnvs(ENVSTATE_REPLACE, ENVSTATE_MUL_X2);
					SetTextureMatrix( 1, pExtData->detail_xscale, pExtData->detail_yscale );
					Bind2DTexture( GL_TEXTURE1_ARB, pExtData->gl_detailtex_id );
				}
				else
				{
					SetTexEnvs(ENVSTATE_REPLACE);
				}

				DrawPolyFromArray(s);
				msurface_t *next = s->texturechain;
				if (s->flags & SURF_SPECULAR)
					AddPolyToSpecularChain(s);
				s = next;
			}
		}
	}

	SetTextureMatrix( 1, 0, 0 );
}




// ============================
// Specular lighting render
//
// ============================

void DrawPoly_SimpleSpecular (msurface_t *s);
void DrawLowQualitySpecularChain(int mode);
void DrawHighQualitySpecularChain();


// shader version for ATI cards. NV will use register combiners
static char specular_fp[] =
"!!ARBfp1.0\n"
"OPTION ARB_precision_hint_fastest;\n"
"ATTRIB tex0 = fragment.texcoord[0];\n"
"ATTRIB tex1 = fragment.texcoord[1];\n"
"ATTRIB tex2 = fragment.texcoord[2];\n"
"ATTRIB tex3 = fragment.texcoord[3];\n"
"PARAM scaler = { 4, 4, 2, -1 };\n"
"OUTPUT outColor = result.color;\n"

"TEMP eyevec, lightvec;\n"
"TEMP specdot, col, res;\n"

"TEX eyevec, tex0, texture[0], CUBE;\n"
"MAD eyevec.rgb, eyevec, scaler.b, scaler.a;\n"

"TEX lightvec, tex1, texture[1], 2D;\n"
"MAD lightvec.rgb, lightvec, scaler.b, scaler.a;\n"

"DP3_SAT specdot.a, eyevec, lightvec;\n"
"POW specdot.a, specdot.a, scaler.r;\n"

"TEX col, tex2, texture[2], 2D;\n" // get specular color from lightmap
"TEX res, tex3, texture[3], 2D;\n" // get color from glossmap
"MUL res.rgb, col, res;\n"

"MUL_SAT outColor, res, specdot.a;\n"
"END";

GLuint	specular_fp_id = -1;


// High quality specular shader
static char specular_fp_high[] =
"!!ARBfp1.0\n"
"OPTION ARB_precision_hint_fastest;\n"
"ATTRIB tex_camdir = fragment.texcoord[0];\n" // camera dir
"ATTRIB tex_lightmap = fragment.texcoord[1];\n" // lightmap texcoord
"ATTRIB tex_face = fragment.texcoord[3];\n" // face texture texcoord
"PARAM scaler = { 16, 8, 2, -1 };\n"
"OUTPUT outColor = result.color;\n"

"TEMP vec, halfvec;\n"
"TEMP specdot, color;\n"

// expand light vector
"TEX vec, tex_lightmap, texture[1], 2D;\n"
"MAD vec.rgb, vec, scaler.b, scaler.a;\n"

// normalize vector to camera
"DP3 halfvec.x, tex_camdir, tex_camdir;\n"
"RSQ halfvec.x, halfvec.x;\n"
"MUL halfvec, tex_camdir, halfvec.x;\n"

// add light vector and renormalize
"ADD halfvec, halfvec, vec;\n"

"DP3 halfvec.w, halfvec, halfvec;\n"
"RSQ halfvec.w, halfvec.w;\n"
"MUL halfvec.xyz, halfvec, halfvec.w;\n"

// expand normal vector
"TEX vec, tex_face, texture[0], 2D;\n"
"MAD vec.rgb, vec, scaler.b, scaler.a;\n"

// calc specular factor
"DP3_SAT specdot.a, vec, halfvec;\n"
"POW specdot.a, specdot.a, scaler.r;\n"

// multiply by specular color
"TEX color, tex_lightmap, texture[2], 2D;\n"
"MUL_SAT color, color, specdot.a;\n"

// multiply by gloss map
"TEX vec, tex_face, texture[3], 2D;\n"
"MUL outColor, color, vec;\n"
"END";

GLuint	specular_high_fp_id = -1;

//
// Fragment programs uploading...
//
GLboolean LoadProgramNative(GLenum target, GLenum format, GLsizei len, const char *string)
{
	GLint errorPos, isNative;
	gl.glProgramStringARB(target, format, len, string);
	gl.glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorPos);
	gl.glGetProgramivARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB, &isNative);
	if ((errorPos == -1) && (isNative == 1))
		return GL_TRUE;
	else
	{
		char buf[64];
		gEngfuncs.Con_Printf("Failed to load fragment program. errorPos %d, isNative %d\n", errorPos, isNative);
		memcpy(buf, &string[errorPos], 63);
		buf[63] = 0;
		gEngfuncs.Con_Printf("------------\n%s\n------------\n", buf);
		return GL_FALSE;
	}
}

//
// prepare texture matrix for converting camera position to
// tangent-space vector to it
//
void SetupTexMatrix_Reflected( msurface_t *s, const vec3_t &vec )
{
	int facedataindex = (s->polys->flags >> 16) & 0xFFFF;
	BrushFace* ext = &faces_extradata[facedataindex];

	float m[16];
	m[0] = ext->s_tangent[0];
	m[4] = ext->s_tangent[1];
	m[8] = ext->s_tangent[2];
	m[12] = -DotProduct(ext->s_tangent, vec);
	m[1] = ext->t_tangent[0];
	m[5] = ext->t_tangent[1];
	m[9] = ext->t_tangent[2];
	m[13] = -DotProduct(ext->t_tangent, vec);
	m[2] = -ext->normal[0];
	m[6] = -ext->normal[1];
	m[10] = -ext->normal[2];
	m[14] = DotProduct(ext->normal, vec);
	m[3] = m[7] = m[11] = 0;
	m[15] = 1;

	gl.glActiveTextureARB(GL_TEXTURE0_ARB);
	gl.glMatrixMode(GL_TEXTURE);
	gl.glLoadMatrixf(m);
}

void SetupTexMatrix( msurface_t *s, const vec3_t &vec )
{
	int facedataindex = (s->polys->flags >> 16) & 0xFFFF;
	BrushFace* ext = &faces_extradata[facedataindex];

	float m[16];
	m[0] = -ext->s_tangent[0];
	m[4] = -ext->s_tangent[1];
	m[8] = -ext->s_tangent[2];
	m[12] = DotProduct(ext->s_tangent, vec);
	m[1] = -ext->t_tangent[0];
	m[5] = -ext->t_tangent[1];
	m[9] = -ext->t_tangent[2];
	m[13] = DotProduct(ext->t_tangent, vec);
	m[2] = -ext->normal[0];
	m[6] = -ext->normal[1];
	m[10] = -ext->normal[2];
	m[14] = DotProduct(ext->normal, vec);
	m[3] = m[7] = m[11] = 0;
	m[15] = 1;

	gl.glActiveTextureARB(GL_TEXTURE0_ARB);
	gl.glMatrixMode(GL_TEXTURE);
	gl.glLoadMatrixf(m);
}

//
// merges surface chains
//
void MergeChains( msurface_t* &dst, msurface_t* src )
{
	if (!dst) {dst = src; return;}

	msurface_t *last = dst;
	while(1)
	{
		if (!last->texturechain) {
			last->texturechain = src;
			break;
		}
		last = last->texturechain;
	}
}



void PrintSpecularSwitchedMsg(int mode)
{
	static int oldmode = 0;
	if (mode != oldmode)
	{
		gEngfuncs.Con_Printf("Simple specular uses mode %d\n", mode);
		oldmode = mode;
	}
}

extern cvar_t *cv_speculartwopass;

// specular modes are:
//
// 1 - combiners, 4 TU
// 2 - combiners, 2 TU, alpha (2 pass)
// 3 - ARB shader, 4 IU
// 4 - env_dot3, 2 TU, alpha (3 pass)

int ChooseSpecularMode()
{
	static int ARB_vp_failed = 0;

	// try to use register combiners
	if (!cv_specular_nocombiners->value)
	{
		if (gl.NV_combiners_supported && gl.MAX_NV_combiners >= 2)
		{
			if (gl.MAX_TU_supported >= 4 && !cv_speculartwopass->value)
			{
				PrintSpecularSwitchedMsg(1);
				return 1;
			}

			if (gl.alphabits)
			{
				PrintSpecularSwitchedMsg(2);
				return 2;
			}
			else
			{
				ONCE( gEngfuncs.Con_Printf("WARNING: Alpha buffer is not present! Specular will reque more passes!\n"); );
			}
		}
	}

	// try to use ARB shaders
	if (gl.ARB_fragment_program_supported && gl.fp_max_image_units >= 4 && gl.fp_max_texcoords >= 4 && !ARB_vp_failed && !cv_specular_noshaders->value)
	{
		if (specular_fp_id != -1)
		{
			PrintSpecularSwitchedMsg(3);
			return 3;
		}
		else
		{
			gl.glEnable(GL_FRAGMENT_PROGRAM_ARB);
			gl.glGenProgramsARB(1, &specular_fp_id);
			gl.glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, specular_fp_id);
			if (LoadProgramNative(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, sizeof(specular_fp)-1, specular_fp))
			{
				gl.glDisable(GL_FRAGMENT_PROGRAM_ARB);
				gEngfuncs.Con_DPrintf("Low quality specular fragment program loaded succesfully\n");
				PrintSpecularSwitchedMsg(3);
				return 3;
			}
			else
			{
				gl.glDisable(GL_FRAGMENT_PROGRAM_ARB);
				specular_fp_id = -1;
				ARB_vp_failed = 1;
				gEngfuncs.Con_DPrintf("Failed to load low quality specular fragment program\n");
			}			
		}
	}

	// last chance: try to use 3-pass method (only alpha buffer requed)
	if (gl.alphabits)
	{
		PrintSpecularSwitchedMsg(4);
		return 4;
	}

	ONCE( gEngfuncs.Con_Printf("ERROR: specular failed!\n"); );
	return 0;
}

//===========================
// Specular rendering
//===========================
void RenderSpecular()
{
	if (!cv_specular->value)
		return;

	//
	// load low quality specular
	//
	int mode = ChooseSpecularMode();
	if (!mode)
	{
		gEngfuncs.Cvar_SetValue("gl_specular", 0);
		return;
	}

	//
	// load high quality specular
	//
	if (cv_highspecular->value)
	{
		if (gl.ARB_fragment_program_supported && gl.fp_max_image_units >= 4 && gl.fp_max_texcoords >= 4)
		{
			if (specular_high_fp_id == -1)
			{
				gl.glEnable(GL_FRAGMENT_PROGRAM_ARB);
				gl.glGenProgramsARB(1, &specular_high_fp_id);
				gl.glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, specular_high_fp_id);
				if (LoadProgramNative(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, sizeof(specular_fp_high) - 1, specular_fp_high))
					gEngfuncs.Con_Printf("High quality specular fragment program loaded succesfully\n");
				else
				{
					specular_high_fp_id = -1;
					gEngfuncs.Cvar_SetValue("gl_highspecular", 0);
				}
				gl.glDisable(GL_FRAGMENT_PROGRAM_ARB);
			}
		}
		else
		{
			gEngfuncs.Con_Printf("High quality specular is not supported by your videocard\n");
			gEngfuncs.Cvar_SetValue("gl_highspecular", 0);
		}
	}

	if (!specularchain_low && !specularchain_high && !specularchain_both)
		return;

	// surfaces that can be drawn with low and high quality specular
	// needs to be attached to one of those specular chains
	if (specularchain_both)
	{
		if (cv_highspecular->value)
			MergeChains(specularchain_high, specularchain_both);
		else
			MergeChains(specularchain_low, specularchain_both);
	}

	//	ResetCache();

	gl.glEnable(GL_BLEND);
	gl.glDepthMask(GL_FALSE);
	gl.glDepthFunc(GL_EQUAL);

	if (specularchain_low)
		DrawLowQualitySpecularChain(mode);

	if (specularchain_high && cv_highspecular->value)
	{
		DrawHighQualitySpecularChain();
		DrawHighQualitySpecularChain();
	//	DrawHighQualitySpecularChain();
	//	DrawHighQualitySpecularChain();
	//	DrawHighQualitySpecularChain();
	}

	// reset texture matrix
	gl.glActiveTextureARB(GL_TEXTURE0_ARB);
	gl.glMatrixMode(GL_TEXTURE);
	gl.glLoadIdentity();
	gl.glMatrixMode(GL_MODELVIEW);

	gl.glDisable(GL_BLEND);
	gl.glDepthMask(GL_TRUE);
}


//===========================
// Low quality specular
//
// TODO: make this shit code more clear! hate this combiners setup code..
//===========================
void DrawLowQualitySpecularChain(int mode)
{

	SetTexPointer(0, TC_VERTEX_POSITION); // Use vertex position as texcoords to calc vector to camera
	SetTexPointer(1, TC_LIGHTMAP);

	gl.glActiveTextureARB( GL_TEXTURE0_ARB );
	gl.glEnable(GL_TEXTURE_CUBE_MAP_ARB);
	gl.glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, norm_cubemap_id);

	// ===========================================================
	if (mode == 2)
	{
		// Draw specular in two pass using register combiners
		SetTexEnvs(ENVSTATE_REPLACE, ENVSTATE_REPLACE);
		gl.glEnable ( GL_REGISTER_COMBINERS_NV );
		gl.glCombinerParameteriNV ( GL_NUM_GENERAL_COMBINERS_NV, 2 );

		// First pass - write dot^4 to alpha

		// RC 0 setup:
		//	spare0 = dot(tex0, tex1)
		gl.glCombinerInputNV ( GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_A_NV, GL_TEXTURE0_ARB,
							   GL_EXPAND_NORMAL_NV, GL_RGB );
		gl.glCombinerInputNV ( GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_B_NV, GL_TEXTURE1_ARB,
							   GL_EXPAND_NORMAL_NV, GL_RGB );
		gl.glCombinerOutputNV ( GL_COMBINER0_NV, GL_RGB,
								GL_SPARE0_NV,          // AB output
								GL_DISCARD_NV,         // CD output
								GL_DISCARD_NV,         // sum output
								GL_NONE, GL_NONE,
								GL_TRUE,               // AB = A dot B
								GL_FALSE, GL_FALSE );

		// RC 1 setup:
		//	spare0 = spare0 ^ 2
		gl.glCombinerInputNV ( GL_COMBINER1_NV, GL_RGB, GL_VARIABLE_A_NV, GL_SPARE0_NV,
							   GL_UNSIGNED_IDENTITY_NV, GL_RGB );
		gl.glCombinerInputNV ( GL_COMBINER1_NV, GL_RGB, GL_VARIABLE_B_NV, GL_SPARE0_NV,
							   GL_UNSIGNED_IDENTITY_NV, GL_RGB );
		gl.glCombinerOutputNV ( GL_COMBINER1_NV, GL_RGB,
								GL_SPARE0_NV,          // AB output
								GL_DISCARD_NV,         // CD output
								GL_DISCARD_NV,         // sum output
								GL_NONE, GL_NONE,
								GL_FALSE, GL_FALSE, GL_FALSE );

		// Final RC setup:
		//	out.a = spare0.b
		gl.glFinalCombinerInputNV ( GL_VARIABLE_A_NV, GL_ZERO,
									GL_UNSIGNED_IDENTITY_NV, GL_RGB );
		gl.glFinalCombinerInputNV ( GL_VARIABLE_B_NV, GL_ZERO,
									GL_UNSIGNED_IDENTITY_NV, GL_RGB );
		gl.glFinalCombinerInputNV ( GL_VARIABLE_C_NV, GL_ZERO,
									GL_UNSIGNED_IDENTITY_NV, GL_RGB );
		gl.glFinalCombinerInputNV ( GL_VARIABLE_D_NV, GL_ZERO,
									GL_UNSIGNED_IDENTITY_NV, GL_RGB );
		gl.glFinalCombinerInputNV ( GL_VARIABLE_E_NV, GL_ZERO,
									GL_UNSIGNED_IDENTITY_NV, GL_RGB );
		gl.glFinalCombinerInputNV ( GL_VARIABLE_F_NV, GL_ZERO,
									GL_UNSIGNED_IDENTITY_NV, GL_RGB );
		gl.glFinalCombinerInputNV ( GL_VARIABLE_G_NV, GL_SPARE0_NV,
									GL_UNSIGNED_IDENTITY_NV, GL_BLUE );

		gl.glBlendFunc(GL_SRC_ALPHA, GL_ZERO); // multiply alpha by itself
		gl.glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE); // render only alpha

		msurface_t* surf = specularchain_low;
		while (surf)
		{
			Bind2DTexture( GL_TEXTURE1_ARB, surf->lightmaptexturenum + g_lightmap_lightvecs);
			SetupTexMatrix_Reflected(surf, vec_to_eyes);
			DrawPolyFromArray(surf);

			surf = surf->texturechain;
		}

		// Second pass: add gloss map and specular color

		gl.glActiveTextureARB( GL_TEXTURE0_ARB );
		gl.glDisable(GL_TEXTURE_CUBE_MAP_ARB);
		
		// reset texture matrix
		gl.glActiveTextureARB(GL_TEXTURE0_ARB);
		gl.glMatrixMode(GL_TEXTURE);
		gl.glLoadIdentity();
		gl.glMatrixMode(GL_MODELVIEW);

		gl.glBlendFunc(GL_DST_ALPHA, GL_ONE); // alpha additive
		gl.glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE); // render RGB
		SetTexPointer(0, TC_TEXTURE);

		gl.glCombinerParameteriNV ( GL_NUM_GENERAL_COMBINERS_NV, 1 );

		// RC 0 setup:
		//	spare0 = tex0 * tex1
		gl.glCombinerInputNV ( GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_A_NV, GL_TEXTURE0_ARB,
							   GL_UNSIGNED_IDENTITY_NV, GL_RGB );
		gl.glCombinerInputNV ( GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_B_NV, GL_TEXTURE1_ARB,
							   GL_UNSIGNED_IDENTITY_NV, GL_RGB );
		gl.glCombinerOutputNV ( GL_COMBINER0_NV, GL_RGB,
								GL_SPARE0_NV,          // AB output
								GL_DISCARD_NV,         // CD output
								GL_DISCARD_NV,         // sum output
								GL_NONE, GL_NONE,
								GL_FALSE,               // AB = A * B
								GL_FALSE, GL_FALSE );

		// Final RC setup:
		//	out = spare0
		gl.glFinalCombinerInputNV ( GL_VARIABLE_A_NV, GL_ZERO,
									GL_UNSIGNED_IDENTITY_NV, GL_RGB );
		gl.glFinalCombinerInputNV ( GL_VARIABLE_B_NV, GL_ZERO,
									GL_UNSIGNED_IDENTITY_NV, GL_RGB );
		gl.glFinalCombinerInputNV ( GL_VARIABLE_C_NV, GL_ZERO,
									GL_UNSIGNED_IDENTITY_NV, GL_RGB );
		gl.glFinalCombinerInputNV ( GL_VARIABLE_D_NV, GL_SPARE0_NV,
									GL_UNSIGNED_IDENTITY_NV, GL_RGB );
		gl.glFinalCombinerInputNV ( GL_VARIABLE_E_NV, GL_ZERO,
									GL_UNSIGNED_IDENTITY_NV, GL_RGB );
		gl.glFinalCombinerInputNV ( GL_VARIABLE_F_NV, GL_ZERO,
									GL_UNSIGNED_IDENTITY_NV, GL_RGB );
		gl.glFinalCombinerInputNV ( GL_VARIABLE_G_NV, GL_ZERO,
									GL_UNSIGNED_IDENTITY_NV, GL_ALPHA );

		surf = specularchain_low;
		while (surf)
		{
			ExtTextureData *pExtData = GetExtTexData (surf->texinfo->texture);
			Bind2DTexture( GL_TEXTURE0_ARB, pExtData->gl_glossmap_id);
			Bind2DTexture( GL_TEXTURE1_ARB, surf->lightmaptexturenum + g_lightmap_addlight);
			DrawPolyFromArray(surf);
			wpolyspecular++;

			surf = surf->texturechain;
		}

		gl.glDisable( GL_REGISTER_COMBINERS_NV );
		return;
	}

	// ===========================================================
	if (mode == 4)
	{
		// draw specular in three pass using only env_dot3

		// first pass - write dot^2 to alpha
		gl.glBlendFunc(GL_SRC_ALPHA, GL_ZERO); // multiply alpha by itself
		gl.glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE); // render only alpha
		SetTexEnvs(ENVSTATE_REPLACE, ENVSTATE_DOT);

		msurface_t* surf = specularchain_low;
		while (surf)
		{
			Bind2DTexture( GL_TEXTURE1_ARB, surf->lightmaptexturenum + g_lightmap_lightvecs);
			SetupTexMatrix_Reflected(surf, vec_to_eyes);
			DrawPolyFromArray(surf);

			surf = surf->texturechain;
		}

		// Second pass: square alpha values in alpha buffer

		gl.glActiveTextureARB( GL_TEXTURE0_ARB );
		gl.glDisable(GL_TEXTURE_CUBE_MAP_ARB);
		
		// reset texture matrix
		gl.glActiveTextureARB(GL_TEXTURE0_ARB);
		gl.glMatrixMode(GL_TEXTURE);
		gl.glLoadIdentity();
		gl.glMatrixMode(GL_MODELVIEW);

		gl.glBlendFunc(GL_ZERO, GL_DST_ALPHA);
		SetTexEnvs(ENVSTATE_OFF); // dont need textures at all
		SetTexPointer(0, TC_OFF);
		SetTexPointer(1, TC_OFF);

		surf = specularchain_low;
		while (surf)
		{
			DrawPolyFromArray(surf);
			surf = surf->texturechain;
		}

		// third pass: add gloss map and specular color
		gl.glBlendFunc(GL_DST_ALPHA, GL_ONE); // alpha additive
		gl.glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE); // render RGB
		SetTexEnvs(ENVSTATE_REPLACE, ENVSTATE_MUL);
		SetTexPointer(0, TC_TEXTURE);
		SetTexPointer(1, TC_LIGHTMAP);

		surf = specularchain_low;
		while (surf)
		{
			ExtTextureData *pExtData = GetExtTexData (surf->texinfo->texture);
			Bind2DTexture( GL_TEXTURE0_ARB, pExtData->gl_glossmap_id);
			Bind2DTexture( GL_TEXTURE1_ARB, surf->lightmaptexturenum + g_lightmap_addlight);
			DrawPolyFromArray(surf);
			wpolyspecular++;

			surf = surf->texturechain;
		}

		return;
	}

	// ===========================================================
	if (mode == 1 || mode == 3)
	{
		// Draw specular in one pass
		SetTexEnvs(ENVSTATE_REPLACE, ENVSTATE_REPLACE, ENVSTATE_REPLACE, ENVSTATE_REPLACE);

		// texture 0 - normalization cubemap / normal map (for HQ specular)
		// texture 1 - lightmap with light vectors
		// texture 2 - lightmap with specular color (diffuse light)
		// texture 3 - gloss map

		SetTexPointer(2, TC_LIGHTMAP);
		SetTexPointer(3, TC_TEXTURE);

		gl.glBlendFunc(GL_ONE, GL_ONE);
		if (mode == 1)
		{
			// setup register combiners	
			gl.glEnable ( GL_REGISTER_COMBINERS_NV );
			gl.glCombinerParameteriNV ( GL_NUM_GENERAL_COMBINERS_NV, 2 );

			// RC 0 setup:
			//	spare0 = dot(tex0, tex1)
			gl.glCombinerInputNV ( GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_A_NV, GL_TEXTURE0_ARB,
								   GL_EXPAND_NORMAL_NV, GL_RGB );
			gl.glCombinerInputNV ( GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_B_NV, GL_TEXTURE1_ARB,
								   GL_EXPAND_NORMAL_NV, GL_RGB );
			gl.glCombinerOutputNV ( GL_COMBINER0_NV, GL_RGB,
									GL_SPARE0_NV,          // AB output
									GL_DISCARD_NV,         // CD output
									GL_DISCARD_NV,         // sum output
									GL_NONE, GL_NONE,
									GL_TRUE,               // AB = A dot B
									GL_FALSE, GL_FALSE );

			// RC 1 setup:
			//	spare0 = spare0 ^ 2
			//	spare1 = tex2 * tex3
			gl.glCombinerInputNV ( GL_COMBINER1_NV, GL_RGB, GL_VARIABLE_A_NV, GL_SPARE0_NV,
								   GL_UNSIGNED_IDENTITY_NV, GL_RGB );
			gl.glCombinerInputNV ( GL_COMBINER1_NV, GL_RGB, GL_VARIABLE_B_NV, GL_SPARE0_NV,
								   GL_UNSIGNED_IDENTITY_NV, GL_RGB );
			gl.glCombinerInputNV ( GL_COMBINER1_NV, GL_RGB, GL_VARIABLE_C_NV, GL_TEXTURE2_ARB,
								   GL_UNSIGNED_IDENTITY_NV, GL_RGB );
			gl.glCombinerInputNV ( GL_COMBINER1_NV, GL_RGB, GL_VARIABLE_D_NV, GL_TEXTURE3_ARB,
								   GL_UNSIGNED_IDENTITY_NV, GL_RGB );
			gl.glCombinerOutputNV ( GL_COMBINER1_NV, GL_RGB,
									GL_SPARE0_NV,          // AB output
									GL_SPARE1_NV,         // CD output
									GL_DISCARD_NV,         // sum output
									GL_NONE, GL_NONE,
									GL_FALSE, GL_FALSE, GL_FALSE );

			// Final RC setup:
			//	out = (spare0 ^ 2) * spare1
			gl.glFinalCombinerInputNV ( GL_VARIABLE_A_NV, GL_E_TIMES_F_NV,
										GL_UNSIGNED_IDENTITY_NV, GL_RGB );
			gl.glFinalCombinerInputNV ( GL_VARIABLE_B_NV, GL_SPARE1_NV,
										GL_UNSIGNED_IDENTITY_NV, GL_RGB );
			gl.glFinalCombinerInputNV ( GL_VARIABLE_C_NV, GL_ZERO,
										GL_UNSIGNED_IDENTITY_NV, GL_RGB );
			gl.glFinalCombinerInputNV ( GL_VARIABLE_D_NV, GL_ZERO,
										GL_UNSIGNED_IDENTITY_NV, GL_RGB );
			gl.glFinalCombinerInputNV ( GL_VARIABLE_E_NV, GL_SPARE0_NV,
										GL_UNSIGNED_IDENTITY_NV, GL_RGB );
			gl.glFinalCombinerInputNV ( GL_VARIABLE_F_NV, GL_SPARE0_NV,
										GL_UNSIGNED_IDENTITY_NV, GL_RGB );
			gl.glFinalCombinerInputNV ( GL_VARIABLE_G_NV, GL_ZERO,
										GL_UNSIGNED_INVERT_NV, GL_ALPHA );
		}
		else
		{
			// setup ARB shader
			gl.glEnable(GL_FRAGMENT_PROGRAM_ARB);
			gl.glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, specular_fp_id);
		}

		msurface_t* surf = specularchain_low;
		while (surf)
		{
			ExtTextureData *pExtData = GetExtTexData (surf->texinfo->texture);
			Bind2DTexture( GL_TEXTURE1_ARB, surf->lightmaptexturenum + g_lightmap_lightvecs);
			Bind2DTexture( GL_TEXTURE2_ARB, surf->lightmaptexturenum + g_lightmap_addlight);
			Bind2DTexture( GL_TEXTURE3_ARB, pExtData->gl_glossmap_id);
			SetupTexMatrix_Reflected(surf, vec_to_eyes);
			DrawPolyFromArray(surf);
			wpolyspecular++;

			surf = surf->texturechain;
		}

		gl.glActiveTextureARB( GL_TEXTURE0_ARB );
		gl.glDisable(GL_TEXTURE_CUBE_MAP_ARB);

		if (mode == 1)
			gl.glDisable( GL_REGISTER_COMBINERS_NV );
		else
			gl.glDisable( GL_FRAGMENT_PROGRAM_ARB );
	}
}


//===========================
// High quality specular
//===========================
void DrawHighQualitySpecularChain()
{
	// setup texture coords for shader..
	SetTexPointer(0, TC_VERTEX_POSITION); // Use vertex position as texcoords to calc vector to camera
	SetTexPointer(1, TC_LIGHTMAP);
	SetTexPointer(2, TC_OFF);
	SetTexPointer(3, TC_TEXTURE);

	gl.glBlendFunc(GL_ONE, GL_ONE);
	SetTexEnvs(ENVSTATE_REPLACE, ENVSTATE_REPLACE, ENVSTATE_REPLACE, ENVSTATE_REPLACE);

	gl.glEnable(GL_FRAGMENT_PROGRAM_ARB);
	gl.glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, specular_high_fp_id);

	msurface_t* surf = specularchain_high;
	while (surf)
	{
		ExtTextureData *pExtData = GetExtTexData (surf->texinfo->texture);
		Bind2DTexture( GL_TEXTURE0_ARB, pExtData->gl_normalmap_id);
		Bind2DTexture( GL_TEXTURE1_ARB, surf->lightmaptexturenum + g_lightmap_lightvecs);
		Bind2DTexture( GL_TEXTURE2_ARB, surf->lightmaptexturenum + g_lightmap_addlight);
		Bind2DTexture( GL_TEXTURE3_ARB, pExtData->gl_extra_glossmap_id);
		SetupTexMatrix(surf, vec_to_eyes);
		DrawPolyFromArray(surf);
		wpolyspecular_hi++;

		surf = surf->texturechain;
	}

	gl.glDisable( GL_FRAGMENT_PROGRAM_ARB );
}


// =================================
// Debug tools
// =================================

void ResetCounters()
{
//	dbgmode = 0;
	wpolytotal = 0;
	wpolybumped = 0;
	wpolynormal = 0;
	wpolyspecular = 0;
	wpolyspecular_hi = 0;
}

void PrintBumpDebugInfo()
{
	if (cv_bumpdebug->value)
	{
	/*	char *msg;
		switch(dbgmode)
		{
		case 0: msg = "Custom renderer disabled"; break;
		case 1: msg = "Custom renderer mode: 2 pass bump (normal)"; break;
		case 2: msg = "Custom renderer mode: 3 pass bump (uses 3 TU)"; break;
		case 3: msg = "Custom renderer mode: 4 pass bump (uses 2 TU)"; break;
		}
		DrawConsoleString( XRES(10), YRES(120), msg );

		if (!dbgmode)
			return;*/

		char tmp[256];
		sprintf(tmp, "%d wpoly total", wpolytotal);
		DrawConsoleString( XRES(10), YRES(135), tmp );

		sprintf(tmp, "%d wpoly bumped", wpolybumped);
		DrawConsoleString( XRES(10), YRES(150), tmp );

		sprintf(tmp, "%d wpoly normal", wpolynormal);
		DrawConsoleString( XRES(10), YRES(165), tmp );

		sprintf(tmp, "%d wpoly has low quality specular", wpolyspecular);
		DrawConsoleString( XRES(10), YRES(180), tmp );	

		sprintf(tmp, "%d wpoly has high quality specular", wpolyspecular_hi);
		DrawConsoleString( XRES(10), YRES(195), tmp );
	}
}