//
// written by BUzer for HL: Paranoia modification
//
//		2005-2006

#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "com_model.h"
#include "glmanager.h"
#include "gl_texloader.h"

#include "studio.h"
#include "studio_util.h"
#include "r_studioint.h"

#include "ref_params.h"

#include "r_efx.h"
#include "event_api.h"
#include "pm_defs.h"
#include "pmtrace.h"

#include "StudioModelRenderer.h"
#include "GameStudioModelRenderer.h"

#include "gl_renderer.h"
#include "gl_dlight.h"
#include "gl_shadows.h"

DynamicLight cl_dlights[MAX_DLIGHTS];
extern CGameStudioModelRenderer g_StudioRenderer;

extern int curvisframe;
extern int norm_cubemap_id;
extern int default_normalmap_id;

int attenuation_1d = 0; // for spotlights
int attenuation_3d = 0; // for pointlights
int atten_point_2d = 0; // for pointlights, on videocards without 3d textures support
int atten_point_1d = 0; // ...

// sets in model space.
// may be rotated if drawing entity
vec3_t current_light_origin;
vec3_t current_spot_vec_forward;
vec3_t current_light_color;

DynamicLight *current_light;
static glpoly_t *chain; // for two-pass light rendering
static int texid; // current spot light texture id

extern cvar_t *cv_dyntwopass; // debug - force lights to draw in two pass
float texMatrix[16];
//============================
// BBox from BBox culling
//============================
vec3_t cullmins, cullmaxs;

void SetPointlightBBox()
{
	cullmins[0] = current_light->origin[0] - current_light->radius;
	cullmins[1] = current_light->origin[1] - current_light->radius;
	cullmins[2] = current_light->origin[2] - current_light->radius;
	cullmaxs[0] = current_light->origin[0] + current_light->radius;
	cullmaxs[1] = current_light->origin[1] + current_light->radius;
	cullmaxs[2] = current_light->origin[2] + current_light->radius;
}

int CullPointlightBBox (vec3_t mins, vec3_t maxs)
{
	if (mins[0] > cullmaxs[0]) return TRUE;
	if (mins[1] > cullmaxs[1]) return TRUE;
	if (mins[2] > cullmaxs[2]) return TRUE;
	if (maxs[0] < cullmins[0]) return TRUE;
	if (maxs[1] < cullmins[1]) return TRUE;
	if (maxs[2] < cullmins[2]) return TRUE;
	return FALSE;
}

int IsPitchReversed(float pitch)
{
	int quadrant = int(pitch / 90) % 4;
	if ((quadrant == 1) || (quadrant == 2)) return TRUE;
	return FALSE;
}


void SetupProjection()
{
	float width = tan((M_PI/360) * current_light->cone_hor);
	float height = tan((M_PI/360) * current_light->cone_ver);
	float nearplane = 1;
	int reversed = IsPitchReversed(current_light->angles[PITCH]);

	vec3_t target = current_light_origin + (current_spot_vec_forward * current_light->radius);

	// enable automatic texture coordinates generation
	GLfloat planeS[] = {1.0, 0.0, 0.0, 0.0};
	GLfloat planeT[] = {0.0, 1.0, 0.0, 0.0};
	GLfloat planeR[] = {0.0, 0.0, 1.0, 0.0};
	GLfloat planeQ[] = {0.0, 0.0, 0.0, 1.0};

	gl.glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR); gl.glTexGenfv(GL_S, GL_EYE_PLANE, planeS);
	gl.glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR); gl.glTexGenfv(GL_T, GL_EYE_PLANE, planeT);
	gl.glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR); gl.glTexGenfv(GL_R, GL_EYE_PLANE, planeR);
	gl.glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR); gl.glTexGenfv(GL_Q, GL_EYE_PLANE, planeQ);
	gl.glEnable(GL_TEXTURE_GEN_S);
	gl.glEnable(GL_TEXTURE_GEN_T);
	gl.glEnable(GL_TEXTURE_GEN_R);
	gl.glEnable(GL_TEXTURE_GEN_Q);

	// load texture projection matrix
	gl.glMatrixMode(GL_TEXTURE);
	gl.glLoadIdentity();
	
	gl.glTranslatef(0.5, 0.5, 0.5);
	gl.glScalef(0.5, 0.5, 0.5);

	gl.glFrustum(-width, width, -height, height, nearplane, current_light->radius);
	gl.gluLookAt(current_light_origin[0], current_light_origin[1], current_light_origin[2],
		target[0], target[1], target[2], 0, 0, reversed ? -1 : 1);

	gl.glMatrixMode(GL_MODELVIEW);
}

void DisableProjection()
{
	gl.glMatrixMode(GL_TEXTURE);
	gl.glLoadIdentity();

	gl.glMatrixMode(GL_MODELVIEW);
	gl.glDisable(GL_TEXTURE_GEN_S);
	gl.glDisable(GL_TEXTURE_GEN_T);
	gl.glDisable(GL_TEXTURE_GEN_R);
	gl.glDisable(GL_TEXTURE_GEN_Q);
}


