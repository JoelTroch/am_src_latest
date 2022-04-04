//
// written by BUzer for HL: Paranoia modification
//
//		2006

#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "com_model.h"
#include "studio.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "dlight.h"
#include "triangleapi.h"

#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <math.h>

#include "studio_util.h"
#include "r_studioint.h"

#include "StudioModelRenderer.h"
#include "GameStudioModelRenderer.h"

#include "gl_renderer.h"
#include "gl_dlight.h"

#include "gl_studiovbo.h"
#include "gl_shaders.h"

#include "pm_defs.h"
#include "pmtrace.h"

extern engine_studio_api_t IEngineStudio;
#define CONPRINT gEngfuncs.Con_Printf

//
// Custom model rendering, based on modelviewer's code
//

vec3_t	g_xformverts[MAXSTUDIOVERTS];	// transformed vertices
vec3_t	g_lightvalues[MAXSTUDIOVERTS];	// light surface normals
vec3_t	g_normalvalues[MAXSTUDIOVERTS];	// surface normals

vec3_t	*g_pxformverts;
vec3_t	*g_pvlightvalues;
vec3_t	*g_pnvalues;

extern cvar_t *cv_cubemap;

//vec3_t	g_lightvec;						// light vector in model reference frame
vec3_t	g_blightvec[MAXSTUDIOBONES];	// light vectors in bone reference frames
//int		g_ambientlight;					// ambient world light
//float	g_shadelight;					// direct world light
//vec3_t	g_lightcolor;
lighting_ext light; // buz

// buz: dynamic lighting
vec3_t	g_bdynlightvec[MAX_MODEL_DYNLIGHTS][MAXSTUDIOBONES]; // light vectors in bone reference frames
vec3_t	g_dynlightcolor[MAX_MODEL_DYNLIGHTS];
int		g_numdynlights;

//vec3_t	g_vright;		// needs to be set to viewer's right in order for chrome to work
float	alpha = 1; // buz: current alpha
int		useblending = FALSE;

#include "gl_texloader.h"

extern int current_ext_texture_id;

char *lowcase(char *string);
void pathtofilename(char *in, char *out);
void filenamewoextention(char *in, char *out);

//================================================
// GL_SetupTextureHeader
//
// gets access to external textures for model
// thanks to XaeroX
//================================================
void CStudioModelRenderer::GL_SetupTextureHeader()
{
	//Get texture header
	if (!m_pStudioHeader->numtextures || !m_pStudioHeader->textureindex)
	{
		char texturename[256];

		strcpy(texturename, m_pRenderModel->name);
		strcpy(&texturename[strlen(texturename) - 4], "T.mdl");

		model_t *textures = IEngineStudio.Mod_ForName(texturename, 0);
		if (!textures)
		{
			m_pTextureHeader = NULL;
			CONPRINT("Mod_ForName: failed for %s\n", texturename);
			return;
		}

		m_pTextureHeader = (studiohdr_t *)IEngineStudio.Mod_Extradata(textures);
		if (!m_pTextureHeader)
			CONPRINT("Mod_Extradata: failed for %s\n", texturename);
	}
	else
	{
		m_pTextureHeader = m_pStudioHeader;
	}
}

