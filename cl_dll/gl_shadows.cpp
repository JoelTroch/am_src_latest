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
#include "gl_shaders.h"

int shadowMaps[MAX_DLIGHTS];
unsigned int m_iShadowFragmentID;
bool shadowPCFsupport;
FrustumCheck worldLightFrustum;
extern int curvisframe;

GLvoid *texcoffset = (GLvoid*)(sizeof(Vector)+sizeof(float)* 2);

char shadow_fp[] =
"!!ARBfp1.0"
"OPTION ARB_fragment_program_shadow;"
"OPTION ARB_precision_hint_fastest;"
"PARAM c[9] = {"
"{0, -0.0019},"
"{-0.0019, 0},"
"{0.0019, 0},"
"{0, 0.0019},"
"{0, -0.00108},"
"{-0.00108, 0},"
"{0.00108, 0},"
"{0, 0.00108},"
"{9, 1}};"
"TEMP R0;"
"TEMP R1;"
"RCP R0.x, fragment.texcoord[2].w;"
"MUL R0.xyz, fragment.texcoord[2], R0.x;"
"TEX R0.w, R0, texture[2], SHADOW2D;"
"ADD R1.xyz, R0, c[0];"
"TEX R1.w, R1, texture[2], SHADOW2D;"
"ADD R0.w, R1.w, R0.w;"
"ADD R1.xyz, R0, c[1];"
"TEX R1.w, R1, texture[2], SHADOW2D;"
"ADD R0.w, R1.w, R0.w;"
"ADD R1.xyz, R0, c[2];"
"TEX R1.w, R1, texture[2], SHADOW2D;"
"ADD R0.w, R1.w, R0.w;"
"ADD R1.xyz, R0, c[3];"
"TEX R1.w, R1, texture[2], SHADOW2D;"
"ADD R0.w, R1.w, R0.w;"
"ADD R1.xyz, R0, c[4];"
"TEX R1.w, R1, texture[2], SHADOW2D;"
"ADD R0.w, R1.w, R0.w;"
"ADD R1.xyz, R0, c[5];"
"TEX R1.w, R1, texture[2], SHADOW2D;"
"ADD R0.w, R1.w, R0.w;"
"ADD R1.xyz, R0, c[6];"
"TEX R1.w, R1, texture[2], SHADOW2D;"
"ADD R0.w, R1.w, R0.w;"
"ADD R1.xyz, R0, c[7];"
"TEX R1.w, R1, texture[2], SHADOW2D;"
"ADD R0.w, R1.w, R0.w;"
"RCP R1.w, c[8].x;"
"MUL R1.w, R0.w, R1.w;"
"TXP R0, fragment.texcoord[1], texture[1], 2D;"// 1 is for 0
"MUL R1, R0, R1.w;"
"TXP R0, fragment.texcoord[3], texture[3], 1D;" //3 is for 1
"MUL R1, R1, R0;"
"MUL result.color.xyz, fragment.color.primary, R1;"
"MOV result.color.w, c[8].y;"
"END";

void InitShadows()
{
	shadowPCFsupport = false;
	for (int i = 0; i < MAX_DLIGHTS; i++)
		shadowMaps[i] = GenerateShadowMap();

	if (LoadArbShader(&m_iShadowFragmentID, FRAGMENT, shadow_fp, sizeof(shadow_fp), "shadow PCF"))
		shadowPCFsupport = true;
}

void SetupPCF()
{
	if (!shadowPCFsupport)
		return;

	gl.glEnable(GL_FRAGMENT_PROGRAM_ARB);
	gl.glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, m_iShadowFragmentID);
}

void DisablePCF()
{
	if (!shadowPCFsupport)
		return;

	gl.glDisable(GL_FRAGMENT_PROGRAM_ARB);
}

void R_TranslateForEntity(cl_entity_t *e,vec3_t origin)
{
	gl.glTranslatef(e->origin[0], e->origin[1], e->origin[2]);

//	gl.glRotatef(e->angles[1], 0, 0, 1);
//	gl.glRotatef(-e->angles[0], 0, 1, 0);
//	gl.glRotatef(e->angles[2], 1, 0, 0);
}