//=========================
// SetupAttenuationSpotlight
//
// sets 1d attenuation texture coordinate generation for spotlights
// (in model space)
//=========================
void SetupAttenuationSpotlight()
{
	GLfloat planeS[4];
	planeS[0] = current_spot_vec_forward[0] / current_light->radius;
	planeS[1] = current_spot_vec_forward[1] / current_light->radius;
	planeS[2] = current_spot_vec_forward[2] / current_light->radius;
	planeS[3] = - DotProduct(current_spot_vec_forward, current_light_origin) / current_light->radius;

	gl.glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR); gl.glTexGenfv(GL_S, GL_EYE_PLANE, planeS);
	gl.glEnable(GL_TEXTURE_GEN_S);
}

void DisableAttenuationSpotlight()
{
	gl.glDisable(GL_TEXTURE_GEN_S);
}


//=========================
// SetupAttenuation3D
//
// sets 3d attenuation texture coordinate generation for point lights
// (in model space)
//=========================
void SetupAttenuation3D()
{
	float r = 1 / (current_light->radius * 2);
	GLfloat planeS[] = {r, 0, 0, -current_light_origin[0] * r + 0.5};
	GLfloat planeT[] = {0, r, 0, -current_light_origin[1] * r + 0.5};
	GLfloat planeR[] = {0, 0, r, -current_light_origin[2] * r + 0.5};

	gl.glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR); gl.glTexGenfv(GL_S, GL_EYE_PLANE, planeS);
	gl.glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR); gl.glTexGenfv(GL_T, GL_EYE_PLANE, planeT);
	gl.glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR); gl.glTexGenfv(GL_R, GL_EYE_PLANE, planeR);
	gl.glEnable(GL_TEXTURE_GEN_S);
	gl.glEnable(GL_TEXTURE_GEN_T);
	gl.glEnable(GL_TEXTURE_GEN_R);
}

void DisableAttenuation3D()
{
	gl.glDisable(GL_TEXTURE_GEN_S);
	gl.glDisable(GL_TEXTURE_GEN_T);
	gl.glDisable(GL_TEXTURE_GEN_R);
}


//=========================
// SetupAttenuation2D, SetupAttenuation1D
//
// used in pair, setups attenuations on cards withoud 3d textures support
//=========================
void SetupAttenuation2D()
{
	float r = 1 / (current_light->radius * 2);
	GLfloat planeS[] = {r, 0, 0, -current_light_origin[0] * r + 0.5};
	GLfloat planeT[] = {0, r, 0, -current_light_origin[1] * r + 0.5};

	gl.glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR); gl.glTexGenfv(GL_S, GL_EYE_PLANE, planeS);
	gl.glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR); gl.glTexGenfv(GL_T, GL_EYE_PLANE, planeT);
	gl.glEnable(GL_TEXTURE_GEN_S);
	gl.glEnable(GL_TEXTURE_GEN_T);
}

void SetupAttenuation1D()
{
	float r = 1 / (current_light->radius * 2);
	GLfloat planeS[] = {0, 0, r, -current_light_origin[2] * r + 0.5};

	gl.glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR); gl.glTexGenfv(GL_S, GL_EYE_PLANE, planeS);
	gl.glEnable(GL_TEXTURE_GEN_S);
}


void DisableAttenuation2D()
{
	gl.glDisable(GL_TEXTURE_GEN_S);
	gl.glDisable(GL_TEXTURE_GEN_T);
}

void DisableAttenuation1D()
{
	gl.glDisable(GL_TEXTURE_GEN_S);
}


//============================
// SetupSpotLight, SetupPointLight
//
// Prepares texture stages and binds all appropriate textures
// current_light_origin and current_spot_vec_forward must be set before calling
//============================

int SetupSpotLight()
{
	//texid = LoadCacheTexture(current_light->spot_texture, MIPS_NO, TRUE);
	texid = current_light->depth;
	if (!texid)
		return FALSE;

	if (!cv_dyntwopass->value && gl.MAX_TU_supported >= 4)
	{
		// setup blending
		gl.glEnable(GL_BLEND);
		gl.glBlendFunc(GL_ONE, GL_ONE);
		gl.glDepthMask(GL_FALSE);

		// setup texture stages
		current_light_color = current_light->color;
		gl.glColor4f(current_light_color.x * 255, current_light_color.y * 255, current_light_color.z * 255, 255);
		SetTexEnvs(ENVSTATE_OFF, ENVSTATE_MUL, ENVSTATE_MUL, ENVSTATE_MUL);

		SetTexPointer(0, TC_OFF);
		SetTexPointer(1, TC_TEXTURE);
		SetTexPointer(2, TC_OFF); // auto generated
		SetTexPointer(3, TC_OFF); // auto generated

		// normalization cube map
	//	gl.glActiveTextureARB( GL_TEXTURE0_ARB );
	//	gl.glEnable(GL_TEXTURE_CUBE_MAP_ARB);
	//	gl.glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, norm_cubemap_id);

		gl.glGetFloatv(GL_TEXTURE_MATRIX, texMatrix);

		//spotlight texture
		gl.glActiveTextureARB(GL_TEXTURE1_ARB);
		int projTexId = LoadCacheTexture(current_light->spot_texture, MIPS_NO, TRUE);
		Bind2DTexture(GL_TEXTURE1_ARB, projTexId);
		SetupProjection();

		// shadowmap texture
		gl.glActiveTextureARB( GL_TEXTURE2_ARB );
		Bind2DTexture(GL_TEXTURE2_ARB, texid);
		SetupProjection();
	
		// attenuation
		gl.glActiveTextureARB( GL_TEXTURE3_ARB );
		gl.glDisable(GL_TEXTURE_2D);
		gl.glEnable(GL_TEXTURE_1D);
		gl.glBindTexture(GL_TEXTURE_1D, attenuation_1d);
		SetupAttenuationSpotlight();

		SetupPCF();
		return TRUE;
	}
}