//================================================
// GL_SetupTextureHeader
//
// gets access to external textures for model
// thanks to XaeroX
//================================================
void CStudioModelRenderer::GL_SetupRenderer(int rendermode)
{
	g_pxformverts = &g_xformverts[0];
	g_pvlightvalues = &g_lightvalues[0];

	g_pnvalues = &g_normalvalues[0];

	/*	gl.glPushMatrix ();

	gl.glTranslatef (m_pCurrentEntity->origin[0], m_pCurrentEntity->origin[1], m_pCurrentEntity->origin[2]);

	gl.glRotatef (m_pCurrentEntity->angles[1],  0, 0, 1);
	gl.glRotatef (m_pCurrentEntity->angles[0],  0, 1, 0);
	gl.glRotatef (m_pCurrentEntity->angles[2],  1, 0, 0);*/

	//	GL_SetupTextureHeader();

	gl.glShadeModel(GL_SMOOTH);
	//	gl.glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	alpha = 1;
	useblending = FALSE;
	if (rendermode == kRenderTransAdd)
	{
		gl.glDepthMask(GL_FALSE);
		gl.glEnable(GL_BLEND);
		gl.glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		alpha = (float)m_pCurrentEntity->curstate.renderamt / 255.0;
		useblending = TRUE;
	}
	else if (rendermode == kRenderTransAlpha)
	{
		gl.glDepthMask(GL_FALSE);
		gl.glEnable(GL_BLEND);
		gl.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		alpha = (float)m_pCurrentEntity->curstate.renderamt / 255.0;
		useblending = TRUE;
	}

	// buz: DAMN! another texture_env_combine bug! It works perfectly until
	// blending is enabled. When i turning on blending, the things becomes much darker!
	// Maybe driver bug, i dunno.
	// On register combiners everything works fine in any case, so let it use
	// register combiners if they supported..

	if (!gl.NV_combiners_supported)
	{
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_MODULATE);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_TEXTURE);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_PRIMARY_COLOR_ARB);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 2);

		/*	gl.glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, GL_MODULATE);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB, GL_TEXTURE);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_ARB, GL_SRC_ALPHA);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_ARB, GL_PRIMARY_COLOR_ARB);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA_ARB, GL_SRC_ALPHA);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_ALPHA_SCALE, 1);*/
	}
	else
	{
		gl.glEnable(GL_REGISTER_COMBINERS_NV);
		gl.glCombinerParameteriNV(GL_NUM_GENERAL_COMBINERS_NV, 1);

		// RC 0 setup:
		//	spare0.rgb = (tex0.rgb * primary_color.rgb) * 2
		gl.glCombinerInputNV(GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_A_NV, GL_TEXTURE0_ARB,
			GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		gl.glCombinerInputNV(GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_B_NV, GL_PRIMARY_COLOR_NV,
			GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		gl.glCombinerOutputNV(GL_COMBINER0_NV, GL_RGB,
			GL_SPARE0_NV,          // AB output
			GL_DISCARD_NV,         // CD output
			GL_DISCARD_NV,         // sum output
			GL_SCALE_BY_TWO_NV, GL_NONE,
			GL_FALSE,               // AB = A * B
			GL_FALSE, GL_FALSE);

		//	spare0.a = tex0.a * primary_color.a
		gl.glCombinerInputNV(GL_COMBINER0_NV, GL_ALPHA, GL_VARIABLE_A_NV, GL_TEXTURE0_ARB,
			GL_UNSIGNED_IDENTITY_NV, GL_ALPHA);
		gl.glCombinerInputNV(GL_COMBINER0_NV, GL_ALPHA, GL_VARIABLE_B_NV, GL_PRIMARY_COLOR_NV,
			GL_UNSIGNED_IDENTITY_NV, GL_ALPHA);
		gl.glCombinerOutputNV(GL_COMBINER0_NV, GL_ALPHA,
			GL_SPARE0_NV,          // AB output
			GL_DISCARD_NV,         // CD output
			GL_DISCARD_NV,         // sum output
			GL_NONE, GL_NONE,
			GL_FALSE,
			GL_FALSE, GL_FALSE);

		// Final RC setup:
		//	out.rgb = spare0.rgb
		//	out.a = spare0.a
		gl.glFinalCombinerInputNV(GL_VARIABLE_A_NV, GL_ZERO,
			GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		gl.glFinalCombinerInputNV(GL_VARIABLE_B_NV, GL_ZERO,
			GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		gl.glFinalCombinerInputNV(GL_VARIABLE_C_NV, GL_ZERO,
			GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		gl.glFinalCombinerInputNV(GL_VARIABLE_D_NV, GL_SPARE0_NV,
			GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		gl.glFinalCombinerInputNV(GL_VARIABLE_E_NV, GL_ZERO,
			GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		gl.glFinalCombinerInputNV(GL_VARIABLE_F_NV, GL_ZERO,
			GL_UNSIGNED_IDENTITY_NV, GL_RGB);
		gl.glFinalCombinerInputNV(GL_VARIABLE_G_NV, GL_SPARE0_NV,
			GL_UNSIGNED_IDENTITY_NV, GL_ALPHA);
	}
}

void CStudioModelRenderer::GL_RestoreRenderer()
{
	//	gl.glPopMatrix ();

	if (!gl.NV_combiners_supported)
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1);
	else
		gl.glDisable(GL_REGISTER_COMBINERS_NV);

	gl.glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	gl.glShadeModel(GL_FLAT);
	//	glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	if (useblending)
	{
		gl.glDepthMask(GL_TRUE);
		gl.glDisable(GL_BLEND);
	}
}


// =================================================================

void VectorRotate(const vec3_t &in1, const float in2[3][4], vec3_t &out)
{
	out[0] = DotProduct(in1, in2[0]);
	out[1] = DotProduct(in1, in2[1]);
	out[2] = DotProduct(in1, in2[2]);
}


// rotate by the inverse of the matrix
void VectorIRotate(const vec3_t &in1, const float in2[3][4], vec3_t &out)
{
	out[0] = in1[0] * in2[0][0] + in1[1] * in2[1][0] + in1[2] * in2[2][0];
	out[1] = in1[0] * in2[0][1] + in1[1] * in2[1][1] + in1[2] * in2[2][1];
	out[2] = in1[0] * in2[0][2] + in1[1] * in2[1][2] + in1[2] * in2[2][2];
}
#define VectorMaximum(a) ( max( (a)[0], max( (a)[1], (a)[2] ) ) )

/*
================
StudioModel::SetupLighting
set some global variables based on entity position
inputs:
outputs:
g_ambientlight
g_shadelight
================
*/

void CStudioModelRenderer::GL_SetupLighting()
{
	//HAWK no more cpu lighting.
	return;

	vec3_t center = (m_pCurrentEntity->curstate.mins + m_pCurrentEntity->curstate.maxs) / 2;
	vec3_t point = m_pCurrentEntity->origin + center;

	EXT_LightPoint(point, &light, (m_pCurrentEntity->curstate.effects & EF_INVLIGHT));

	// TODO: only do it for bones that actually have textures
	for (int i = 0; i < m_pStudioHeader->numbones; i++)
	{
		//	VectorIRotate( g_lightvec, (*m_pbonetransform)[i], g_blightvec[i] );
		VectorIRotate(light.lightdir, (*m_pbonetransform)[i], g_blightvec[i]);
	}

	//
	// buz: Dynamic lighting
	//

	//	CONPRINT("model: %s\n", m_pRenderModel->name);

	g_numdynlights = 0;
	DynamicLight* dlights[32];

	int isViewModel = (m_pCurrentEntity == gEngfuncs.GetViewModel()) ? TRUE : FALSE;

	int numlights;
	numlights = GetDlightsForPoint(m_pCurrentEntity->origin,
		m_pCurrentEntity->curstate.mins, m_pCurrentEntity->curstate.maxs, dlights, isViewModel);


	for (int i = 0; i < numlights; i++)
	{
		if (g_numdynlights == MAX_MODEL_DYNLIGHTS)
			break;

		vec3_t forward; // light forward for spotlights check
		float spotcos;
		if (dlights[i]->spot_texture[0])
		{
			AngleVectors(dlights[i]->angles, forward, NULL, NULL);
			spotcos = cos((dlights[i]->cone_hor)*0.3*(M_PI * 2 / 360));
		}

		float r = dlights[i]->radius * dlights[i]->radius; // squared radius

		// use special hack view models when they lit by flashlight -
		// light should come from back of model, angles independent.
		// This just looks cool.
		int isViewModelHack = FALSE;
		vec3_t alternateOrigin;
		alternateOrigin = Vector(0, 0, 0);

		for (int b = 0; b < m_pStudioHeader->numbones; b++)
		{
			vec3_t vec;
			if (isViewModelHack)
			{
				vec[0] = (*m_pbonetransform)[b][0][3] - alternateOrigin[0];
				vec[1] = (*m_pbonetransform)[b][1][3] - alternateOrigin[1];
				vec[2] = (*m_pbonetransform)[b][2][3] - alternateOrigin[2];
			}
			else
			{
				vec[0] = (*m_pbonetransform)[b][0][3] - dlights[i]->origin[0];
				vec[1] = (*m_pbonetransform)[b][1][3] - dlights[i]->origin[1];
				vec[2] = (*m_pbonetransform)[b][2][3] - dlights[i]->origin[2];
			}

			float dist = DotProduct(vec, vec);
			float atten = (dist / r - 1) * -1;
			if (atten < 0) atten = 0;
			dist = sqrt(dist);
			if (dist)
			{
				dist = 1 / dist;
				vec[0] *= dist;
				vec[1] *= dist;
				vec[2] *= dist;
			}

			VectorIRotate(vec, (*m_pbonetransform)[b], g_bdynlightvec[g_numdynlights][b]);
			VectorScale(g_bdynlightvec[g_numdynlights][b], atten, g_bdynlightvec[g_numdynlights][b]);

			if (dlights[i]->spot_texture[0])
			{
				// calc spotlight cone
				atten = DotProduct(forward, vec);
				if (atten < spotcos)
					VectorClear(g_bdynlightvec[g_numdynlights][b]);
				else
					VectorScale(g_bdynlightvec[g_numdynlights][b], (atten - spotcos) / (1 - spotcos), g_bdynlightvec[g_numdynlights][b]);
			}
		}

		g_dynlightcolor[g_numdynlights][0] = ApplyGamma(dlights[i]->color[0]) / 2;
		g_dynlightcolor[g_numdynlights][1] = ApplyGamma(dlights[i]->color[1]) / 2;
		g_dynlightcolor[g_numdynlights][2] = ApplyGamma(dlights[i]->color[2]) / 2;
		g_numdynlights++;
	}
}

void CStudioModelRenderer::SetupShaderLighting()
{
	int dynlights = 0;
	DynamicLight* dlights[32];
	float shaderData[45];

	int numlights = 0;
	numlights = GetDlightsForPoint(m_pCurrentEntity->origin,
		m_pCurrentEntity->curstate.mins, m_pCurrentEntity->curstate.maxs, dlights, false);

	if (!numlights)
	{
		shaderData[0] = 0.0;
		if (studioShader){
			gl.glUniform1fvARB(studioShader->dynlight, 45, &shaderData[0]);
			gl.glUniform3fARB(studioShader->skycolor,
				ApplyGamma(sv_skycolor_r->value / 255.0),
				ApplyGamma(sv_skycolor_g->value / 255.0),
				ApplyGamma(sv_skycolor_b->value / 255.0));

			gl.glUniform3fARB(studioShader->skyvec,
				sv_skyvec_x->value,
				sv_skyvec_y->value,
				sv_skyvec_z->value);
		}
		return;
	}

	shaderData[0] = (float)numlights;

	for (int i = 0; i < numlights; i++)
	{
		if (dynlights == MAX_MODEL_DYNLIGHTS)
			break;

		vec3_t forward; // light forward for spotlights check
		float spotcos;
		if (dlights[i]->spot_texture[0])
		{
			AngleVectors(dlights[i]->angles, forward, NULL, NULL);
			shaderData[dynlights * 11 + 1] = forward.x;
			shaderData[dynlights * 11 + 2] = forward.y;
			shaderData[dynlights * 11 + 3] = forward.z;

			spotcos = cos((dlights[i]->cone_hor + dlights[i]->cone_ver)*0.3*(M_PI * 2 / 360));
			shaderData[dynlights * 11 + 10] = spotcos;
		}
		else continue;//only spotlights are allowed

		float r = dlights[i]->radius * dlights[i]->radius; // squared radius
		shaderData[dynlights * 11 + 11] = r;

		shaderData[dynlights * 11 + 4] = dlights[i]->origin[0];
		shaderData[dynlights * 11 + 5] = dlights[i]->origin[1];
		shaderData[dynlights * 11 + 6] = dlights[i]->origin[2];

		shaderData[dynlights * 11 + 7] = (float)(ApplyGamma(dlights[i]->color[0]) / 2.0);
		shaderData[dynlights * 11 + 8] = (float)(ApplyGamma(dlights[i]->color[1]) / 2.0);
		shaderData[dynlights * 11 + 9] = (float)(ApplyGamma(dlights[i]->color[2]) / 2.0);
		dynlights++;
	}

	if (studioShader)
	{
		gl.glUniform1fvARB(studioShader->dynlight, 45, &shaderData[0]);

		gl.glUniform3fARB(studioShader->skycolor,
			ApplyGamma(sv_skycolor_r->value / 255.0),
			ApplyGamma(sv_skycolor_g->value / 255.0),
			ApplyGamma(sv_skycolor_b->value / 255.0));

		gl.glUniform3fARB(studioShader->skyvec,
			sv_skyvec_x->value,
			sv_skyvec_y->value,
			sv_skyvec_z->value);
	}
}

void CStudioModelRenderer::SetupShaderLightingProp()
{
	int dynlights = 0;
	DynamicLight* dlights[32];
	float shaderData[45];

	int numlights = 0;
	numlights = GetDlightsForPoint(m_pCurrentEntity->origin,
		m_pCurrentEntity->curstate.mins, m_pCurrentEntity->curstate.maxs, dlights, false);

	if (!numlights)
	{
		shaderData[0] = 0.0;
		if (studioShaderProp){
			gl.glUniform1fvARB(studioShaderProp->dynlight, 45, &shaderData[0]);
			gl.glUniform3fARB(studioShaderProp->skycolor,
				ApplyGamma(sv_skycolor_r->value / 255.0),
				ApplyGamma(sv_skycolor_g->value / 255.0),
				ApplyGamma(sv_skycolor_b->value / 255.0));

			gl.glUniform3fARB(studioShaderProp->skyvec,
				sv_skyvec_x->value,
				sv_skyvec_y->value,
				sv_skyvec_z->value);
		}
		return;
	}

	shaderData[0] = (float)numlights;

	for (int i = 0; i < numlights; i++)
	{
		if (dynlights == MAX_MODEL_DYNLIGHTS)
			break;

		vec3_t forward; // light forward for spotlights check
		float spotcos;
		if (dlights[i]->spot_texture[0])
		{
			AngleVectors(dlights[i]->angles, forward, NULL, NULL);
			shaderData[i * 11 + 1] = forward.x;
			shaderData[i * 11 + 2] = forward.y;
			shaderData[i * 11 + 3] = forward.z;

			spotcos = cos((dlights[i]->cone_hor + dlights[i]->cone_ver)*0.3*(M_PI * 2 / 360));
			shaderData[i * 11 + 10] = spotcos;
		}
		else continue;//only spotlights are allowed

		float r = dlights[i]->radius * dlights[i]->radius; // squared radius
		shaderData[i * 11 + 11] = r;

		shaderData[i * 11 + 4] = dlights[i]->origin[0];
		shaderData[i * 11 + 5] = dlights[i]->origin[1];
		shaderData[i * 11 + 6] = dlights[i]->origin[2];

		shaderData[i * 11 + 7] = (float)(ApplyGamma(dlights[i]->color[0]) / 2.0);
		shaderData[i * 11 + 8] = (float)(ApplyGamma(dlights[i]->color[1]) / 2.0);
		shaderData[i * 11 + 9] = (float)(ApplyGamma(dlights[i]->color[2]) / 2.0);
		dynlights++;
	}

	if (studioShaderProp)
	{
		gl.glUniform1fvARB(studioShaderProp->dynlight, 45, &shaderData[0]);

		gl.glUniform3fARB(studioShaderProp->skycolor,
			ApplyGamma(sv_skycolor_r->value / 255.0),
			ApplyGamma(sv_skycolor_g->value / 255.0),
			ApplyGamma(sv_skycolor_b->value / 255.0));

		gl.glUniform3fARB(studioShaderProp->skyvec,
			sv_skyvec_x->value,
			sv_skyvec_y->value,
			sv_skyvec_z->value);
	}
}

/*
=================
StudioModel::SetupModel
based on the body part, figure out which mesh it should be using.
inputs:
currententity
outputs:
pstudiomesh
pmdl
=================
*/
void CStudioModelRenderer::GL_SetupModel(int bodypart)
{
	int index;

	if (bodypart > m_pStudioHeader->numbodyparts)
	{
		CONPRINT("SetupModel: no such bodypart %d\n", bodypart);
		bodypart = 0;
	}

	m_pBodyPart = (mstudiobodyparts_t *)((byte *)m_pStudioHeader + m_pStudioHeader->bodypartindex) + bodypart;

	index = m_pCurrentEntity->curstate.body / m_pBodyPart->base;
	index = index % m_pBodyPart->nummodels;

	m_pSubModel = (mstudiomodel_t *)((byte *)m_pStudioHeader + m_pBodyPart->modelindex) + index;
}

void transpose(const float *m1, float *m2)
{
	m2[0] = m1[0];  m2[4] = m1[1];  m2[8] = m1[2];  m2[12] = m1[3];
	m2[1] = m1[4];  m2[5] = m1[5];  m2[9] = m1[6];  m2[13] = m1[7];
	m2[2] = m1[8];  m2[6] = m1[9];  m2[10] = m1[10]; m2[14] = m1[11];
	m2[3] = m1[12]; m2[7] = m1[13]; m2[11] = m1[14]; m2[15] = m1[15];
}

// buz: colored
// hawk qmark
void CStudioModelRenderer::Lighting(float *lv, int bone, int flags, vec3_t normal)
{
	vec3_t 	illum;
	float	lightcos;

	illum = light.ambientlight;

	if (flags & STUDIO_NF_FLATSHADE)
	{
		VectorMASSE(illum, 0.8, light.addlight, illum);
	}
	else
	{
		float r;
		DotProductSSE(&lightcos, normal, g_blightvec[bone]);

		if (lightcos > 1.0)
			lightcos = 1;

		VectorAddSSE(illum, light.addlight, illum);

		r = cv_lambert->value;
		if (r <= 1.0) r = 1.0;

		lightcos = (lightcos + (r - 1.0)) / r; 		// do modified hemispherical lighting
		if (lightcos > 0.0)
		{
			VectorMASSE(illum, -lightcos, light.addlight, illum);
		}

		if (illum[0] <= 0) illum[0] = 0;
		if (illum[1] <= 0) illum[1] = 0;
		if (illum[2] <= 0) illum[2] = 0;

		// buz: now add all dynamic lights
		for (int i = 0; i < g_numdynlights; i++)
		{
			DotProductSSE(&lightcos, normal, g_bdynlightvec[i][bone]);
			lightcos = -lightcos;

			if (lightcos > 0)
				VectorMASSE(illum, lightcos, g_dynlightcolor[i], illum);
		}
	}

	float max = VectorMaximum(illum);
	if (max > 1.0)
	{
		float scale = 1.0 / max;

		illum[0] *= scale;
		illum[1] *= scale;
		illum[2] *= scale;
	}

	lv[0] = illum[0];
	lv[1] = illum[1];
	lv[2] = illum[2];

}
int iSkinID = 0;
bool needsChange = false;
char szToken[512];
extern vec3_t render_origin;
void CStudioModelRenderer::GL_DrawPoints()
{
	if (!m_pCurrentEntity)
		return;
	
	byte *pvertbone = ((byte *)m_pStudioHeader + m_pSubModel->vertinfoindex);
	byte *pnormbone = ((byte *)m_pStudioHeader + m_pSubModel->norminfoindex);

	if (!m_pTextureHeader)
		return;

	if (m_pCurrentEntity->curstate.renderfx == 60) // light tester entity
		return;

	if (!m_pCurrentEntity->model)
		return;

	mstudiotexture_t *ptexture = (mstudiotexture_t *)((byte *)m_pTextureHeader + m_pTextureHeader->textureindex);
	mstudiomesh_t *pmesh = (mstudiomesh_t *)((byte *)m_pStudioHeader + m_pSubModel->meshindex);

	vec3_t *pstudioverts = (vec3_t *)((byte *)m_pStudioHeader + m_pSubModel->vertindex);
	vec3_t *pstudionorms = (vec3_t *)((byte *)m_pStudioHeader + m_pSubModel->normindex);

	int skinnum = m_pCurrentEntity->curstate.skin; // for short..

	short *pskinref = (short *)((byte *)m_pTextureHeader + m_pTextureHeader->skinindex);
	if (skinnum != 0 && skinnum < m_pTextureHeader->numskinfamilies)
		pskinref += (skinnum * m_pTextureHeader->numskinref);

	if (m_pCurrentEntity != gEngfuncs.GetViewModel())
		gl.glCullFace(GL_FRONT);

	int cubemap = -1;
	if (m_pCurrentEntity && cv_cubemap->value > 0)
		cubemap = GetCubemapIdByPoint(m_pCurrentEntity->origin);

	if (cv_cubemap->value <= 0)
		cubemap = -1;

	if (studioShader)
		studioShader->bind();

	SetupShaderLighting();
	gl.glUniformMatrix3x4fv(studioShader->transform, 128, false, (const GLfloat*)&m_pbonetransform[0][0][0]);
	
	float camera[4];
	camera[0] = render_origin[0];
	camera[1] = render_origin[1];
	camera[2] = render_origin[2];
	camera[3] = 0.0;
	gl.glUniform4fvARB(studioShader->camuniform, 1, camera);

	vec3_t center = (m_pCurrentEntity->curstate.mins + m_pCurrentEntity->curstate.maxs) / 2;
	vec3_t point = m_pCurrentEntity->origin + center;

	lighting_ext lightinfo;
	EXT_LightPointStudio(point, &lightinfo, (m_pCurrentEntity->curstate.effects & EF_INVLIGHT));

	Vector light = lightinfo.ambientlight;
	Vector lightdir = lightinfo.lightdir;
	Vector addlight = lightinfo.addlight;

	for (int i = 0; i < 3; i++)
	{
		addlight[i] = ApplyGamma(addlight[i])*0.75;
		light[i] = ApplyGamma(light[i]) *0.75;
	}

	gl.glUniform3fARB(studioShader->colors, light.x, light.y, light.z);
	gl.glUniform3fARB(studioShader->lightdirection, lightdir.x, lightdir.y, lightdir.z);
	gl.glUniform3fARB(studioShader->addlight, addlight.x, addlight.y, addlight.z);
	

	for (int j = 0; j < m_pSubModel->nummesh; j++)
	{
		short		*ptricmds;
		pmesh = (mstudiomesh_t *)((byte *)m_pStudioHeader + m_pSubModel->meshindex) + j;
		ptricmds = (short *)((byte *)m_pStudioHeader + pmesh->triindex);
		mstudiotexture_t *ptex = &ptexture[pskinref[pmesh->skinref]];

		studiovbo_t *modelData = getVbo(m_pCurrentEntity, pmesh, ptricmds, ptex, pstudioverts, pvertbone, pstudionorms, pnormbone);
		if (!modelData) continue;

		char tname[64];
		char modelpath[64];
		pathtofilename(m_pCurrentEntity->model->name, modelpath);
		
		ExtStudioTextureData *extData = modelData->extTextData;

		if (gEngfuncs.pfnGetCvarFloat("ak_test") >= 1)
		{
			gEngfuncs.Cvar_SetValue("ak_test", 0);
		//	if (!needsChange)
			{
				needsChange = true;
				iSkinID++;
				if (iSkinID >= 3)
					iSkinID = 0;
			}
		}

		if (!modelData->extTextData)//if not, generate it
		{
			if (!strcmp(modelpath, "v_ak47"))//only for AK
			{
				if (needsChange)
				{
					if (strstr(ptex->name, "hands") != NULL)
					{
						gEngfuncs.Con_Printf("textures replaced, shutting down\n");
						needsChange = false;
					}

					if (iSkinID == 0)//skin ID 0 restores original textures
						sprintf(tname, "%s/%s", modelpath, ptex->name);
					else
						sprintf(tname, "%s0%i/%s", modelpath, iSkinID, ptex->name);
					
					strncpy(szToken, tname, sizeof(szToken));
					gEngfuncs.Con_Printf("szToken %s   \n", szToken);
				}
				else
					sprintf(tname, "%s/%s", modelpath, ptex->name);
			}
			else
				sprintf(tname, "%s/%s", modelpath, ptex->name);



			extData = AddExtStudioData(tname, ptex->index, ptex->name);
			modelData->extTextData = extData;
		}
		else
		{
			if (needsChange)
			{
			/*	if (strstr(szToken, "v_ak4702") != NULL)
				{
				//	gEngfuncs.Con_Printf("Found skin - shutting down\n");
				//	needsChange = false;
				}
				else*/ if (!strcmp(modelpath, "v_ak47"))
				{
					gEngfuncs.Con_Printf("deleting textures...\n");

					extData->gl_original = 0;
					extData->gl_textureindex = 0;
					extData->gl_glossmap_id = 0;


					extData = NULL;
					modelData->extTextData = NULL;
					continue;
				}
			}

		}
	//	gEngfuncs.Con_Printf("needs change %i, iSkinID %i\n", needsChange, iSkinID);

		/*
				if (!modelData->extTextData)//if not, generate it
		{
			if (!strcmp(modelpath, "v_ak47"))
			{
				if (gEngfuncs.pfnGetCvarFloat("ak_test") >= 1)
				{
					if (strstr(ptex->name, "hands") != NULL)
					{
						gEngfuncs.Con_Printf("textures replaced, shutting down\n");
						gEngfuncs.Cvar_SetValue("ak_test", 0);
					}

					sprintf(tname, "%s02/%s", modelpath, ptex->name);
					
					
					strncpy(szToken, tname, sizeof(szToken));
					gEngfuncs.Con_Printf("szToken %s   \n", szToken);
					//needsChange = false;
				}
				else
					sprintf(tname, "%s/%s", modelpath, ptex->name);
			}
			else
				sprintf(tname, "%s/%s", modelpath, ptex->name);



			extData = AddExtStudioData(tname, ptex->index, ptex->name);
			modelData->extTextData = extData;
		}
		else
		{
			if (gEngfuncs.pfnGetCvarFloat("ak_test") >= 1)
			{
			//	if (!needsChange)
				//	needsChange = true;

			//	gEngfuncs.Con_Printf(" tname %s\n", tname);
			//	gEngfuncs.Con_Printf("szToken %s   \n", szToken);
				if (strstr(szToken, "v_ak4702") != NULL)
				{
					gEngfuncs.Con_Printf("Found skin - shutting down\n");
					gEngfuncs.Cvar_SetValue("ak_test", 0);

				}
				else if (!strcmp(modelpath, "v_ak47"))
				{
					gEngfuncs.Con_Printf("deleting textures...\n");

					extData->gl_original = 0;
					extData->gl_textureindex = 0;
					extData->gl_glossmap_id = 0;


					extData = NULL;
					modelData->extTextData = NULL;
					continue;
				}
			}

		}
		*/
		if (!extData) continue;
		
		int glossid = 0;
		int texid = ptex->index;
		
		if (extData->gl_textureindex > 0 && extData->gl_textureindex >= (1 << 25))
			texid = extData->gl_textureindex;

		glossid = extData->gl_glossmap_id;

		int cubemapping = (glossid >= (1 << 25) && cubemap >= 0) ? 1 : 0;
		if (cubemapping > 0 )
		{
			gl.SaveStates();
			gl.glActiveTextureARB(GL_TEXTURE0_ARB);
			gl.glEnable(GL_TEXTURE_2D);
			gl.glUniform1iARB(studioShader->textureUniform, 0);
			gl.glBindTexture(GL_TEXTURE_2D, texid);

			gl.glActiveTextureARB(GL_TEXTURE1_ARB);
			gl.glEnable(GL_TEXTURE_2D);
			gl.glUniform1iARB(studioShader->maskUniform, 1);

			gl.glBindTexture(GL_TEXTURE_2D, glossid);

			gl.glActiveTextureARB(GL_TEXTURE2_ARB);
			gl.glUniform1iARB(studioShader->cubemapUniform, 2);
			SetupCubemap(cubemap);
		}
		else
		{
			gl.glActiveTextureARB(GL_TEXTURE0_ARB);
			gl.glDisable(GL_TEXTURE_CUBE_MAP_ARB);
			gl.glEnable(GL_TEXTURE_2D); 
			DisableCubemap(1);
			gl.glUniform1iARB(studioShader->textureUniform, 0);
			gl.glBindTexture(GL_TEXTURE_2D, texid);
		}
		gl.glUniform1fARB(studioShader->enablecubes, (float)cubemapping);

		gl.glEnable(GL_BLEND);
		gl.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		if (ptex->flags & STUDIO_NF_ADDITIVE && !useblending)	// buz
		{
			gl.glEnable(GL_BLEND);
			gl.glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		}

		if (ptex->flags & STUDIO_NF_ALPHATEST && !useblending)	// buz
		{
			gl.glEnable(GL_ALPHA_TEST);
			gl.glAlphaFunc(GL_GREATER, 0.5);
		}
		
		gl.glEnableClientState(GL_ARRAY_BUFFER_ARB);
		gl.glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, modelData->iboId);
		gl.glBindBufferARB(GL_ARRAY_BUFFER_ARB, modelData->vboId);

		gl.glEnableClientState(GL_VERTEX_ARRAY);
		gl.glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		gl.glEnableClientState(GL_NORMAL_ARRAY);

		gl.glEnableVertexAttribArrayARB(studioShader->boneid);
		gl.glVertexAttribPointerARB(studioShader->boneid, 1, GL_FLOAT, GL_FALSE, sizeof(studioVertex), BONEIDOFFSET);
		gl.glEnableVertexAttribArrayARB(studioShader->tangent);
		gl.glVertexAttribPointerARB(studioShader->tangent, 4, GL_FLOAT, GL_FALSE, sizeof(studioVertex), TANGENTOFFSET);
		gl.glEnableVertexAttribArrayARB(studioShader->binormal);
		gl.glVertexAttribPointerARB(studioShader->binormal, 3, GL_FLOAT, GL_FALSE, sizeof(studioVertex), BINORMALOFFSET);

		gl.glVertexPointer(3, GL_FLOAT, sizeof(studioVertex), 0);
		gl.glTexCoordPointer(2, GL_FLOAT, sizeof(studioVertex), TEXCOORDOFFSET);
		gl.glNormalPointer(GL_FLOAT, sizeof(studioVertex), NORMALOFFSET);

		gl.glDrawElements(GL_TRIANGLES, modelData->indexCount, GL_UNSIGNED_INT, NULL);
		
		gl.glDisableClientState(GL_VERTEX_ARRAY);
		gl.glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		gl.glDisableClientState(GL_NORMAL_ARRAY);

		gl.glDisableClientState(GL_ARRAY_BUFFER_ARB);
		gl.glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
		gl.glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);

		gl.glDisableVertexAttribArrayARB(studioShader->boneid);
		gl.glDisableVertexAttribArrayARB(studioShader->tangent);
		gl.glDisableVertexAttribArrayARB(studioShader->binormal);

		if ( cubemapping > 0 )
		{
			gl.glActiveTextureARB(GL_TEXTURE0_ARB);
			gl.glEnable(GL_TEXTURE_2D);
			
			gl.glActiveTextureARB(GL_TEXTURE1_ARB);
			gl.glDisable(GL_TEXTURE_2D);

			gl.glActiveTextureARB(GL_TEXTURE2_ARB);
			DisableCubemap(1); 

			gl.RestoreStates();
			gl.glActiveTextureARB(GL_TEXTURE0_ARB);
			gl.glDisable(GL_TEXTURE_CUBE_MAP_ARB);
			gl.glEnable(GL_TEXTURE_2D);

		}

		if (ptex->flags & STUDIO_NF_ADDITIVE && !useblending)	// buz
		{
			gl.glDisable(GL_BLEND);
		}

		if (ptex->flags & STUDIO_NF_ALPHATEST && !useblending)	// buz
		{
			gl.glDisable(GL_ALPHA_TEST);
			gl.glAlphaFunc(GL_GREATER, 0); // if leave 0.5, then grass sprites looks strange, i dunno why
		}

	}

	if (studioShader)
		studioShader->unbind();
}

void CStudioModelRenderer::GL_DrawPointsProp()
{
	if (!m_pCurrentEntity)
		return;

	byte *pvertbone = ((byte *)m_pStudioHeader + m_pSubModel->vertinfoindex);
	byte *pnormbone = ((byte *)m_pStudioHeader + m_pSubModel->norminfoindex);

	if (!m_pTextureHeader)
		return;

	if (m_pCurrentEntity->curstate.renderfx == 60) // light tester entity
		return;

	if (!m_pCurrentEntity->model)
		return;

	mstudiotexture_t *ptexture = (mstudiotexture_t *)((byte *)m_pTextureHeader + m_pTextureHeader->textureindex);
	mstudiomesh_t *pmesh = (mstudiomesh_t *)((byte *)m_pStudioHeader + m_pSubModel->meshindex);

	vec3_t *pstudioverts = (vec3_t *)((byte *)m_pStudioHeader + m_pSubModel->vertindex);
	vec3_t *pstudionorms = (vec3_t *)((byte *)m_pStudioHeader + m_pSubModel->normindex);

	int skinnum = m_pCurrentEntity->curstate.skin; // for short..

	short *pskinref = (short *)((byte *)m_pTextureHeader + m_pTextureHeader->skinindex);
	if (skinnum != 0 && skinnum < m_pTextureHeader->numskinfamilies)
		pskinref += (skinnum * m_pTextureHeader->numskinref);

	if (m_pCurrentEntity != gEngfuncs.GetViewModel())
		gl.glCullFace(GL_FRONT);

	int cubemap = -1;
	if (m_pCurrentEntity && cv_cubemap->value > 0)
		cubemap = GetCubemapIdByPoint(m_pCurrentEntity->origin);

	if (cv_cubemap->value <= 0)
		cubemap = -1;

	if (studioShaderProp)
		studioShaderProp->bind();

	SetupShaderLightingProp();
	
	float camera[4];
	camera[0] = render_origin[0];
	camera[1] = render_origin[1];
	camera[2] = render_origin[2];
	camera[3] = 0.0;
	gl.glUniform4fvARB(studioShaderProp->camuniform, 1, camera);
	
	for (int j = 0; j < m_pSubModel->nummesh; j++)
	{
		short		*ptricmds;
		pmesh = (mstudiomesh_t *)((byte *)m_pStudioHeader + m_pSubModel->meshindex) + j;
		ptricmds = (short *)((byte *)m_pStudioHeader + pmesh->triindex);
		mstudiotexture_t *ptex = &ptexture[pskinref[pmesh->skinref]];

		studiovbo_t *modelData = findVboProp(pmesh,m_pCurrentEntity);
		if (!modelData){
			for (int i = 0; i < m_pSubModel->numverts; i++)
			{
				VectorTransform(pstudioverts[i], (*m_pbonetransform)[pvertbone[i]], g_pxformverts[i]);
			}
			for (int i = 0; i < m_pSubModel->numnorms; i++)
			{
				VectorRotate(pstudionorms[i], (*m_pbonetransform)[pvertbone[i]], g_pnvalues[i]);
			}
		//	gEngfuncs.Con_Printf("Generate VBO for prop, transforms.\n");
			modelData = getVboProp(m_pCurrentEntity, pmesh, ptricmds, ptex, g_pxformverts, pvertbone, g_pnvalues, pnormbone);
		}

		if (!modelData) continue;

		ExtStudioTextureData *extData = modelData->extTextData;
		if (!modelData->extTextData)
		{
			char tname[64];
			char modelpath[64];
			pathtofilename(m_pCurrentEntity->model->name, modelpath);
			sprintf(tname, "%s/%s", modelpath, ptex->name);
			extData = AddExtStudioData(tname, ptex->index, ptex->name);
			modelData->extTextData = extData;
		//	gEngfuncs.Con_Printf("generating extra data %s/%s\n", modelpath, ptex->name);
		}
		if (!extData) continue;

		int glossid = 0;
		int texid = ptex->index;

		if (extData->gl_textureindex > 0 && extData->gl_textureindex >= (1 << 25))
			texid = extData->gl_textureindex;

		glossid = extData->gl_glossmap_id;

		int cubemapping = (glossid >= (1 << 25) && cubemap >= 0) ? 1 : 0;
		if (cubemapping > 0)
		{
			gl.SaveStates();
			gl.glActiveTextureARB(GL_TEXTURE0_ARB);
			gl.glEnable(GL_TEXTURE_2D);
			gl.glUniform1iARB(studioShaderProp->textureUniform, 0);
			gl.glBindTexture(GL_TEXTURE_2D, texid);

			gl.glActiveTextureARB(GL_TEXTURE1_ARB);
			gl.glEnable(GL_TEXTURE_2D);
			gl.glUniform1iARB(studioShaderProp->maskUniform, 1);

			gl.glBindTexture(GL_TEXTURE_2D, glossid);

			gl.glActiveTextureARB(GL_TEXTURE2_ARB);
			gl.glUniform1iARB(studioShaderProp->cubemapUniform, 2);
			SetupCubemap(cubemap);
		}
		else
		{
			gl.glActiveTextureARB(GL_TEXTURE0_ARB);
			gl.glDisable(GL_TEXTURE_CUBE_MAP_ARB);
			gl.glEnable(GL_TEXTURE_2D);
			DisableCubemap(1);
			gl.glUniform1iARB(studioShaderProp->textureUniform, 0);
			gl.glBindTexture(GL_TEXTURE_2D, texid);
		}
		gl.glUniform1fARB(studioShaderProp->enablecubes, (float)cubemapping);

		gl.glEnable(GL_BLEND);
		gl.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		if (ptex->flags & STUDIO_NF_ADDITIVE && !useblending)	// buz
		{
			gl.glEnable(GL_BLEND);
			gl.glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		}

		if (ptex->flags & STUDIO_NF_ALPHATEST && !useblending)	// buz
		{
			gl.glEnable(GL_ALPHA_TEST);
			gl.glAlphaFunc(GL_GREATER, 0.5);
		}

		gl.glEnableClientState(GL_ARRAY_BUFFER_ARB);
		gl.glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, modelData->iboId);
		gl.glBindBufferARB(GL_ARRAY_BUFFER_ARB, modelData->vboId);

		gl.glEnableClientState(GL_VERTEX_ARRAY);
		gl.glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		gl.glEnableClientState(GL_NORMAL_ARRAY);

		gl.glVertexPointer(3, GL_FLOAT, sizeof(studioVertexProp), 0);
		gl.glTexCoordPointer(2, GL_FLOAT, sizeof(studioVertexProp), TEXCOORDOFFSETPROP);
		gl.glNormalPointer(GL_FLOAT, sizeof(studioVertexProp), NORMALOFFSETPROP);

		gl.glEnableVertexAttribArrayARB(studioShaderProp->colorattrib);
		gl.glVertexAttribPointerARB(studioShaderProp->colorattrib, 3, GL_FLOAT, GL_FALSE, sizeof(studioVertexProp), LIGHTINGOFFSETPROP);

		gl.glDrawElements(GL_TRIANGLES, modelData->indexCount, GL_UNSIGNED_INT, NULL);

		gl.glDisableClientState(GL_VERTEX_ARRAY);
		gl.glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		gl.glDisableClientState(GL_NORMAL_ARRAY);
		gl.glDisableVertexAttribArrayARB(studioShaderProp->colorattrib);

		gl.glDisableClientState(GL_ARRAY_BUFFER_ARB);
		gl.glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
		gl.glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);

		if (cubemapping > 0)
		{
			gl.glActiveTextureARB(GL_TEXTURE0_ARB);
			gl.glEnable(GL_TEXTURE_2D);

			gl.glActiveTextureARB(GL_TEXTURE1_ARB);
			gl.glDisable(GL_TEXTURE_2D);

			gl.glActiveTextureARB(GL_TEXTURE2_ARB);
			DisableCubemap(1);

			gl.RestoreStates();
			gl.glActiveTextureARB(GL_TEXTURE0_ARB);
			gl.glDisable(GL_TEXTURE_CUBE_MAP_ARB);
			gl.glEnable(GL_TEXTURE_2D);

		}

		if (ptex->flags & STUDIO_NF_ADDITIVE && !useblending)	// buz
		{
			gl.glDisable(GL_BLEND);
		}

		if (ptex->flags & STUDIO_NF_ALPHATEST && !useblending)	// buz
		{
			gl.glDisable(GL_ALPHA_TEST);
			gl.glAlphaFunc(GL_GREATER, 0); // if leave 0.5, then grass sprites looks strange, i dunno why
		}

	}

	if (studioShaderProp)
		studioShaderProp->unbind();
}

void CStudioModelRenderer::GL_DrawPointsShadow()
{
	if (!m_pCurrentEntity)
		return;

	byte *pvertbone = ((byte *)m_pStudioHeader + m_pSubModel->vertinfoindex);
	byte *pnormbone = ((byte *)m_pStudioHeader + m_pSubModel->norminfoindex);

	if (!m_pTextureHeader)
		return;

	if (m_pCurrentEntity->curstate.renderfx == 60) // light tester entity
		return;

	if (!m_pCurrentEntity->model)
		return;

	mstudiotexture_t *ptexture = (mstudiotexture_t *)((byte *)m_pTextureHeader + m_pTextureHeader->textureindex);
	mstudiomesh_t *pmesh = (mstudiomesh_t *)((byte *)m_pStudioHeader + m_pSubModel->meshindex);

	vec3_t *pstudioverts = (vec3_t *)((byte *)m_pStudioHeader + m_pSubModel->vertindex);
	vec3_t *pstudionorms = (vec3_t *)((byte *)m_pStudioHeader + m_pSubModel->normindex);

	int skinnum = m_pCurrentEntity->curstate.skin; // for short..

	short *pskinref = (short *)((byte *)m_pTextureHeader + m_pTextureHeader->skinindex);
	if (skinnum != 0 && skinnum < m_pTextureHeader->numskinfamilies)
		pskinref += (skinnum * m_pTextureHeader->numskinref);

	if (m_pCurrentEntity != gEngfuncs.GetViewModel())
		gl.glCullFace(GL_FRONT);

	if (studioShaderNocube)
		studioShaderNocube->bind();

	int alphafunc;
	float alphaval;
	int alphatestenabled;

	gl.glGetIntegerv(GL_ALPHA_TEST_FUNC, &alphafunc);
	gl.glGetFloatv(GL_ALPHA_TEST_REF, &alphaval);
	if (gl.glIsEnabled(GL_ALPHA_TEST)) alphatestenabled = TRUE;
	else alphatestenabled = FALSE;

	gl.glUniformMatrix3x4fv(studioShaderNocube->transform, 128, false, (const GLfloat*)&m_pbonetransform[0][0][0]);
	for (int j = 0; j < m_pSubModel->nummesh; j++)
	{
		short		*ptricmds;
		pmesh = (mstudiomesh_t *)((byte *)m_pStudioHeader + m_pSubModel->meshindex) + j;
		ptricmds = (short *)((byte *)m_pStudioHeader + pmesh->triindex);
		mstudiotexture_t *ptex = &ptexture[pskinref[pmesh->skinref]];

		studiovbo_t *modelData = findVbo(pmesh);
		if (!modelData)
			continue;

		ExtStudioTextureData *extData = modelData->extTextData;
		if (!modelData->extTextData)
		{
			char tname[64];
			char modelpath[64];
			pathtofilename(m_pCurrentEntity->model->name, modelpath);
			sprintf(tname, "%s/%s", modelpath, ptex->name);
			extData = AddExtStudioData(tname, ptex->index, ptex->name);
			modelData->extTextData = extData;
		}
		if (!extData) continue;

		int texid = ptex->index;
		if (extData->gl_textureindex > 0)
			texid = extData->gl_textureindex;
		bool transparent = (ptex->flags & STUDIO_NF_ALPHATEST);
		bool additive = (ptex->flags & STUDIO_NF_ADDITIVE);
		if (additive) continue;

		if (extData->gl_textureindex > 0)
			texid = extData->gl_textureindex;
		
		if (transparent){
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			gl.glEnable(GL_ALPHA_TEST);
			gl.glAlphaFunc(GL_GREATER, 0.5);
		}
		else{
			glDisable(GL_BLEND);
		}

		gl.glActiveTextureARB(GL_TEXTURE0_ARB);
		gl.glDisable(GL_TEXTURE_CUBE_MAP_ARB);
		gl.glEnable(GL_TEXTURE_2D);
		gl.glBindTexture(GL_TEXTURE_2D, texid);

		gl.glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, modelData->iboId);
		gl.glBindBufferARB(GL_ARRAY_BUFFER_ARB, modelData->vboId);
		
		gl.glEnableClientState(GL_VERTEX_ARRAY);
		gl.glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		
		gl.glEnableVertexAttribArrayARB(studioShaderNocube->boneid);
		gl.glVertexAttribPointerARB(studioShaderNocube->boneid, 1, GL_FLOAT, GL_FALSE, sizeof(studioVertex), BONEIDOFFSET);
		
		gl.glVertexPointer(3, GL_FLOAT, sizeof(studioVertex), 0);
		gl.glTexCoordPointer(2, GL_FLOAT, sizeof(studioVertex), TEXCOORDOFFSET);
		
		gl.glDrawElements(GL_TRIANGLES, modelData->indexCount, GL_UNSIGNED_INT, NULL);
		
		gl.glDisableVertexAttribArrayARB(studioShaderNocube->boneid);
		
		gl.glDisableClientState(GL_VERTEX_ARRAY);
		//gl.glDisableClientState(GL_TEXTURE_COORD_ARRAY);

		if (alphatestenabled)
			gl.glEnable(GL_ALPHA_TEST);
		else
			gl.glDisable(GL_ALPHA_TEST);

		gl.glAlphaFunc(alphafunc, alphaval);
		
		if (useblending){
			gl.glEnable(GL_BLEND);
		}
		else{
			gl.glDisable(GL_BLEND);
		}
	}
	
	gl.glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
	gl.glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

	if (studioShaderNocube)
		studioShaderNocube->unbind();
}

