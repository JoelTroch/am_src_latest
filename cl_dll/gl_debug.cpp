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

#include "r_efx.h"
#include "event_api.h"
#include "pm_defs.h"
#include "pmtrace.h"
#include "triangleapi.h"

#include "gl_renderer.h"
#include <direct.h>


extern cvar_t *cv_bumpvecs;


void DrawVector(const vec3_t &start, const vec3_t &end, float r, float g, float b, float a);
/*
void DrawVector(const vec3_t &start, const vec3_t &end, float r, float g, float b, float a)
{
	HLSPRITE hsprTexture = LoadSprite("sprites/white.spr");
	if (!hsprTexture){
		gEngfuncs.Con_Printf("NO SPRITE white.spr!\n");
		return;
	}
	const model_s *pTexture = gEngfuncs.GetSpritePointer( hsprTexture );
	gEngfuncs.pTriAPI->SpriteTexture( (struct model_s *)pTexture, 0 ); 
	gEngfuncs.pTriAPI->RenderMode( kRenderNormal );
	gEngfuncs.pTriAPI->Color4f( r, g, b, a );
	gEngfuncs.pTriAPI->Begin( TRI_LINES );
		gEngfuncs.pTriAPI->Vertex3f( start[0], start[1], start[2] );
		gEngfuncs.pTriAPI->Vertex3f( end[0], end[1], end[2] );
	gEngfuncs.pTriAPI->End();
}*/

void DrawVectorDir(const vec3_t &start, const vec3_t &dir, float scale, float r, float g, float b, float a)
{
	vec3_t end;
	VectorMA(start, scale, dir, end);
	DrawVector(start, end, r, g, b, a);
}

vec3_t drawvecsrc;
extern vec3_t v_angles;
extern vec3_t v_origin;
float gdist;
color24 test_add_color, test_base_color;