int SetupPointLight()
{
//	if (!attenuation_3d || gl.MAX_TU_supported < 4)
//		return FALSE;

	current_light_color[0] = ApplyGamma(current_light->color[0]) / 2;
	current_light_color[1] = ApplyGamma(current_light->color[1]) / 2;
	current_light_color[2] = ApplyGamma(current_light->color[2]) / 2;

	gl.glEnable(GL_BLEND);
	gl.glDepthMask(GL_FALSE);

	if (!cv_dyntwopass->value && attenuation_3d && gl.MAX_TU_supported >= 4)
	{
		// draw light in one pass
		gl.glBlendFunc(GL_ONE, GL_ONE);
	
		// setup texture stages
		SetTexEnvs(ENVSTATE_REPLACE, ENVSTATE_DOT, ENVSTATE_MUL, ENVSTATE_MUL_PREV_CONST);
		gl.glColor3f(current_light_color[0], current_light_color[1], current_light_color[2]);

		SetTexPointer(0, TC_VERTEX_POSITION);
		SetTexPointer(1, TC_TEXTURE);
		SetTexPointer(2, TC_OFF); // auto generated
		SetTexPointer(3, TC_OFF); // not used

		// normalization cube map
		gl.glActiveTextureARB( GL_TEXTURE0_ARB );
		gl.glEnable(GL_TEXTURE_CUBE_MAP_ARB);
		gl.glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, norm_cubemap_id);

		// 3d attenuation texture
		gl.glActiveTextureARB( GL_TEXTURE2_ARB );
		gl.glEnable(GL_TEXTURE_3D);
		gl.glBindTexture(GL_TEXTURE_3D, attenuation_3d);
		SetupAttenuation3D();

		// light color (bind dummy texture)
		gl.glActiveTextureARB( GL_TEXTURE3_ARB );
		Bind2DTexture(GL_TEXTURE3_ARB, default_normalmap_id);
		return TRUE;
	}
}


//============================
// FinishSpotLight, FinishPointLight
//
// renders second pass if requed and clears all used gl states
//============================
void FinishSpotLight()
{
	gl.glActiveTextureARB( GL_TEXTURE0_ARB );
	gl.glDisable(GL_TEXTURE_CUBE_MAP_ARB);
	gl.glMatrixMode(GL_TEXTURE);
	gl.glLoadMatrixf(texMatrix);
	gl.glMatrixMode(GL_MODELVIEW);

	DisablePCF();

	gl.glActiveTextureARB(GL_TEXTURE1_ARB);
	DisableProjection();
	gl.glActiveTextureARB(GL_TEXTURE2_ARB);
	DisableProjection();
	gl.glActiveTextureARB( GL_TEXTURE3_ARB );
	gl.glDisable(GL_TEXTURE_1D);
	gl.glEnable(GL_TEXTURE_2D);
	DisableAttenuationSpotlight();

}


void FinishPointLight()
{
	gl.glActiveTextureARB( GL_TEXTURE0_ARB );
	gl.glDisable(GL_TEXTURE_CUBE_MAP_ARB);
	gl.glMatrixMode(GL_TEXTURE);
	gl.glLoadIdentity();
	gl.glMatrixMode(GL_MODELVIEW);

	gl.glActiveTextureARB( GL_TEXTURE2_ARB );
	gl.glDisable(GL_TEXTURE_3D);
	DisableAttenuation3D();

}


//============================
// LightDrawPoly
//
// binds normal map, sets texture matrix, and renders a polygon
//============================
void LightDrawPoly( msurface_t *surf,bool spotlight )
{
	if (!spotlight)
	{
		int normalmap_id = default_normalmap_id;
		ExtTextureData *pExtData = GetExtTexData (surf->texinfo->texture);
		if (pExtData && pExtData->gl_normalmap_id)
			normalmap_id = pExtData->gl_normalmap_id;

		Bind2DTexture(GL_TEXTURE1_ARB, normalmap_id);
		SetupTexMatrix (surf, current_light_origin);
	}

	DrawPolyFromArray(surf);
}

/*******************************
*
* Entity lighting rendering
*
********************************/