void CStudioModelRenderer::GL_DrawPointsShadowProp(){
	if (!m_pCurrentEntity)
		return;

	byte *pvertbone = ((byte *)m_pStudioHeader + m_pSubModel->vertinfoindex);
	byte *pnormbone = ((byte *)m_pStudioHeader + m_pSubModel->norminfoindex);

	if (!m_pTextureHeader)
		return;

	if (m_pCurrentEntity->curstate.renderfx == 60) // light tester entity
		return;

	if (!m_pCurrentEntity->model)
		return;

	mstudiotexture_t *ptexture = (mstudiotexture_t *)((byte *)m_pTextureHeader + m_pTextureHeader->textureindex);
	mstudiomesh_t *pmesh = (mstudiomesh_t *)((byte *)m_pStudioHeader + m_pSubModel->meshindex);

	vec3_t *pstudioverts = (vec3_t *)((byte *)m_pStudioHeader + m_pSubModel->vertindex);
	vec3_t *pstudionorms = (vec3_t *)((byte *)m_pStudioHeader + m_pSubModel->normindex);

	int skinnum = m_pCurrentEntity->curstate.skin; // for short..

	short *pskinref = (short *)((byte *)m_pTextureHeader + m_pTextureHeader->skinindex);
	if (skinnum != 0 && skinnum < m_pTextureHeader->numskinfamilies)
		pskinref += (skinnum * m_pTextureHeader->numskinref);

	if (m_pCurrentEntity != gEngfuncs.GetViewModel())
		gl.glCullFace(GL_FRONT);

	if (studioShaderPropNocube)
		studioShaderPropNocube->bind();

	int alphafunc;
	float alphaval;
	int alphatestenabled;

	gl.glGetIntegerv(GL_ALPHA_TEST_FUNC, &alphafunc);
	gl.glGetFloatv(GL_ALPHA_TEST_REF, &alphaval);
	if (gl.glIsEnabled(GL_ALPHA_TEST)) alphatestenabled = TRUE;
	else alphatestenabled = FALSE;

	for (int j = 0; j < m_pSubModel->nummesh; j++)
	{
		short		*ptricmds;
		pmesh = (mstudiomesh_t *)((byte *)m_pStudioHeader + m_pSubModel->meshindex) + j;
		ptricmds = (short *)((byte *)m_pStudioHeader + pmesh->triindex);
		mstudiotexture_t *ptex = &ptexture[pskinref[pmesh->skinref]];

		studiovbo_t *modelData = findVboProp(pmesh,m_pCurrentEntity);// getVboProp(m_pCurrentEntity, pmesh, ptricmds, ptex, g_pxformverts, pvertbone, g_pnvalues, pnormbone);
		if (!modelData) continue;

		ExtStudioTextureData *extData = modelData->extTextData;
		if (!modelData->extTextData)
		{
			char tname[64];
			char modelpath[64];
			pathtofilename(m_pCurrentEntity->model->name, modelpath);
			sprintf(tname, "%s/%s", modelpath, ptex->name);
			extData = AddExtStudioData(tname, ptex->index, ptex->name);
			modelData->extTextData = extData;
			//	gEngfuncs.Con_Printf("generating extra data\n");
		}
		if (!extData) continue;

		int texid = ptex->index;
		if (extData->gl_textureindex > 0)
			texid = extData->gl_textureindex;
		bool transparent = (ptex->flags & STUDIO_NF_ALPHATEST);
		bool additive = (ptex->flags & STUDIO_NF_ADDITIVE);
		if (additive) continue;

		if (extData->gl_textureindex > 0)
			texid = extData->gl_textureindex;

		if (transparent){
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			gl.glEnable(GL_ALPHA_TEST);
			gl.glAlphaFunc(GL_GREATER, 0.5);
		}
		else{
			glDisable(GL_BLEND);
		}

		gl.glActiveTextureARB(GL_TEXTURE0_ARB);
		gl.glDisable(GL_TEXTURE_CUBE_MAP_ARB);
		gl.glEnable(GL_TEXTURE_2D);
		DisableCubemap(1);
		gl.glBindTexture(GL_TEXTURE_2D, texid);

		gl.glEnableClientState(GL_ARRAY_BUFFER_ARB);

		gl.glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, modelData->iboId);
		gl.glBindBufferARB(GL_ARRAY_BUFFER_ARB, modelData->vboId);

		gl.glEnableClientState(GL_VERTEX_ARRAY);
		if (transparent)
			gl.glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		gl.glVertexPointer(3, GL_FLOAT, sizeof(studioVertexProp), 0);

		if (transparent)
			gl.glTexCoordPointer(2, GL_FLOAT, sizeof(studioVertexProp), TEXCOORDOFFSETPROP);

		gl.glDrawElements(GL_TRIANGLES, modelData->indexCount, GL_UNSIGNED_INT, NULL);

		gl.glDisableClientState(GL_VERTEX_ARRAY);
		//if (transparent)
		//	gl.glDisableClientState(GL_TEXTURE_COORD_ARRAY);

		gl.glDisableClientState(GL_ARRAY_BUFFER_ARB);
		gl.glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
		gl.glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);

		if (alphatestenabled)
			gl.glEnable(GL_ALPHA_TEST);
		else
			gl.glDisable(GL_ALPHA_TEST);

		gl.glAlphaFunc(alphafunc, alphaval);

		if (useblending){
			gl.glEnable(GL_BLEND);
		}
		else{
			gl.glDisable(GL_BLEND);
		}
	}

	if (studioShaderPropNocube)
		studioShaderPropNocube->unbind();
}