void R_DrawBrushModelShadow(cl_entity_t *e, DynamicLight *light)
{
	//	int			j, k;
	vec3_t		mins, maxs;
	int			i;//, numsurfaces;
	msurface_t	*psurf;
	float		dot;
	mplane_t	*pplane;
	model_t		*clmodel;
	int			rotated;
	vec3_t		trans;

	currententity = e;
	clmodel = e->model;

	if (!e || !light || !e->model || !(e->model->type == mod_brush))
		return;

	if (e->angles[0] || e->angles[1] || e->angles[2])
	{
		rotated = true;
		for (i = 0; i<3; i++)
		{
			mins[i] = e->origin[i] - clmodel->radius;
			maxs[i] = e->origin[i] + clmodel->radius;
		}
	}
	else
	{
		rotated = false;
		VectorAdd(e->origin, clmodel->mins, mins);
		VectorAdd(e->origin, clmodel->maxs, maxs);
	}

	if (worldLightFrustum.R_CullBox(mins, maxs))
		return;

	VectorSubtract(light->origin, e->origin, vec_to_eyes);

	if (rotated)
	{
		vec3_t	temp;
		vec3_t	forward, right, up;

		VectorCopy(vec_to_eyes, temp);
		AngleVectors(e->angles, forward, right, up);
		vec_to_eyes[0] = DotProduct(temp, forward);
		vec_to_eyes[1] = -DotProduct(temp, right);
		vec_to_eyes[2] = DotProduct(temp, up);
	}

	psurf = &clmodel->surfaces[clmodel->firstmodelsurface];

	gl.glPushMatrix();

	if (rotated)
	{
		e->angles[0] = -e->angles[0];	// stupid quake bug
		R_RotateForEntity(e);
		e->angles[0] = -e->angles[0];	// stupid quake bug
	}
	else
	{
		R_TranslateForEntity(e, vec_to_eyes);
	}

	SetTexEnvs(ENVSTATE_REPLACE);
	SetTexPointer(0, TC_TEXTURE);

	if (e->curstate.rendermode == kRenderTransAlpha)
	{
		gl.glEnable(GL_ALPHA_TEST);
		gl.glAlphaFunc(GL_GREATER, 0.25);
	}
	else if (e->curstate.rendermode == kRenderTransTexture)
	{
		//gl.glDepthMask(GL_FALSE);
		gl.glEnable(GL_BLEND);
		gl.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		gl.glColor4ub(255, 255, 255, e->curstate.renderamt);
	}
	else if (e->curstate.rendermode == kRenderTransAdd)
	{
		gl.glDepthMask(GL_FALSE);
	}

	for (i = 0; i<clmodel->nummodelsurfaces; i++, psurf++)
	{
		pplane = psurf->plane;
		dot = DotProduct(vec_to_eyes, pplane->normal) - pplane->dist;

		bool frontFace = ((psurf->flags & SURF_PLANEBACK) && (dot < -BACKFACE_EPSILON)) ||
			(!(psurf->flags & SURF_PLANEBACK) && (dot > BACKFACE_EPSILON));

		if (frontFace || e->curstate.rendermode == kRenderTransTexture)
		{
			psurf->visframe = framecount;

			if (!psurf->texinfo || !psurf->texinfo->texture)
				continue;

			if (psurf->flags & SURF_DRAWSKY)
				continue;

			if (psurf->flags & SURF_DRAWTURB)
				continue;

			Bind2DTexture(GL_TEXTURE0_ARB, psurf->texinfo->texture->gl_texturenum);
			DrawPolyFromArray(psurf->polys);
		}
	}

	if (e->curstate.rendermode == kRenderTransAlpha)
	{
		gl.glDisable(GL_ALPHA_TEST);
	}
	else if (e->curstate.rendermode == kRenderTransTexture || e->curstate.rendermode == kRenderTransAdd)
	{
		gl.glDepthMask(GL_TRUE);
		gl.glDisable(GL_BLEND);
	}

	SetTexEnvs(ENVSTATE_OFF);
	SetTexPointer(0, TC_LIGHTMAP);

	gl.glPopMatrix();
}

void RecursiveDrawWorldShadow(mnode_t *node)
{
	mleaf_t		*pleaf;
	int c = 0;
	int side;
	double dot;
	mplane_t *plane = node->plane;
	msurface_t	*surf, **mark;

	if (node->contents == CONTENTS_SOLID)
		return;		// solid

	if (node->visframe != curvisframe)
		return;

	if (worldLightFrustum.R_CullBox(node->minmaxs, node->minmaxs + 3)) // cull from spotlight cone
		return;

	// buz: visible surfaces already marked
	if (node->contents < 0)
	{
		pleaf = (mleaf_t *)node;
		mark = pleaf->firstmarksurface;
		c = pleaf->nummarksurfaces;

		if (c)
		{
			do
			{
				(*mark)->visframe = framecount;
				mark++;
			} while (--c);
		}
		return;
	}

	// node is just a decision point, so go down the apropriate sides
	// find which side of the node we are on
	plane = node->plane;
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
	RecursiveDrawWorldShadow(node->children[side]);

	// draw stuff
	c = node->numsurfaces;
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

				gl.glColor4f(1, 1, 0, 1);
				LightDrawPoly(surf, true);
			}
		}
	}

	// recurse down the back side
	RecursiveDrawWorldShadow(node->children[!side]);
}