//=====================================
// DrawEntityFacesForLight
//
// Draws all faces on given entity which visible to light and camera
//=====================================
void DrawEntityFacesForLight (cl_entity_t *e,bool spot = false)
{
	msurface_t *psurf = &e->model->surfaces[e->model->firstmodelsurface];
	for (int i=0 ; i<e->model->nummodelsurfaces ; i++, psurf++)
	{
		if (psurf->visframe == framecount) // visible
		{
			mplane_t *pplane = psurf->plane;
			float dot = DotProduct (current_light_origin, pplane->normal) - pplane->dist;
			if ( ((psurf->flags & SURF_PLANEBACK) && (dot < -BACKFACE_EPSILON)) ||
				(!(psurf->flags & SURF_PLANEBACK) && (dot > BACKFACE_EPSILON)) )
			{
				if (dot < current_light->radius)
					LightDrawPoly(psurf,spot);
			}
		}
	}
}

//=====================================
// DrawDynamicLightForEntity
//
// Renders dynamic lights on given entity.
// Used on 'moved' entities, who has non-zero angles or origin
//=====================================
void DrawDynamicLightForEntity (cl_entity_t *e)
{
	vec3_t		mins, maxs;
	int			rotated;

	if (e->angles[0] || e->angles[1] || e->angles[2])
	{
		rotated = true;
		for (int i=0 ; i<3 ; i++)
		{
			mins[i] = e->origin[i] - e->model->radius;
			maxs[i] = e->origin[i] + e->model->radius;
		}
	}
	else
	{
		rotated = false;
		VectorAdd (e->origin, e->model->mins, mins);
		VectorAdd (e->origin, e->model->maxs, maxs);
	}

	gl.glDepthFunc(GL_EQUAL);

	// go through dynamic lights list
	float time = gEngfuncs.GetClientTime();
	DynamicLight *dl = cl_dlights;
	for (int l=0 ; l<MAX_DLIGHTS ; l++, dl++)
	{
		vec3_t temp, forward, right, up;

		if (dl->die < time || !dl->radius)
			continue;

		current_light = dl;
		VectorSubtract (current_light->origin, e->origin, current_light_origin);
		if (rotated)
		{
			AngleVectors (e->angles, forward, right, up);

			VectorCopy (current_light_origin, temp);
			current_light_origin[0] = DotProduct (temp, forward);
			current_light_origin[1] = -DotProduct (temp, right);
			current_light_origin[2] = DotProduct (temp, up);
		}

		//
		// Setup light
		//
		if (current_light->spot_texture[0]) // spotlight
		{
			if (current_light->frustum.R_CullBox(mins, maxs))
				continue;

			// store light direction vector for attenuation calc
			AngleVectors(current_light->angles, current_spot_vec_forward, NULL, NULL);
			if (rotated)
			{
				VectorCopy (current_spot_vec_forward, temp);
				current_spot_vec_forward[0] = DotProduct (temp, forward);
				current_spot_vec_forward[1] = -DotProduct (temp, right);
				current_spot_vec_forward[2] = DotProduct (temp, up);
			}

			if (!SetupSpotLight())
				continue;
		}
		else // pointlight
		{
			SetPointlightBBox();
			if (CullPointlightBBox (mins, maxs))
				continue;

			if (!SetupPointLight())
				continue;
		}

		// Draw faces
		DrawEntityFacesForLight(e, current_light->spot_texture[0]);

		//
		// Reset light
		//
		if (current_light->spot_texture[0])
			FinishSpotLight();
		else
			FinishPointLight();
	}
}

//=====================================
// RecursiveDrawWorldLight
//
// recursively goes throug all world nodes drawing visible polygons
//=====================================
void RecursiveDrawWorldLight (mnode_t *node)
{
	if (node->contents == CONTENTS_SOLID)
		return;		// solid

	if (node->visframe != curvisframe)
		return;

	// buz: visible surfaces already marked
	if (node->contents < 0)
		return;

	if (current_light->spot_texture[0])
	{
		if (current_light->frustum.R_CullBox(node->minmaxs, node->minmaxs + 3)) // cull from spotlight cone
			return;
	}
	else
	{
		if (CullPointlightBBox (node->minmaxs, node->minmaxs+3)) // cull from point light bbox
			return;
	}

// node is just a decision point, so go down the apropriate sides
// find which side of the node we are on
	int side;
	double dot;
	mplane_t *plane = node->plane;
	switch (plane->type)
	{
	case PLANE_X:
		dot = current_light_origin[0] - plane->dist;	break;
	case PLANE_Y:
		dot = current_light_origin[1] - plane->dist;	break;
	case PLANE_Z:
		dot = current_light_origin[2] - plane->dist;	break;
	default:
		dot = DotProduct (current_light_origin, plane->normal) - plane->dist; break;
	}

	if (dot >= 0) side = 0;
	else side = 1;

// recurse down the children, front side first
	RecursiveDrawWorldLight (node->children[side]);

// draw stuff
	int c = node->numsurfaces;
	if (c)
	{
		model_t *world = gEngfuncs.GetEntityByIndex(0)->model;
		msurface_t *surf = world->surfaces + node->firstsurface;

		if (dot < 0 -BACKFACE_EPSILON)
			side = SURF_PLANEBACK;
		else if (dot > BACKFACE_EPSILON)
			side = 0;
		{
			for ( ; c ; c--, surf++)
			{
				if (surf->visframe != framecount)
					continue;

				// don't backface underwater surfaces, because they warp
				if ( !(surf->flags & SURF_UNDERWATER) && ( (dot < 0) ^ !!(surf->flags & SURF_PLANEBACK)) )
					continue;		// wrong side

				LightDrawPoly(surf);
			}
		}
	}

// recurse down the back side
	RecursiveDrawWorldLight (node->children[!side]);
}