#if 0
void CStudioModelRenderer::GL_DrawPointsShadowProp()
{
	if (!m_pCurrentEntity)
		return;

	byte *pvertbone = ((byte *)m_pStudioHeader + m_pSubModel->vertinfoindex);
	byte *pnormbone = ((byte *)m_pStudioHeader + m_pSubModel->norminfoindex);

	if (!m_pTextureHeader)
		return;

	if (m_pCurrentEntity->curstate.renderfx == 60) // light tester entity
		return;

	if (!m_pCurrentEntity->model)
		return;

	mstudiotexture_t *ptexture = (mstudiotexture_t *)((byte *)m_pTextureHeader + m_pTextureHeader->textureindex);
	mstudiomesh_t *pmesh = (mstudiomesh_t *)((byte *)m_pStudioHeader + m_pSubModel->meshindex);

	vec3_t *pstudioverts = (vec3_t *)((byte *)m_pStudioHeader + m_pSubModel->vertindex);
	vec3_t *pstudionorms = (vec3_t *)((byte *)m_pStudioHeader + m_pSubModel->normindex);

	int skinnum = m_pCurrentEntity->curstate.skin; // for short..

	short *pskinref = (short *)((byte *)m_pTextureHeader + m_pTextureHeader->skinindex);
	if (skinnum != 0 && skinnum < m_pTextureHeader->numskinfamilies)
		pskinref += (skinnum * m_pTextureHeader->numskinref);

	gl.glCullFace(GL_NONE);

	if (studioShaderPropNocube)
		studioShaderPropNocube->bind();

	for (int j = 0; j < m_pSubModel->nummesh; j++)
	{
		short		*ptricmds;
		pmesh = (mstudiomesh_t *)((byte *)m_pStudioHeader + m_pSubModel->meshindex) + j;
		ptricmds = (short *)((byte *)m_pStudioHeader + pmesh->triindex);
		mstudiotexture_t *ptex = &ptexture[pskinref[pmesh->skinref]];

		studiovbo_t *modelData = getVboProp(m_pCurrentEntity, pmesh, ptricmds, ptex, pstudioverts, pvertbone, pstudionorms);
		if (!modelData)
			continue;

		ExtStudioTextureData *extData = modelData->extTextData;
		if (!modelData->extTextData)
		{
			char tname[64];
			char modelpath[64];
			pathtofilename(m_pCurrentEntity->model->name, modelpath);
			sprintf(tname, "%s/%s", modelpath, ptex->name);
			extData = AddExtStudioData(tname, ptex->index, ptex->name);
			modelData->extTextData = extData;
		}
		if (!extData) continue;

		int texid = ptex->index;
		if (extData->gl_textureindex > 0)
			texid = extData->gl_textureindex;
		bool transparent = (ptex->flags & STUDIO_NF_ALPHATEST);
		bool additive = (ptex->flags & STUDIO_NF_ADDITIVE);
		if (additive || transparent) continue;

		gl.glActiveTextureARB(GL_TEXTURE0_ARB);
		gl.glEnable(GL_TEXTURE_2D);
		gl.glBindTexture(GL_TEXTURE_2D, texid);

		if (transparent)
		{
			gl.glEnable(GL_BLEND);
			gl.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			gl.glEnable(GL_ALPHA_TEST);
			gl.glAlphaFunc(GL_GREATER, 0.5);
		}
		else
			gl.glDisable(GL_BLEND);

		gl.glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, modelData->iboId);
		gl.glBindBufferARB(GL_ARRAY_BUFFER_ARB, modelData->vboId);

		gl.glEnableClientState(GL_VERTEX_ARRAY);
		gl.glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		gl.glVertexPointer(3, GL_FLOAT, sizeof(studioVertexProp), 0);
		gl.glTexCoordPointer(2, GL_FLOAT, sizeof(studioVertexProp), TEXCOORDOFFSETPROP);

		gl.glDrawElements(GL_TRIANGLES, modelData->indexCount, GL_UNSIGNED_INT, NULL);

		gl.glDisableClientState(GL_VERTEX_ARRAY);
		gl.glDisableClientState(GL_TEXTURE_COORD_ARRAY);

		if (transparent)	// buz
		{
			gl.glDisable(GL_BLEND);
			gl.glDisable(GL_ALPHA_TEST);
			gl.glAlphaFunc(GL_GREATER, 0);
		}
	}

	gl.glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
	gl.glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

	if (studioShaderPropNocube)
		studioShaderPropNocube->unbind();
}
#endif