int RecursiveDrawTextureVecs (mnode_t *node, const vec3_t &start, const vec3_t &end)
{
	int			r;
	float		front, back, frac;
	int			side;
	mplane_t	*plane;
	vec3_t		mid;
	msurface_t	*surf;
	int			s, t, ds, dt;
	int			i;
	mtexinfo_t	*tex;
	color24		*lightmap; // buz

	if (node->contents < 0)
		return -1;		// didn't hit anything
	
// calculate mid point

// FIXME: optimize for axial
	plane = node->plane;
	front = DotProduct (start, plane->normal) - plane->dist;
	back = DotProduct (end, plane->normal) - plane->dist;
	side = front < 0;
	
	if ( (back < 0) == side)
		return RecursiveDrawTextureVecs (node->children[side], start, end);
	
	frac = front / (front-back);
	mid[0] = start[0] + (end[0] - start[0])*frac;
	mid[1] = start[1] + (end[1] - start[1])*frac;
	mid[2] = start[2] + (end[2] - start[2])*frac;
	
// go down front side	
	r = RecursiveDrawTextureVecs (node->children[side], start, mid);
	if (r >= 0)
		return r;		// hit something
		
	if ( (back < 0) == side )
		return -1;		// didn't hit anuthing
		
// check for impact on this node
	model_t* world = gEngfuncs.GetEntityByIndex(0)->model; // buz

	surf = world->surfaces + node->firstsurface;
	for (i=0 ; i<node->numsurfaces ; i++, surf++)
	{
		if (surf->flags & SURF_DRAWTILED)
			continue;	// no lightmaps

		tex = surf->texinfo;
		
		s = DotProduct (mid, tex->vecs[0]) + tex->vecs[0][3];
		t = DotProduct (mid, tex->vecs[1]) + tex->vecs[1][3];;

		if (s < surf->texturemins[0] ||
		t < surf->texturemins[1])
			continue;
		
		ds = s - surf->texturemins[0];
		dt = t - surf->texturemins[1];
		
		if ( ds > surf->extents[0] || dt > surf->extents[1] )
			continue;

		if (!surf->samples)
			continue;

		ds >>= 4;
		dt >>= 4;

		lightmap = surf->samples;
		r = 0;
		if (lightmap)
		{
			lightmap += dt * ((surf->extents[0]>>4)+1) + ds;
			
			for (int style = 0; style < MAXLIGHTMAPS && surf->styles[style] != 255; style++)
			{
				if (surf->styles[style] == BUMP_LIGHTVECS_STYLE)
				{
					vec3_t outcolor;
					outcolor[0] = (float)lightmap->r / 255.0f;
					outcolor[1] = (float)lightmap->g / 255.0f;
					outcolor[2] = (float)lightmap->b / 255.0f;

					vec3_t vec_x;
					vec3_t vec_y;
					VectorCopy(tex->vecs[0], vec_x);
					VectorCopy(tex->vecs[1], vec_y);
					VectorNormalize(vec_x);
					VectorNormalize(vec_y);
					DrawVectorDir(drawvecsrc, vec_x, 8, 1, 0, 0, 1);
					DrawVectorDir(drawvecsrc, vec_y, 8, 0, 1, 0, 1);

					vec3_t pnormal;
					VectorCopy(surf->plane->normal, pnormal);
					if (surf->flags & SURF_PLANEBACK)
					{
						VectorInverse(pnormal);
					}
					DrawVectorDir(drawvecsrc, pnormal, 8, 0, 0, 1, 1);

					vec3_t tmp;
					VectorClear(tmp);
					VectorMA(tmp, outcolor[0]*2 - 1, vec_x, tmp);
					VectorMA(tmp, outcolor[1]*2 - 1, vec_y, tmp);
					VectorMA(tmp, outcolor[2]*2 - 1, pnormal, tmp);

				//	DrawVectorDir(drawvecsrc, tmp, 300, outcolor[0], outcolor[1], outcolor[2], 1);
					DrawVectorDir(drawvecsrc, tmp, 300, 1, 1, 0, 1);
					gdist = tmp.Length();					
				}
				else if (surf->styles[style] == BUMP_ADDLIGHT_STYLE)
				{
					test_add_color.r = lightmap->r;
					test_add_color.g = lightmap->g;
					test_add_color.b = lightmap->b;
				}
				else if (surf->styles[style] == BUMP_BASELIGHT_STYLE)
				{
					test_base_color.r = lightmap->r;
					test_base_color.g = lightmap->g;
					test_base_color.b = lightmap->b;
				}

				lightmap += ((surf->extents[0]>>4)+1) *
						((surf->extents[1]>>4)+1);
			}

		//	gEngfuncs.Con_Printf("vecs draw %f, %f, %f\n", drawvecsrc[0], drawvecsrc[1], drawvecsrc[2]);

			/*	for (maps = 0 ; maps < MAXLIGHTMAPS && surf->styles[maps] != 255 ;
					maps++)
			{
				scale = d_lightstylevalue[surf->styles[maps]];
				r += *lightmap * scale;
				lightmap += ((surf->extents[0]>>4)+1) *
						((surf->extents[1]>>4)+1);
			}			
			r >>= 8;*/
		}		
		return r;
	}

// go down back side
	return RecursiveDrawTextureVecs (node->children[!side], mid, end);
}


void DrawTextureVecs ()
{
	gdist = 0;

	if (!cv_bumpvecs->value)
		return;

	vec3_t fwd, end;

	model_t* world = gEngfuncs.GetEntityByIndex(0)->model; // buz
	
	if (!world->lightdata)
		return;
	
//	cl_entity_t *player = gEngfuncs.GetLocalPlayer();
	AngleVectors(v_angles, fwd, NULL, NULL);
	VectorMA(v_origin, 1024, fwd, end);

	pmtrace_t pmtrace;
	gEngfuncs.pEventAPI->EV_SetTraceHull( 2 );
	gEngfuncs.pEventAPI->EV_PlayerTrace( v_origin, end, PM_STUDIO_IGNORE | PM_WORLD_ONLY, -1, &pmtrace );					

	if ( pmtrace.fraction != 1 )
	{
		VectorMA( pmtrace.endpos, 2, pmtrace.plane.normal, drawvecsrc );
		RecursiveDrawTextureVecs (world->nodes, v_origin, end);
	}
}