void RecursiveDrawWorldLightSpot(mnode_t *node)
{
	if (node->contents == CONTENTS_SOLID)
		return;		// solid

	if (node->visframe != curvisframe)
		return;

	// buz: visible surfaces already marked
	if (node->contents < 0)
		return;

	if (current_light->spot_texture[0])
	{
		if (current_light->frustum.R_CullBox(node->minmaxs, node->minmaxs + 3)) // cull from spotlight cone
			return;
	}
	else
	{
		if (CullPointlightBBox(node->minmaxs, node->minmaxs + 3)) // cull from point light bbox
			return;
	}

	// node is just a decision point, so go down the apropriate sides
	// find which side of the node we are on
	int side;
	double dot;
	mplane_t *plane = node->plane;
	switch (plane->type)
	{
	case PLANE_X:
		dot = current_light_origin[0] - plane->dist;	break;
	case PLANE_Y:
		dot = current_light_origin[1] - plane->dist;	break;
	case PLANE_Z:
		dot = current_light_origin[2] - plane->dist;	break;
	default:
		dot = DotProduct(current_light_origin, plane->normal) - plane->dist; break;
	}

	if (dot >= 0) side = 0;
	else side = 1;

	// recurse down the children, front side first
	RecursiveDrawWorldLightSpot(node->children[side]);

	// draw stuff
	int c = node->numsurfaces;
	if (c)
	{
		model_t *world = gEngfuncs.GetEntityByIndex(0)->model;
		msurface_t *surf = world->surfaces + node->firstsurface;

		if (dot < 0 - BACKFACE_EPSILON)
			side = SURF_PLANEBACK;
		else if (dot > BACKFACE_EPSILON)
			side = 0;
		{
			for (; c; c--, surf++)
			{
				if (surf->visframe != framecount)
					continue;

				// don't backface underwater surfaces, because they warp
				if (!(surf->flags & SURF_UNDERWATER) && ((dot < 0) ^ !!(surf->flags & SURF_PLANEBACK)))
					continue;		// wrong side

				LightDrawPoly(surf,true);
			}
		}
	}

	// recurse down the back side
	RecursiveDrawWorldLightSpot(node->children[!side]);
}

extern cvar_t *cv_entdynamiclight;

//=====================================
// DrawDynamicLights
//
// called from main renderer to draw dynamic light
// on world and static entites
//=====================================
void DrawDynamicLights()
{
	//	gEngfuncs.Con_Printf("Paranoia dynamic light features are not supported by your video card!\n");
	//	gEngfuncs.Cvar_SetValue( "gl_dynlight", 0 );

	gl.glDepthFunc(GL_EQUAL);

	float time = gEngfuncs.GetClientTime();
	DynamicLight *dl = cl_dlights;
	for (int i=0 ; i<MAX_DLIGHTS ; i++, dl++)
	{
		if (dl->die < time || !dl->radius)
			continue;

		current_light = dl;
		current_light_origin = current_light->origin;
		if (current_light->spot_texture[0]) // spotlight
		{
			current_light->frustum.R_SetFrustum(current_light->angles, current_light->origin,
				current_light->cone_hor*1.5, current_light->cone_ver*1.5, current_light->radius*1.2);
			vec3_t lightangles = current_light->angles;
			FixVectorForSpotlight(lightangles);
			AngleVectors(lightangles, current_spot_vec_forward, NULL, NULL);

			// set clipping before light setup, because we should not
			// change something after the light set up
			if (current_light->key > -666) // dont set clip planes for flashlight
				current_light->frustum.GL_FrustumSetClipPlanes();

			if (!SetupSpotLight())
			{
				if (current_light->key > -666)
					current_light->frustum.GL_DisableClipPlanes();
				continue;
			}
		}
		else // pointlight
		{
			SetPointlightBBox();

			if (!SetupPointLight())
				continue;
		}

		if (current_light->spot_texture[0])
		{
			model_t* world = gEngfuncs.GetEntityByIndex(0)->model;
			RecursiveDrawWorldLightSpot(world->nodes);
		}
		else
		{
			model_t* world = gEngfuncs.GetEntityByIndex(0)->model;
			RecursiveDrawWorldLight(world->nodes);
		}

		// Draw static entites
		if (cv_entdynamiclight->value)
		{
			for (int k = 0; k < numrenderents; k++)
			{
				if (!IsEntityMoved(renderents[k]) && renderents[k]->curstate.renderfx != 70)
				{
					vec3_t mins, maxs;
					cl_entity_t *e = renderents[k];
					VectorAdd (e->origin, e->model->mins, mins);
					VectorAdd (e->origin, e->model->maxs, maxs);

					if (current_light->spot_texture[0])
					{
						if (current_light->frustum.R_CullBox(mins, maxs))
							continue;
					}
					else
					{
						if (CullPointlightBBox (mins, maxs))
							continue;
					}					

					bool spot = current_light->spot_texture[0];
					DrawEntityFacesForLight(e, spot);
				}
			}
		}

		if (current_light->spot_texture[0])
		{
			FinishSpotLight();

			if (current_light->key > -666)
				current_light->frustum.GL_DisableClipPlanes();
		}
		else
			FinishPointLight();
	}
}