//=============================
// EXT_LightPoint
//
// Gets extended lighting info from bump lightmaps.
// Light values should be multiplied by 2, if you want exact color value
//=============================
//int recursioncheck;
int EXT_RecursiveLightPoint(mnode_t *node, const vec3_t &start, const vec3_t &end, lighting_ext *lightinfo);

void EXT_LightPoint(vec3_t pos, lighting_ext *lightinfo, int invlight)
{
	vec3_t		end;
	model_t* world = gEngfuncs.GetEntityByIndex(0)->model;
	memset(lightinfo, 0, sizeof(lighting_ext));

	if (!world->lightdata)
	{
		lightinfo->ambientlight[0] = 0.5;
		lightinfo->ambientlight[1] = 0.5;
		lightinfo->ambientlight[2] = 0.5;

		return;
	}

	end[0] = pos[0];
	end[1] = pos[1];

	if (invlight)
		end[2] = pos[2] + 2048;
	else
		end[2] = pos[2] - 2048;

	//	recursioncheck = 0;
	if (!EXT_RecursiveLightPoint(world->nodes, pos, end, lightinfo))
	{
		// Object is too high from ground (for example, helicopter).
		// Use sky color and direction
		lightinfo->ambientlight[0] = lightinfo->addlight[0] = 0.5 + ApplyGamma(sv_skycolor_r->value / 255) * CVAR_GET_FLOAT("prop_brightness");
		lightinfo->ambientlight[1] = lightinfo->addlight[1] = 0.5 + ApplyGamma(sv_skycolor_g->value / 255) * CVAR_GET_FLOAT("prop_brightness");
		lightinfo->ambientlight[2] = lightinfo->addlight[2] = 0.5 + ApplyGamma(sv_skycolor_b->value / 255) * CVAR_GET_FLOAT("prop_brightness");
		lightinfo->lightdir = Vector(0, 0, 1);
	}
	//	CONPRINT("calls: %d\n", recursioncheck);
}