extern CGameStudioModelRenderer g_StudioRenderer;

int r_studiopoly;

void MultiplyMatrix4x4(float *a, float *b, float *o)
{
	for (int i = 0; i < 16; i++)
		o[i] = a[i] * b[i];
}

void DrawShadowForLight(DynamicLight *l)
{

	float flProj[16];
	float flModel[16];

	float newProj[16];
	float newMView[16];
	float finalProj[16];

	vec3_t light_forward;
	vec3_t vAngles = l->angles;
	FixVectorForSpotlight(vAngles);

	vec3_t vUp, vRight;
	float width = tan((M_PI / 360) * l->cone_hor);
	float height = tan((M_PI / 360) * l->cone_ver);
	float nearplane = 1;
	int visFrame = curvisframe;
	cl_entity_t *olde;

	// Save matrixes.
	gl.glGetFloatv(GL_PROJECTION_MATRIX, flProj);
	gl.glGetFloatv(GL_MODELVIEW_MATRIX, flModel);

	//disable z-fighting.
	gl.glPolygonOffset(5, 0);
	gl.glEnable(GL_POLYGON_OFFSET_FILL);

	gl.glClearColor(GL_ZERO, GL_ZERO, GL_ZERO, GL_ZERO);
	gl.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_ACCUM_BUFFER_BIT);
	//	gl.glColorMask(GL_ZERO, GL_ZERO, GL_ZERO, GL_ZERO);

	gl.glCullFace(GL_FRONT);
	gl.glDisable(GL_BLEND);
	gl.glDisable(GL_FOG);
	gl.glShadeModel(GL_FLAT);

	gl.glViewport(GL_ZERO, GL_ZERO, gl_shadowsize->value, gl_shadowsize->value);


	float flSize = tan((M_PI / 360) * l->cone_hor);
	float flFrustum[] = { 2 / (flSize * 2), 0, 0, 0, 0, 2 / (flSize * 2), 0, 0, 0, 0, -1, -1, 0, 0, -2, 0 };

	gl.glMatrixMode(GL_PROJECTION);
	gl.glLoadIdentity();
	gl.glFrustum(-width, width, -height, height, nearplane, l->radius);

	FixVectorForSpotlight(vAngles);
	AngleVectors(vAngles, light_forward, vRight, vUp);

	int bReversed = IsPitchReversed(vAngles[PITCH]);
	vec3_t vTarget = l->origin + (light_forward * l->radius);

	gl.glMatrixMode(GL_MODELVIEW);
	gl.glLoadIdentity();
	gl.gluLookAt(l->origin[0], l->origin[1], l->origin[2], vTarget[0], vTarget[1], vTarget[2], 0, 0, bReversed ? -1 : 1);

	gl.glGetFloatv(GL_PROJECTION_MATRIX, newProj);
	gl.glGetFloatv(GL_MODELVIEW_MATRIX, newMView);

	l->cMV = flModel;
	l->cP = flProj;
	l->lMV = newMView;
	l->lP = newProj;

	MultiplyMatrix4x4(newProj, newMView, finalProj);
	MultiplyMatrix4x4(finalProj, flProj, finalProj);

	gl.glMatrixMode(GL_PROJECTION);
	gl.glMatrixMode(GL_MODELVIEW);

	current_light = l;
	current_light_origin = l->origin;
	current_spot_vec_forward = light_forward;

	cl_entity_t *localPlayer = gEngfuncs.GetLocalPlayer();
	if (!localPlayer)
		return;

	int curmsgnum = localPlayer->curstate.messagenum;

	model_t *world = gEngfuncs.GetEntityByIndex(0)->model;
	mleaf_t *leaf = Mod_PointInLeaf(l->origin, world);
	curvisframe = leaf->visframe;
	
	gl.SaveStates();
	int b, ib;
	gl.glGetIntegerv(GL_ARRAY_BUFFER_BINDING_ARB, &b);
	gl.glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING_ARB, &ib);

	int ssize, sstride, stype;
	gl.glGetIntegerv(GL_TEXTURE_COORD_ARRAY_SIZE, &ssize);
	gl.glGetIntegerv(GL_TEXTURE_COORD_ARRAY_STRIDE, &sstride);
	gl.glGetIntegerv(GL_TEXTURE_COORD_ARRAY_TYPE, &stype);
	
	olde = currententity;
	for (int i = 0; i < 2048; i++)
	{
		cl_entity_t *ent = gEngfuncs.GetEntityByIndex(i);

		if (!ent || !ent->model || ent == gEngfuncs.GetViewModel())
			continue;

		if (ent->curstate.messagenum != curmsgnum)
			continue;

		if (ent->model->type == mod_studio && !ent->player && !(ent->curstate.effects & EF_NODRAW))
		{
			g_StudioRenderer.m_pCurrentEntity = ent;
			for (int i = 0; i < 3; i++)
			{
				g_StudioRenderer.m_vRenderOrigin[i] = l->origin[i];
				g_StudioRenderer.m_vNormal[i] = current_spot_vec_forward[i];
				g_StudioRenderer.m_vUp[i] = vUp[i];
				g_StudioRenderer.m_vRight[i] = vRight[i];
			}

			g_StudioRenderer.StudioDrawModel(STUDIO_SHADOWS, ent);
		}
	}
	g_StudioRenderer.m_pCurrentEntity = olde;
	
	gl.glBindBufferARB(GL_ARRAY_BUFFER_ARB, b);
	gl.glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, ib);
	gl.RestoreStates();

	gl.glTexCoordPointer(ssize, stype, sstride, texcoffset);
	
	void EnableVertexArray();
	EnableVertexArray();
	gl.glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	SetTexEnvs(ENVSTATE_OFF, ENVSTATE_OFF, ENVSTATE_OFF, ENVSTATE_OFF);
	gl.glActiveTextureARB(GL_TEXTURE0_ARB);

	olde = currententity;
	for (int k = 0; k < numrenderents; k++)
	{
		if (!IsEntityMoved(renderents[k]))
			R_DrawBrushModelShadow(renderents[k], current_light);
	}
	currententity = olde;

	RecursiveDrawWorldShadow(world->nodes);
	curvisframe = visFrame;
	r_studiopoly = 0;


	gl.glBindTexture(GL_TEXTURE_2D, l->depth);
	gl.glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 0, 0, gl_shadowsize->value, gl_shadowsize->value, 0);
	gl.glViewport(GL_ZERO, GL_ZERO, ScreenWidth, ScreenHeight);
	gl.glColorMask(GL_ONE, GL_ONE, GL_ONE, GL_ONE);

	gl.glDisable(GL_POLYGON_OFFSET_FILL);

	gl.glMatrixMode(GL_PROJECTION);
	gl.glLoadMatrixf(flProj);

	gl.glMatrixMode(GL_MODELVIEW);
	gl.glLoadMatrixf(flModel);

	gl.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_ACCUM_BUFFER_BIT);
}
void DrawLightShadows()
{

	float time = gEngfuncs.GetClientTime();
	DynamicLight *dl = cl_dlights;
	for (int i = 0; i<MAX_DLIGHTS; i++, dl++)
	{
		if (dl->die < time || !dl->radius)
			continue;

		if (dl->spot_texture[0]) // spotlight
		{
			dl->frustum.R_SetFrustum(dl->angles, dl->origin,
				dl->cone_hor, dl->cone_hor, dl->radius);
			worldLightFrustum = dl->frustum;

			DrawShadowForLight(dl);
		}
	}
}

