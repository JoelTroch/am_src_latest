/*

It's a modern-new blur with class system.
I newer use a classes, only procedual programming,
Let me test it :)
*/

#include "hud.h"
#include "cl_util.h"
#include "glmanager.h"

#include "blur.h"
#include "gl_texloader.h"
#include "gl_shaders.h"
#include "glStuff.h"

#define GLOW_RESOLUTION 256//128

CGlowEffect gGlow;
void CGlowEffect::InitGlow()
{
	if (inited)
		return;

	inited = false;

	//Now init a textures:
	CreateEmptyTex(ScreenWidth, ScreenHeight, screentex, GL_TEXTURE_2D, GL_RGB, true);
	CreateEmptyTex(GLOW_RESOLUTION, GLOW_RESOLUTION, glowtex, GL_TEXTURE_2D, GL_RGB, true);

	//And shaders:
	if (!LoadShaderFile("shaders/gaussianglow.asm", fp_glow))
	{
		gEngfuncs.Con_Printf("Unable to load glow shader\n");
		return;
	}
	else
		gEngfuncs.Con_Printf("Glow shader loaded successfully\n");

	gEngfuncs.Con_Printf("Glow has been inited;\n");
	inited = true;
}

extern cvar_t *r_bloom;
void CGlowEffect::DrawEffect()
{
	if (!r_bloom) return;
	if (!r_bloom->value) return;

	InterpolateIt();
	calpha = 0.25;
	cstrength = 1.5;//1..5

	if (this->calpha < 0.001 || this->cstrength < 0.0000001)
	{
		gl.glColor4f(1, 1, 1, 1);
		gl.glBindTexture(GL_TEXTURE_2D, screentex);
		gl.glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, ScreenWidth, ScreenHeight, 0);
		gGlStuff.DrawQuadS(0, 0, screentex, ScreenWidth, ScreenHeight, false);
		return;
	}

	gl.glColor4f(1, 1, 1, 1);

	gl.glEnable(GL_BLEND);
	gl.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	gl.glBindTexture(GL_TEXTURE_2D, screentex);
	gl.glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, ScreenWidth, ScreenHeight, 0);

	gl.glViewport(0, 0, GLOW_RESOLUTION, GLOW_RESOLUTION);
	gl.glBindTexture(GL_TEXTURE_2D, screentex);

	//draw our screen quad in small VP.
	gGlStuff.DrawQuadS(0, 0, screentex, ScreenWidth, ScreenHeight, false);
//	gGlStuff.DrawScreenOV();

	//and copy it into small texture.
	gl.glBindTexture(GL_TEXTURE_2D, glowtex);
	gl.glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, GLOW_RESOLUTION, GLOW_RESOLUTION, 0);

	gl.glColor4f(1, 1, 1, 1);
	gl.glDisable(GL_BLEND);

	gl.glEnable(GL_FRAGMENT_PROGRAM_ARB);
	gl.glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, fp_glow);
	glProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 0, 1 * cstrength, 0, 0.0, 0);//horizontal

	gGlStuff.DrawQuadS(0, 0, glowtex, ScreenWidth, ScreenHeight, false);
	gl.glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, GLOW_RESOLUTION, GLOW_RESOLUTION, 0);

	glProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 0, 0, 1 * cstrength, 0.0, 0);//vertical
	gGlStuff.DrawQuadS(0, 0, glowtex, ScreenWidth, ScreenHeight, false);
	gl.glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, GLOW_RESOLUTION, GLOW_RESOLUTION, 0);
	gl.glDisable(GL_FRAGMENT_PROGRAM_ARB);

	//restore view to original.
	gl.glColor4f(1, 1, 1, 1);
	gl.glViewport(0, 0, ScreenWidth, ScreenHeight);
	gGlStuff.DrawQuadS(0, 0, screentex, ScreenWidth, ScreenHeight, false);

	gl.glEnable(GL_BLEND);
	gl.glColor4f(calpha,calpha,calpha, 1);
	gGlStuff.DrawQuadS(0, 0, glowtex, ScreenWidth, ScreenHeight, true);
//	gGlStuff.DrawScreenOV();
	gl.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void CGlowEffect::SetNew(float _str, float _a, float _f, int _fmode)
{
	alpha = ialpha;
	strength = istrength;

	fademode = _fmode;
	fadetime = _f;
	ialpha = _a;
	istrength = _str;

	gEngfuncs.Con_Printf("Glow updated\n");
	lastchanged = gEngfuncs.GetClientTime();
}

void CGlowEffect::ResetGlow()
{
	SetNew(0, 0, 1, 0);
}

void CGlowEffect::InterpolateIt()
{
	if (fadetime <= 0)
		fadetime = 1.0;

	float frac = (gEngfuncs.GetClientTime() - lastchanged) / fadetime;

	if (frac < 0)
		return;
	if (frac > 1)
		return;

	calpha = LnrIntrpltn(alpha, ialpha, 1.0 - frac);
	cstrength = LnrIntrpltn(strength, istrength, 1.0 - frac);
}