void EXT_LightPointStudio(vec3_t pos, lighting_ext *lightinfo, int invlight)
{
	vec3_t		end;
	model_t* world = gEngfuncs.GetEntityByIndex(0)->model;
	memset(lightinfo, 0, sizeof(lighting_ext));

	if (!world->lightdata)
	{
		lightinfo->ambientlight[0] = 0.5;
		lightinfo->ambientlight[1] = 0.5;
		lightinfo->ambientlight[2] = 0.5;

		return;
	}

	end[0] = pos[0];
	end[1] = pos[1];

	if (invlight)
		end[2] = pos[2] + 2048;
	else
		end[2] = pos[2] - 2048;

/*	Vector forward;
	float frac = 1.0;
	for (int i = 0; i < 6; i++)
	{
		forward = pos;
		switch (i)
		{
		case 0://up
			forward.z += 2048.0f;
			break;
		case 1://down
			forward.z -= 2048.0f;
			break;
		case 2://left
			forward.x -= 2048.0f;
			break;
		case 3://right
			forward.y += 2048.0f;
			break;
		case 4://forward
			forward.x += 2048.0f;
			break;
		case 5://back
			forward.x -= 2048.0f;
			break;
		}

		pmtrace_t * trace = gEngfuncs.PM_TraceLine(pos, forward, PM_TRACELINE_PHYSENTSONLY, 1, -1);
		if (trace->fraction <= frac)
		{
			frac = trace->fraction;
			end = forward;
		}
	}*/

	//	recursioncheck = 0;
	if (!EXT_RecursiveLightPoint(world->nodes, pos, end, lightinfo))
	{
		// Object is too high from ground (for example, helicopter).
		// Use sky color and direction
		lightinfo->ambientlight[0] = lightinfo->addlight[0] = 0.5 + ApplyGamma(sv_skycolor_r->value / 255) * CVAR_GET_FLOAT("prop_brightness");
		lightinfo->ambientlight[1] = lightinfo->addlight[1] = 0.5 + ApplyGamma(sv_skycolor_g->value / 255) * CVAR_GET_FLOAT("prop_brightness");
		lightinfo->ambientlight[2] = lightinfo->addlight[2] = 0.5 + ApplyGamma(sv_skycolor_b->value / 255) * CVAR_GET_FLOAT("prop_brightness");
		lightinfo->lightdir = Vector(0, 0, 1);
	}
	//	CONPRINT("calls: %d\n", recursioncheck);
}