/*******************************
* Other lights management -
* creation, tracking, deleting..
********************************/


//=====================================
// MY_AllocDlight
//
// Quake's func, allocates dlight
//=====================================
DynamicLight *MY_AllocDlight (int key)
{
	int		i=0;
	DynamicLight	*dl;
	float time = gEngfuncs.GetClientTime();

// first look for an exact key match
	if (key)
	{
		dl = cl_dlights;
		for (i=0 ; i<MAX_DLIGHTS ; i++, dl++)
		{
			if (dl->key == key)
			{
				memset (dl, 0, sizeof(*dl));
				dl->key = key;
				dl->depth = shadowMaps[i];
				return dl;
			}
		}
	}

// then look for anything else
	dl = cl_dlights;
	for (i=0 ; i<MAX_DLIGHTS ; i++, dl++)
	{
		if (dl->die < time)
		{
			memset (dl, 0, sizeof(*dl));
			dl->key = key;
			dl->depth = shadowMaps[i];
			return dl;
		}
	}

	dl = &cl_dlights[0];
	memset (dl, 0, sizeof(*dl));
	dl->key = key;
	dl->depth = shadowMaps[i];
	return dl;
}


void MY_DecayLights (void)
{
	static float lasttime = 0;
	float time = gEngfuncs.GetClientTime();
	float frametime = time - lasttime;
	if (frametime > 1) frametime = 1; // cap map changes..
	if (frametime < 0) frametime = 0; // 
	lasttime = time;

	DynamicLight *dl = cl_dlights;
	for (int i=0 ; i<MAX_DLIGHTS ; i++, dl++)
	{
		if (dl->die < time || !dl->radius)
			continue;
		
		dl->radius -= frametime*dl->decay;
		if (dl->radius < 0)
			dl->radius = 0;
	}
}


void ResetDynamicLights()
{
	memset (cl_dlights, 0, sizeof(cl_dlights));
}


//=====================================
// HasDynamicLights
//
// Should return TRUE if we're going to draw some dynamic lighting in this frame -
// renderer will separate lightmaps and material textures drawing
//=====================================
int HasDynamicLights()
{
	float time = gEngfuncs.GetClientTime();
	DynamicLight *dl = cl_dlights;
	for (int i=0 ; i<MAX_DLIGHTS ; i++, dl++)
	{
		if (dl->die >= time && dl->radius)
			return TRUE;
	}

	return FALSE;
}



/*******************************
*
* Attenuation textures generation
*
********************************/


int CreateSpotlightAttenuationTexture()
{
	color24 buf[256];
	for (int i = 0; i < 256; i++)
	{
		float dist = (float)i;
	//	float att = (1 - ((dist*dist) / (256*256))) * 255;
		float att = (((dist*dist) / (256*256)) - 1) * -255;

		// clear attenuation at ends to prevent light go outside
		if (i == 255 || i == 0)
			att = 0;

		buf[i].r = buf[i].g = buf[i].b = (unsigned char)att;
	}

	gl.glBindTexture(GL_TEXTURE_1D, current_ext_texture_id);
	gl.glTexImage1D (GL_TEXTURE_1D, 0, 3, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, buf);
	gl.glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	gl.glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	gl.glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	

	gEngfuncs.Con_DPrintf("Created spotlight attenuation texture\n");
	current_ext_texture_id++;
	return (current_ext_texture_id - 1);
}


#define ATTEN_3D_TEX_SIZE	32
vec3_t plightcol = Vector(1.0f, 1.0f, 1.0f); // color of 3d attenuation texture