void FixVectorForSpotlight(vec3_t &vec)
{
	if (vec[YAW] == 0) vec[YAW] = 1;
	if (vec[YAW] == 90) vec[YAW] = 89;
	if (vec[YAW] == 180) vec[YAW] = 179;
	if (vec[YAW] == 270) vec[YAW] = 269;
	if (vec[YAW] == -90) vec[YAW] = -89;
	if (vec[YAW] == -180) vec[YAW] = -179;
	if (vec[YAW] == -270) vec[YAW] = -269;

	if (vec[ROLL] == 0) vec[ROLL] = 1;
	if (vec[ROLL] == 90) vec[ROLL] = 89;
	if (vec[ROLL] == 180) vec[ROLL] = 179;
	if (vec[ROLL] == 270) vec[ROLL] = 269;
	if (vec[ROLL] == -90) vec[ROLL] = -89;
	if (vec[ROLL] == -180) vec[ROLL] = -179;
	if (vec[ROLL] == -270) vec[ROLL] = -269;

	if (vec[PITCH] == 0) vec[PITCH] = 1;
	if (vec[PITCH] == 90) vec[PITCH] = 89;
	if (vec[PITCH] == 180) vec[PITCH] = 179;
	if (vec[PITCH] == 270) vec[PITCH] = 269;
	if (vec[PITCH] == -90) vec[PITCH] = -89;
	if (vec[PITCH] == -180) vec[PITCH] = -179;
	if (vec[PITCH] == -270) vec[PITCH] = -269;

}