// ====================================
// buz: Test TGA writer
// ====================================
typedef struct tgaheader_s
{
	unsigned char	IdLength;
	unsigned char	ColorMap;
	unsigned char	DataType;
	unsigned char	unused[5];
	unsigned short	OriginX;
	unsigned short	OriginY;
	unsigned short	Width;
	unsigned short	Height;
	unsigned char	BPP;
	unsigned char	Description;
} TGAheader;

void getFilePath(char *filename, char *out)
{
	if (!filename)
		return;

	int m = 0;

	for (int i = strlen(filename) - 1; i >= 0; i--)
	{
		m = i;
		if (filename[i] == '/' || filename[i] == '\\')
			break;
	}

	memset(out, 0, 256);
	memcpy(out, filename, m);
}

int WriteTGA_24(color24 *pixels, unsigned short width, unsigned short height, const char *filename)
{
	TGAheader TGAhdr;
	memset(&TGAhdr, 0, sizeof(TGAheader));
	TGAhdr.Width = width;
	TGAhdr.Height = height;
	TGAhdr.BPP = sizeof(color24)* 8;
	TGAhdr.Description = 0x20;
	TGAhdr.DataType = 2; // no compression

	char path[256];
	getFilePath((char*)filename, path);
	if (strlen(path) <= 1)
		return 0;

	//gEngfuncs.Con_Printf("Cubemap path:%s\n", path);
	_mkdir(path);

	FILE *fp = fopen(filename, "wb");
	if (!fp)
		return FALSE;

	if (!fwrite(&TGAhdr, sizeof(TGAheader), 1, fp))
	{
		fclose(fp);
		return FALSE;
	}

	int numpixels = width * height;
	/*	for (int i = 0; i < numpixels; i++, pixels++)
	{
	color24 wrap;
	wrap.r = pixels->b;
	wrap.g = pixels->g;
	wrap.b = pixels->r;
	if (!fwrite( &wrap, sizeof(color24), 1, fp ))
	{
	fclose(fp);
	return FALSE;
	}
	}*/
	for (int x = width - 1; x >= 0; x--)
	{
		for (int y = height - 1; y >= 0; y--)
		{
			int n = y + x * width;
			color24 wrap;
			wrap.r = pixels[n].b;
			wrap.g = pixels[n].g;
			wrap.b = pixels[n].r;
			if (!fwrite(&wrap, sizeof(color24), 1, fp))
			{
				fclose(fp);
				return FALSE;
			}
		}
	}

	fclose(fp);
	return TRUE;
}

int WriteTGA_24old(color24 *pixels, unsigned short width, unsigned short height, const char *filename)
{
	TGAheader TGAhdr;
	memset(&TGAhdr, 0, sizeof(TGAheader));
	TGAhdr.Width = width;
	TGAhdr.Height = height;
	TGAhdr.BPP = sizeof(color24)* 8;
	TGAhdr.Description = 0x20;
	TGAhdr.DataType = 2; // no compression

	char path[256];
	getFilePath((char*)filename, path);
	if (strlen(path) <= 1)
		return 0;

	//gEngfuncs.Con_Printf("Cubemap path:%s\n", path);
	_mkdir(path);

	FILE *fp = fopen(filename, "wb");
	if (!fp)
		return FALSE;

	if (!fwrite(&TGAhdr, sizeof(TGAheader), 1, fp))
	{
		fclose(fp);
		return FALSE;
	}

	int numpixels = width * height;
	for (int i = 0; i < numpixels; i++, pixels++)
	{
	color24 wrap;
	wrap.r = pixels->b;
	wrap.g = pixels->g;
	wrap.b = pixels->r;
	if (!fwrite( &wrap, sizeof(color24), 1, fp ))
	{
	fclose(fp);
	return FALSE;
	}
	}

	fclose(fp);
	return TRUE;
}


