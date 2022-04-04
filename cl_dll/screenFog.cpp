/*

It's a modern-new fog for screenspace.
Written by HAWK (Alexander Bakanov)
01.18.2015
*/

#include "hud.h"
#include "cl_util.h"
#include "glmanager.h"

#include "fog.h"
#include "gl_texloader.h"
#include "gl_shaders.h"
#include "glStuff.h"

extern cvar_t *r_fogmode;
CFogEffect gFog;
void CFogEffect::InitFog()
{
	if (inited)
		return;

	inited = false;

	//Now init a textures:
	CreateEmptyTex(ScreenWidth, ScreenHeight, depthtex, GL_TEXTURE_2D, GL_DEPTH_COMPONENT, true);
	CreateEmptyTex(ScreenWidth, ScreenHeight, screentex, GL_TEXTURE_2D, GL_RGB, true);

	//And shaders:
	if (!LoadShaderFile("shaders/fog.arf", fp_fog))
	{
		gEngfuncs.Con_Printf("Unable to load fog shader\n");
		return;
	}
	else
		gEngfuncs.Con_Printf("Fog shader loaded successfully\n");

	if (!LoadShaderFile("shaders/foghi.arf", fp_foghi))
	{
		gEngfuncs.Con_Printf("Unable to load foghi shader\n");
		return;
	}
	else
		gEngfuncs.Con_Printf("FogHi shader loaded successfully\n");

	gEngfuncs.Con_Printf("Fog has been inited;\n");
	inited = true;
}

void CFogEffect::DrawEffect()
{
	if (r_fogmode->value <= 0)
		return;

	if (gHUD.fogValues[3] <= 0)
		return;
	if (gHUD.fogValues[4] <= 0)
		return;

	gl.glColor4f(gHUD.fogValues[0],
		gHUD.fogValues[1],gHUD.fogValues[2], 1);

	gl.glEnable(GL_BLEND);
	gl.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	gl.glBindTexture(GL_TEXTURE_2D, depthtex);
	gl.glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 0, 0, ScreenWidth, ScreenHeight, 0);

	glFogf(GL_FOG_START, gHUD.fogValues[3]);
	glFogf(GL_FOG_END, gHUD.fogValues[4]);

	if (r_fogmode->value > 1)
	{
		gl.glBindTexture(GL_TEXTURE_2D, screentex);
		gl.glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, ScreenWidth, ScreenHeight, 0);

		gl.SaveStates();
		gl.glEnable(GL_FRAGMENT_PROGRAM_ARB);
		gl.glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, fp_foghi);

		gl.glActiveTextureARB(GL_TEXTURE1_ARB);
		gl.glBindTexture(GL_TEXTURE_2D, screentex);
		gl.glActiveTextureARB(GL_TEXTURE0_ARB);
		gl.glBindTexture(GL_TEXTURE_2D, depthtex);

		gGlStuff.DrawQuadRAW(ScreenWidth, 0, 0, ScreenHeight);

		gl.glDisable(GL_FRAGMENT_PROGRAM_ARB);

		gl.RestoreStates();
	}
	else if (r_fogmode->value <= 1)
	{
		gl.glEnable(GL_FRAGMENT_PROGRAM_ARB);
		gl.glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, fp_fog);

		gGlStuff.DrawQuad(ScreenWidth, 0, depthtex, 0, ScreenHeight);

		gl.glDisable(GL_FRAGMENT_PROGRAM_ARB);
	}
}

void CFogEffect::ResetFog()
{
}