void EXT_LightPointStudioProp(vec3_t pos,vec3_t normal, lighting_ext *lightinfo, int invlight)
{
	vec3_t		end;
	model_t* world = gEngfuncs.GetEntityByIndex(0)->model;
	memset(lightinfo, 0, sizeof(lighting_ext));

	if (!world->lightdata)
	{
		lightinfo->ambientlight[0] = 0.5;
		lightinfo->ambientlight[1] = 0.5;
		lightinfo->ambientlight[2] = 0.5;
		return;
	}

	end[0] = pos[0];
	end[1] = pos[1];
	end[2] = pos[2];
	end = end + normal * 2048;

	pmtrace_t *trace = gEngfuncs.PM_TraceLine(pos, end, PM_TRACELINE_PHYSENTSONLY, 2, -1);
	if (trace)
	{
		int contents = gEngfuncs.PM_PointContents(trace->endpos, NULL);
		if (contents != CONTENTS_SKY && contents != CONTENTS_EMPTY)
		{
			end = trace->endpos; end.z -= 2048;
		}
	}

	//	recursioncheck = 0;
	if (!EXT_RecursiveLightPoint(world->nodes, pos, end, lightinfo))
	{
		// Object is too high from ground (for example, helicopter).
		// Use sky color and direction
	/*	lightinfo->ambientlight[0] = lightinfo->addlight[0] = 0.5;
		lightinfo->ambientlight[1] = lightinfo->addlight[1] = 0.5;
		lightinfo->ambientlight[2] = lightinfo->addlight[2] = 0.5;*/
		lightinfo->ambientlight[0] = lightinfo->addlight[0] = 0.5 + ApplyGamma(sv_skycolor_r->value / 255) * CVAR_GET_FLOAT("prop_brightness");
		lightinfo->ambientlight[1] = lightinfo->addlight[1] = 0.5 + ApplyGamma(sv_skycolor_g->value / 255) * CVAR_GET_FLOAT("prop_brightness");
		lightinfo->ambientlight[2] = lightinfo->addlight[2] = 0.5 + ApplyGamma(sv_skycolor_b->value / 255) * CVAR_GET_FLOAT("prop_brightness");
		lightinfo->lightdir = Vector(0, 0, 1);
	}
}