int Create3DAttenuationTexture()
{
	if (!gl.EXT_3Dtexture_supported)
		return 0;

	color24 *buf = new color24[ATTEN_3D_TEX_SIZE*ATTEN_3D_TEX_SIZE*ATTEN_3D_TEX_SIZE];
	if (!buf)
	{
		gEngfuncs.Con_Printf("Error: Failed to allocate memory for 3d attenuation texture!\n");
		return 0;
	}

	float f = (ATTEN_3D_TEX_SIZE/2) * (ATTEN_3D_TEX_SIZE/2);

	for (int x = 0; x < ATTEN_3D_TEX_SIZE; x++)
	{
		for (int y = 0; y < ATTEN_3D_TEX_SIZE; y++)
		{
			for (int z = 0; z < ATTEN_3D_TEX_SIZE; z++)
			{
				vec3_t vec;
				vec[0] = (float)x - (ATTEN_3D_TEX_SIZE/2);
				vec[1] = (float)y - (ATTEN_3D_TEX_SIZE/2);
				vec[2] = (float)z - (ATTEN_3D_TEX_SIZE/2);
				float dist = vec.Length();
				if (dist > (ATTEN_3D_TEX_SIZE/2))
					dist = (ATTEN_3D_TEX_SIZE/2);

				float att;
				if (x==0 || y == 0 || z==0 || x==ATTEN_3D_TEX_SIZE-1 || y==ATTEN_3D_TEX_SIZE-1 || z==ATTEN_3D_TEX_SIZE-1)
					att = 0;
				else
				//	att = (1 - ((dist*dist) / f)) * 255;
					att = (((dist*dist) / f) -1 ) * -255;

				buf[x*ATTEN_3D_TEX_SIZE*ATTEN_3D_TEX_SIZE + y*ATTEN_3D_TEX_SIZE + z].r = (unsigned char)(att*plightcol[0]);
				buf[x*ATTEN_3D_TEX_SIZE*ATTEN_3D_TEX_SIZE + y*ATTEN_3D_TEX_SIZE + z].g = (unsigned char)(att*plightcol[1]);
				buf[x*ATTEN_3D_TEX_SIZE*ATTEN_3D_TEX_SIZE + y*ATTEN_3D_TEX_SIZE + z].b = (unsigned char)(att*plightcol[2]);
			}
		}
	}

	gl.glBindTexture(GL_TEXTURE_3D, current_ext_texture_id);

	gl.glTexImage3DEXT(GL_TEXTURE_3D, 0, 3,
					ATTEN_3D_TEX_SIZE, ATTEN_3D_TEX_SIZE, ATTEN_3D_TEX_SIZE,
					0, GL_RGB, GL_UNSIGNED_BYTE, buf);

	gl.glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	gl.glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	gl.glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
	gl.glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl.glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	

	delete[] buf;

	gEngfuncs.Con_DPrintf("Created 3D attenuation texture\n");
	current_ext_texture_id++;
	return (current_ext_texture_id - 1);
}



int Create2DAttenuationForOldCards()
{
//	unsigned char buf[128][128][4];
	unsigned char buf[128][128];
	for (int x = 0; x < 128; x++)
	{
		for (int y = 0; y < 128; y++)
		{
			float result;
			if (x==128 || x==0 || y==128 || y==0)
				result = 255;
			else
			{
				float xf = ((float)x - 64) / 64.0;
				float yf = ((float)y - 64) / 64.0;
				result = ((xf * xf) + (yf * yf)) * 255;
				if (result > 255) result = 255;
			}
			
		//	buf[x][y][0] = buf[x][y][1] = buf[x][y][2] = buf[x][y][3] = (unsigned char)result;
			buf[x][y] = (unsigned char)result;
		}
	}

	gl.glBindTexture(GL_TEXTURE_2D, current_ext_texture_id);
//	gl.glTexImage2D (GL_TEXTURE_2D, 0, 4, 128, 128, 0, GL_RGBA, GL_UNSIGNED_BYTE, buf);
	gl.glTexImage2D (GL_TEXTURE_2D, 0, GL_ALPHA8, 128, 128, 0, GL_ALPHA, GL_UNSIGNED_BYTE, buf);
	gl.glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	gl.glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	

	gEngfuncs.Con_DPrintf("Created 2d attenuation texture\n");
	current_ext_texture_id++;
	return (current_ext_texture_id - 1);
}


int Create1DAttenuationForOldCards()
{
//	unsigned char buf[128][4];
	unsigned char buf[128];
	for (int x = 0; x < 128; x++)
	{
		float result;
		if (x==128 || x==0)
			result = 255;
		else
		{
			float xf = ((float)x - 64) / 64.0;
			result = (xf * xf) * 255;
			if (result > 255) result = 255;
		}
		
	//	buf[x][0] = buf[x][1] = buf[x][2] = buf[x][3] = (unsigned char)result;
		buf[x] = (unsigned char)result;
	}

	gl.glBindTexture(GL_TEXTURE_1D, current_ext_texture_id);
//	gl.glTexImage1D (GL_TEXTURE_1D, 0, 4, 128, 0, GL_RGBA, GL_UNSIGNED_BYTE, buf);
	gl.glTexImage1D (GL_TEXTURE_1D, 0, GL_ALPHA8, 128, 0, GL_ALPHA, GL_UNSIGNED_BYTE, buf);
	gl.glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	gl.glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl.glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

	gEngfuncs.Con_DPrintf("Created 1d attenuation texture\n");
	current_ext_texture_id++;
	return (current_ext_texture_id - 1);
}


//================================
// CreateAttenuationTextures
//
// called from main renderer each frame
//================================
void CreateAttenuationTextures()
{
	if (!attenuation_1d)
		attenuation_1d = CreateSpotlightAttenuationTexture();

	if (!attenuation_3d)
		attenuation_3d = Create3DAttenuationTexture();

	if (!atten_point_2d)
		atten_point_2d = Create2DAttenuationForOldCards();

	if (!atten_point_1d)
		atten_point_1d = Create1DAttenuationForOldCards();
}