void DumpTextures()
{
	model_t* world = gEngfuncs.GetEntityByIndex(0)->model;
	if (!world) return;

	GLint oldbind;
	gl.glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldbind);

	texture_t** tex = (texture_t**)world->textures;
	for (int i = 0; i < world->numtextures; i++)
	{
		if (!tex[i])
			continue;

		if (tex[i]->name[0] == 0)
			continue;

		GLint width, height;
		gl.glBindTexture(GL_TEXTURE_2D, tex[i]->gl_texturenum);
		gl.glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
		gl.glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
		
		color24 *pixels = new color24[width*height];
		gl.glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);

		char buf[256];
		sprintf(buf, "%s/dumped/%s.tga", gEngfuncs.pfnGetGameDirectory(), tex[i]->name);
		if (WriteTGA_24old(pixels, width, height, buf))
			gEngfuncs.Con_Printf("written %s (%d x %d)\n", buf, width, height);
		else
			gEngfuncs.Con_Printf("error writting %s\n", buf);

		delete[] pixels;
	}

	extern int lightmapstotal;
	for (int i = g_lightmap_baselight; i < g_lightmap_baselight + (lightmapstotal+1); i++)
	{
		GLint width, height;
		gl.glBindTexture(GL_TEXTURE_2D, i);
		gl.glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
		gl.glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

		color24 *pixels = new color24[width*height];
		gl.glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);

		char buf[256];
		sprintf(buf, "%s/dumped/map%i.tga", gEngfuncs.pfnGetGameDirectory(), i - g_lightmap_baselight);
		if (WriteTGA_24old(pixels, width, height, buf))
			gEngfuncs.Con_Printf("written %s (%d x %d)\n", buf, width, height);
		else
			gEngfuncs.Con_Printf("error writting %s\n", buf);

		delete[] pixels;
	}

	gl.glBindTexture(GL_TEXTURE_2D, oldbind);
}


int TextureId( const char *name )
{
	int num = 0;
	model_t* world = gEngfuncs.GetEntityByIndex(0)->model;
	texture_t** tex = (texture_t**)world->textures;
	for (int i = 0; i < world->numtextures; i++)
	{
		if (tex[i]->name[0] != 0)
		{
			if (!strcmp(name, tex[i]->name))
				return num;
			num++;
		}
	}
	return -1;
}



void DumpLeafInfo()
{
	int i;
	model_t* world = gEngfuncs.GetEntityByIndex(0)->model;
	if (!world)
	{
		gEngfuncs.Con_Printf("WTF???\n");
		return;
	}

	char buf[256];
	sprintf(buf, "%s/drawlog.txt", gEngfuncs.pfnGetGameDirectory());
	FILE *fp = fopen(buf, "wt");
	if (!fp)
	{
		gEngfuncs.Con_Printf("ERROR: couldnt open file %s for writing!\n", buf);
		return;
	}

	msurface_t* surfaces = &world->surfaces[world->firstmodelsurface];
	for (i = 0; i < world->nummodelsurfaces; i++)
	{
		surfaces[i].visframe = 0;
	}

	fprintf ( fp, "%d leafs\n", world->numleafs );
	int counter = 0;

	for (i = 0; i < world->numleafs; i++)
	{
		fprintf ( fp, "%d msurfs\n", world->leafs[i].nummarksurfaces );
		counter += world->leafs[i].nummarksurfaces;

		msurface_t **mark = world->leafs[i].firstmarksurface;
		int c = world->leafs[i].nummarksurfaces;
		if (c)
		{
			do
			{
				(*mark)->visframe += 1;
				mark++;
			} while (--c);
		}
	}

	fprintf ( fp, "%d marksurfaces total, %d average\n", counter, counter/world->numleafs );
	fprintf ( fp, "\n\n%d surfaces in world\n", world->nummodelsurfaces );

	counter = 0;
	surfaces = &world->surfaces[world->firstmodelsurface];
	for (i = 0; i < world->nummodelsurfaces; i++)
	{
		fprintf ( fp, "%d refs\n", surfaces[i].visframe );
		counter += surfaces[i].visframe;
		surfaces[i].visframe = 0;		
	}

	fprintf ( fp, "%d references total, %d average\n", counter, counter/world->numsurfaces );

	fclose(fp);
	gEngfuncs.Con_Printf("leaf info dump created\n");
}