void PARSE_COLOR(vec3_t &out, color24 *lightmap)
{
	out[0] = (float)lightmap->r / 255.0f;
	out[1] = (float)lightmap->g / 255.0f;
	out[2] = (float)lightmap->b / 255.0f;
}


int EXT_RecursiveLightPoint(mnode_t *node, const vec3_t &start, const vec3_t &end, lighting_ext *light)
{
	float		front, back, frac;
	int			side;
	mplane_t	*plane;
	vec3_t		mid;
	msurface_t	*surf;
	int			s, t, ds, dt;
	int			i;
	mtexinfo_t	*tex;
	color24		*lightmap;

	//	recursioncheck++;

	if (node->contents < 0)
		return FALSE;		// didn't hit anything

	// calculate mid point

	// FIXME: optimize for axial
	plane = node->plane;
	front = DotProduct(start, plane->normal) - plane->dist;
	back = DotProduct(end, plane->normal) - plane->dist;
	side = front < 0;

	if ((back < 0) == side)
		return EXT_RecursiveLightPoint(node->children[side], start, end, light);

	frac = front / (front - back);
	mid[0] = start[0] + (end[0] - start[0])*frac;
	mid[1] = start[1] + (end[1] - start[1])*frac;
	mid[2] = start[2] + (end[2] - start[2])*frac;

	// go down front side	
	int r = EXT_RecursiveLightPoint(node->children[side], start, mid, light);
	if (r) return TRUE;		// hit something

	if ((back < 0) == side)
		return FALSE;		// didn't hit anuthing

	// check for impact on this node
	model_t* world = gEngfuncs.GetEntityByIndex(0)->model;

	surf = world->surfaces + node->firstsurface;
	for (i = 0; i<node->numsurfaces; i++, surf++)
	{
		if (surf->flags & (SURF_DRAWTILED | SURF_DRAWSKY))
			continue;	// no lightmaps

		tex = surf->texinfo;

		s = DotProduct(mid, tex->vecs[0]) + tex->vecs[0][3];
		t = DotProduct(mid, tex->vecs[1]) + tex->vecs[1][3];;

		if (s < surf->texturemins[0] ||
			t < surf->texturemins[1])
			continue;

		ds = s - surf->texturemins[0];
		dt = t - surf->texturemins[1];

		if (ds > surf->extents[0] || dt > surf->extents[1])
			continue;

		if (!surf->samples)
			continue;

		ds >>= 4;
		dt >>= 4;

		lightmap = surf->samples;
		if (lightmap)
		{
			vec3_t origcolor = Vector(0, 0, 0);
			vec3_t ambientcolor = Vector(0, 0, 0);
			vec3_t diffusecolor = Vector(0, 0, 0);
			vec3_t lightvec;
			float dot;
			int not_normal = 0;

			lightmap += dt * ((surf->extents[0] >> 4) + 1) + ds;

			for (int style = 0; style < MAXLIGHTMAPS && surf->styles[style] != 255; style++)
			{
				if (surf->styles[style] == 0)
				{
					PARSE_COLOR(origcolor, lightmap);
				}
				else if (surf->styles[style] == BUMP_LIGHTVECS_STYLE)
				{
					vec3_t tmp;
					PARSE_COLOR(tmp, lightmap);

					vec3_t vec_x;
					vec3_t vec_y;
					VectorCopy(tex->vecs[0], vec_x);
					VectorCopy(tex->vecs[1], vec_y);
					VectorNormalize(vec_x);
					VectorNormalize(vec_y);

					vec3_t pnormal;
					VectorCopy(surf->plane->normal, pnormal);
					if (surf->flags & SURF_PLANEBACK)
					{
						VectorInverse(pnormal);
					}

					dot = tmp[2] * 2 - 1;

					VectorClear(lightvec);
					VectorMA(lightvec, tmp[0] * 2 - 1, vec_x, lightvec);
					VectorMA(lightvec, tmp[1] * 2 - 1, vec_y, lightvec);
					VectorMA(lightvec, tmp[2] * 2 - 1, pnormal, lightvec);
				}
				else if (surf->styles[style] == BUMP_ADDLIGHT_STYLE)
				{
					PARSE_COLOR(diffusecolor, lightmap);
				}
				else if (surf->styles[style] == BUMP_BASELIGHT_STYLE)
				{
					PARSE_COLOR(ambientcolor, lightmap);
					not_normal = 1;
				}

				lightmap += ((surf->extents[0] >> 4) + 1) *
					((surf->extents[1] >> 4) + 1);
			}

			// apply gamma and write to ext_lightinfo struct
			//	if (diffusecolor[0] || diffusecolor[1] || diffusecolor[2] ||
			//		ambientcolor[0] || ambientcolor[1] || ambientcolor[2])
			if (not_normal)
			{
				vec3_t temp;
				vec3_t scale = Vector(0, 0, 0);
				VectorScale(diffusecolor, dot, temp);
				VectorAdd(temp, ambientcolor, temp);
				VectorScale(temp, 2, temp);

				if (temp[0]) scale[0] = ApplyGamma(temp[0]) / temp[0];
				if (temp[1]) scale[1] = ApplyGamma(temp[1]) / temp[1];
				if (temp[2]) scale[2] = ApplyGamma(temp[2]) / temp[2];

				//this would be what the sun is touching
				light->addlight[0] = diffusecolor[0] * scale[0] - 0.2;//2;
				light->addlight[1] = diffusecolor[1] * scale[1] - 0.2;
				light->addlight[2] = diffusecolor[2] * scale[2] - 0.2;

		//		gEngfuncs.Con_Printf("origcolor[0] %i\n",origcolor[0]);

				//this would be the color from the surface that lies in
				light->ambientlight[0] = ApplyGamma(origcolor[0] -0.1);
				light->ambientlight[1] = ApplyGamma(origcolor[1]  - 0.1);
				light->ambientlight[2] = ApplyGamma(origcolor[2]- 0.1);
			
	
			/*	light->ambientlight[0] =0.5+ ApplyGamma(sv_skycolor_r->value / 255) * CVAR_GET_FLOAT("prop_brightness");
				light->ambientlight[1] = 0.5 + ApplyGamma(sv_skycolor_g->value / 255) * CVAR_GET_FLOAT("prop_brightness");
				light->ambientlight[2] = 0.5 + ApplyGamma(sv_skycolor_b->value / 255) * CVAR_GET_FLOAT("prop_brightness");*/

				VectorCopy(lightvec, light->lightdir);
			}
			else
			{
				//this would be the highlights
			 	light->ambientlight[0] = ApplyGamma(origcolor[0]);
			 	light->ambientlight[1] = ApplyGamma(origcolor[1]);
				light->ambientlight[2] = ApplyGamma(origcolor[2]);
			}
		}
		return TRUE;
	}

	// go down back side
	return EXT_RecursiveLightPoint(node->children[!side], mid, end, light);
}