//================================
// GetDlightsForPoint
//
// fills array with pointers to lights, who affects this point.
// returns number of dlights written
//================================
int GetDlightsForPoint(vec3_t point, vec3_t mins, vec3_t maxs, DynamicLight **pOutArray, int alwaysFlashlight)
{
	int num = 0;
	float time = gEngfuncs.GetClientTime();
	DynamicLight *dl = cl_dlights;
	mins = point + mins;
	maxs = point + maxs;
	for (int i=0 ; i<MAX_DLIGHTS ; i++, dl++)
	{
		if (dl->die >= time && dl->radius)
		{
			if (!dl->spot_texture[0]) continue;

			if (!alwaysFlashlight || dl->key > -666) // hack for viewmodels - they always need flashlight
			{
				// check distance
				vec3_t vec = point - dl->origin;
				float squaredist = DotProduct(vec, vec);
				if ((dl->radius * dl->radius) < squaredist)
					continue;

				// make frustum check if spot light
				if (dl->spot_texture[0])
				{
					if (dl->frustum.R_CullBox (mins, maxs))
						continue;
				}

				// perform trace
				pmtrace_t pmtrace;
				gEngfuncs.pEventAPI->EV_SetTraceHull( 2 );
				gEngfuncs.pEventAPI->EV_PlayerTrace( point, dl->origin, PM_WORLD_ONLY, -1, &pmtrace );

				if (pmtrace.startsolid)
					continue;

				if (pmtrace.fraction < 1.0)
					continue; // blocked
			}

			pOutArray[num] = dl;
			num++;
		}
	}
	return num;
}

vec3_t LightForOrigin(vec3_t origin)
{
	float flRad;
	float flDist;
	float flAtten;
	float flCos;

	vec3_t vDir;
	vec3_t vNorm;
	vec3_t vForward;

	float flTime = gEngfuncs.GetClientTime();
	model_t *pWorld = gEngfuncs.GetEntityByIndex(0)->model;
	vec3_t vEndPos = origin - Vector(0, 0, 8964);
	vec3_t vColor = Vector(0, 0, 0);
	vColor = Vector(1.0,1.0,1.0);

	lighting_ext light;
	if (g_StudioRenderer.m_pCurrentEntity)
	{
		EXT_LightPoint(origin, &light, (g_StudioRenderer.m_pCurrentEntity->curstate.effects & EF_INVLIGHT));
		vColor = light.ambientlight;
		VectorMA(vColor, 0.8, light.addlight, vColor);
	}

	DynamicLight *oldCurrent = current_light;
	DynamicLight *pLight = cl_dlights;
	for (int i = 0; i < MAX_DLIGHTS; i++, pLight++)
	{
		if (pLight->die < flTime || !pLight->radius)
			continue;

		if (pLight->cone_hor)
		{
			if (pLight->frustum.R_CullBox(origin, origin))
				continue;

			vec3_t vAngles = pLight->angles;
			AngleVectors(vAngles, vForward, NULL, NULL);
		}
		else
		{
			current_light = pLight;
			SetPointlightBBox();
			if (CullPointlightBBox(origin, origin))
				continue;
		}

		flRad = pLight->radius*pLight->radius;
		VectorSubtract(origin, pLight->origin, vDir);
		flDist = DotProduct(vDir, vDir);
		flAtten = (flDist / flRad - 1)* -1;

		if (pLight->cone_hor)
		{
			VectorNormalize(vDir);
			flCos = cos((pLight->cone_hor * 2)*0.3*(M_PI * 2 / 360));
			flDist = DotProduct(vForward, vDir);

			if (flDist < 0 || flDist < flCos)
				continue;

			flAtten *= (flDist - flCos) / (1.0 - flCos);
		}

		if (flAtten <= 0)
			continue;

		VectorMA(vColor, flAtten, pLight->color, vColor);
	}

	return vColor;
}

vec3_t StaticLightDirectionForOrigin(vec3_t origin)
{
	vec3_t vDir = Vector(0, 0, 0);

	lighting_ext light;
	if (g_StudioRenderer.m_pCurrentEntity)
	{
		EXT_LightPoint(origin, &light, (g_StudioRenderer.m_pCurrentEntity->curstate.effects & EF_INVLIGHT));
		vDir = light.lightdir;
	}

	return vDir;
}

vec3_t StaticLightBaselightForOrigin(vec3_t origin)
{
	vec3_t vDir = Vector(0, 0, 0);

	lighting_ext light;
	if (g_StudioRenderer.m_pCurrentEntity)
	{
		EXT_LightPoint(origin, &light, (g_StudioRenderer.m_pCurrentEntity->curstate.effects & EF_INVLIGHT));
		vDir = light.ambientlight;
	}

	return vDir;
}

vec3_t StaticLightAddlightForOrigin(vec3_t origin)
{
	vec3_t vDir = Vector(0, 0, 0);

	lighting_ext light;
	if (g_StudioRenderer.m_pCurrentEntity)
	{
		EXT_LightPoint(origin, &light, (g_StudioRenderer.m_pCurrentEntity->curstate.effects & EF_INVLIGHT));
		vDir = light.addlight;
	}

	return vDir;
}