void DumpLevel()
{
	model_t* world = gEngfuncs.GetEntityByIndex(0)->model;
	if (!world)
	{
		gEngfuncs.Con_Printf("WTF???\n");
		return;
	}

	char buf[256];
	sprintf(buf, "%s/dumped/leveldump.lev", gEngfuncs.pfnGetGameDirectory());
	FILE *fp = fopen(buf, "wb");
	if (!fp)
	{
		gEngfuncs.Con_Printf("ERROR: couldnt open file %s for writing!\n", buf);
		return;
	}

	// calc the number of textures
	int numtextures = 0;
	texture_t** tex = (texture_t**)world->textures;
	for (int i = 0; i < world->numtextures; i++)
	{
		if (tex[i]->name[0] != 0)
			numtextures++;
	}

	fwrite( &numtextures, sizeof(numtextures), 1, fp );

	// save texture names
	for (int i = 0; i < world->numtextures; i++)
	{
		if (tex[i]->name[0] != 0)
			fwrite( tex[i]->name, 16, 1, fp );
	}

	// calculate number of used faces and vertexes
	int numpolys = 0;
	int numverts = 0;
	msurface_t* surfaces = world->surfaces;
	for (int i = 0; i < world->numsurfaces; i++)
	{
		if (!(surfaces[i].flags & (SURF_DRAWSKY|SURF_DRAWTURB)))
		{
			glpoly_t *p = surfaces[i].polys;
			if (p->numverts > 0)
			{
				numpolys++;
				numverts += p->numverts;
			}
		}
	}

	int polydatasize = (numpolys * sizeof(int) * 2) + (numverts * sizeof(float) * 5);
	char *polydata = (char*)malloc(polydatasize);
	if (!polydata)
	{
		gEngfuncs.Con_Printf("ACHTUNG!\n");
		fclose(fp);
		return;
	}

	char *cur = polydata;
	for (int i = 0; i < world->numsurfaces; i++)
	{
		if (!(surfaces[i].flags & (SURF_DRAWSKY|SURF_DRAWTURB)))
		{
			glpoly_t *p = surfaces[i].polys;
			if (p->numverts > 0)
			{
				float *v = p->verts[0];
				int *val = (int*)cur;
				val[0] = TextureId(surfaces[i].texinfo->texture->name); // store texture id
				if (val[0] == -1)
					gEngfuncs.Con_Printf("warning: bad texture id for %s!\n", surfaces[i].texinfo->texture->name);
				val[1] = p->numverts; // store number of vertices
				cur += 2 * sizeof(int);
				for (int j=0; j<p->numverts; j++, v+= VERTEXSIZE)
				{
					float *vdata = (float*)cur;
					vdata[0] = v[0]; // copy position
					vdata[1] = v[1];
					vdata[2] = v[2];
					vdata[3] = v[3]; // copy texccords
					vdata[4] = v[4];
					cur += 5 * sizeof(float);
				}
			}
		}
	}

	fwrite( &numpolys, sizeof(numpolys), 1, fp );
	fwrite( &polydatasize, sizeof(polydatasize), 1, fp );
	fwrite( polydata, polydatasize, 1, fp );
	fclose(fp);
	free(polydata);
	gEngfuncs.Con_Printf("leveldump created (%d textures, %d polys)\n", numtextures, numpolys);
}


void PrintOtherDebugInfo()
{
	if (gdist)
	{
		char msg[256];
		sprintf(msg, "light vector length: %f\n", gdist);
		DrawConsoleString( XRES(10), YRES(50), msg );

		sprintf(msg, "add light: {%d, %d, %d}\n", test_add_color.r, test_add_color.g, test_add_color.b);
		DrawConsoleString( XRES(10), YRES(65), msg );

		sprintf(msg, "base light: {%d, %d, %d}\n", test_base_color.r, test_base_color.g, test_base_color.b);
		DrawConsoleString( XRES(10), YRES(80), msg );
	}
}