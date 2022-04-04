/*
	HAWK (C) 10.01.2015.
*/

#include "hud.h"
#include "cl_util.h"
#include "glmanager.h"
#include "../common/com_model.h"

#include "raindrops.h"
#include "gl_texloader.h"

#include "gl_shaders.h"
#include "glStuff.h"

#include "pm_defs.h"
#include "event_api.h"

#define RAINDROP_SIZE (float)XRES(32)

CRainDropsEffect gRaindrops;
void CRainDropsEffect::InitRain()
{
	if (inited)
		return;

	inited = false;
	//Now init a textures:
	CreateEmptyTex(ScreenWidth, ScreenHeight, screentex, GL_TEXTURE_2D, GL_RGB, true);
	CreateEmptyTex(ScreenWidth, ScreenHeight, rainScreenTexture, GL_TEXTURE_2D, GL_RGB, true);
	CreateEmptyTex(400, 300, rainScreenTextureLow, GL_TEXTURE_2D, GL_RGB, true);

	if (!LoadShaderFile("shaders/raindrop.arf", fp_raindrop))
	{
		gEngfuncs.Con_Printf("Unable to load raindrop frag. shader\n");
		return;
	}
	else
		gEngfuncs.Con_Printf("Raindrop frag. shader loaded successfully\n");

	if (!LoadShaderFile("shaders/rain.arf", fp_rain))
	{
		gEngfuncs.Con_Printf("Unable to load rain frag. shader\n");
		return;
	}
	else
		gEngfuncs.Con_Printf("Rain frag. shader loaded successfully\n");

	gEngfuncs.Con_Printf("Raindrops has been inited;\n");

	for (int i = 1; i <= 25; i++){
		char szName[64];
		sprintf(szName, "gfx/rainDrops/640_%i.tga", i);
		rainTextures[i - 1] = CreateTexture(szName, MIPS_YES);

		if (!rainTextures[i - 1])
			gEngfuncs.Con_Printf("Unable to load %s\n", szName);
	}
	for (int i = 1; i <= 25; i++){
		char szName[64];
		sprintf(szName, "gfx/rainDropsLow/640_%i.tga", i);
		rainTexturesLow[i - 1] = CreateTexture(szName, MIPS_YES);

		if (!rainTexturesLow[i - 1])
			gEngfuncs.Con_Printf("Unable to load %s\n", szName);
	}

	rainSplash[0] = CreateTexture("gfx/rain/rain_hi.tga", MIPS_YES);
	rainSplash[1] = CreateTexture("gfx/rain/rain_low.tga", MIPS_YES);

	gEngfuncs.Con_Printf("rainSplash[0] = %i\n", rainSplash[0]);
	inited = true;
}

void CRainDropsEffect::CaptureScreen()
{
	gl.glBindTexture(GL_TEXTURE_2D, screentex);
	gl.glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, ScreenWidth, ScreenHeight, 0);
}

void CRainDropsEffect::DrawRaindrops()
{
	if (r_rainmode->value == 3)
		gRaindrops.CaptureScreen();
		
	ProcessRainDrops();

	CRainDrop *currentObject, *prevObject;
	currentObject = objectsHead;
	prevObject = NULL;
	float flTime = gEngfuncs.GetClientTime();
	while (currentObject)
	{
		if (currentObject->Draw(flTime))
		{
			CRainDrop *myNext = currentObject->next;
			delete currentObject;
			currentObject = myNext;

			if (prevObject)
				prevObject->next = myNext;
			else
				objectsHead = myNext;
		}
		else
		{
			prevObject = currentObject;
			currentObject = currentObject->next;
		}
	}

	if (r_rainmode->value == 0 || r_rainmode->value == 3)
		return;

	gl.glBindTexture(GL_TEXTURE_2D, screentex);
	gl.glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0,0,ScreenWidth,ScreenHeight, 0);

	gl.glDisable(GL_BLEND);
	gl.glColor4f(1, 1, 1, 1);
	gl.glActiveTextureARB(GL_TEXTURE1_ARB);
	gl.glBindTexture(GL_TEXTURE_2D, screentex);
	gl.glActiveTextureARB(GL_TEXTURE0_ARB);
	gl.glBindTexture(GL_TEXTURE_2D, rainScreenTexture);
	
	gl.glEnable(GL_FRAGMENT_PROGRAM_ARB);
	gl.glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, fp_rain);

	gGlStuff.DrawQuadRAW(ScreenWidth, 0, 0, ScreenHeight);

	gl.glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, 0);
	gl.glDisable(GL_FRAGMENT_PROGRAM_ARB);

	gl.RestoreStates();
	gl.glActiveTextureARB(GL_TEXTURE0_ARB);
}

void CRainDropsEffect::ResetRain()
{
	Reset();
}

int CRainDrop::Draw(float flTime)
{
	if (r_rainmode->value == 2 || r_rainmode->value == 3)
		return DrawHighRaindrop(flTime);
	else
		return DrawLowRaindrop(flTime);
}

int CRainDrop::DrawHighRaindrop(float flTime)
{
	float frac = (dieTime - flTime) / lifeTime;
	if (frac < 0)return 1;//delete
	if (frac > 1.0)frac = 1.0;
	frac = 1.0 - frac;

	gl.SaveStates();
	gl.glColor4f(1, 1, 1, 1);

	gl.glEnable(GL_BLEND);
	gl.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	float ex = x + RAINDROP_SIZE;
	float ey = y + RAINDROP_SIZE;

	float cosin = frac + 0.5;

	gl.glActiveTextureARB(GL_TEXTURE1_ARB);
	gl.glBindTexture(GL_TEXTURE_2D, gRaindrops.screentex);
	gl.glActiveTextureARB(GL_TEXTURE0_ARB);
	gl.glBindTexture(GL_TEXTURE_2D, texture);

	gl.glEnable(GL_FRAGMENT_PROGRAM_ARB);
	gl.glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, gRaindrops.fp_raindrop);
	glProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 0, cosin, 1.0 - frac, 0, 0);

	float xc, yc, xec, yec;
	xc = (float)x / (float)ScreenWidth;
	yc = (float)y / (float)ScreenHeight;
	xec = (float)ex / (float)ScreenWidth;
	yec = (float)ey / (float)ScreenHeight;
	yc = 1.0 - yc;
	yec = 1.0 - yec;

	glBegin(GL_QUADS);
	glTexCoord2f(0, 1);
	gl.glMultiTexCoord2fARB(GL_TEXTURE1_ARB, xc, yec);
	glVertex3f(x, y, 0);
	glTexCoord2f(0, 0);
	gl.glMultiTexCoord2fARB(GL_TEXTURE1_ARB, xc, yc);
	glVertex3f(x, ey, 0);
	glTexCoord2f(1, 0);
	gl.glMultiTexCoord2fARB(GL_TEXTURE1_ARB, xec, yc);
	glVertex3f(ex, ey, 0);
	glTexCoord2f(1, 1);
	gl.glMultiTexCoord2fARB(GL_TEXTURE1_ARB, xec, yec);
	glVertex3f(ex, y, 0);
	glEnd();

	gl.glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, 0);
	gl.glDisable(GL_FRAGMENT_PROGRAM_ARB);

	gl.RestoreStates();
	return 0;
}

int CRainDrop::DrawLowRaindrop(float flTime)
{
	float frac = (dieTime - flTime) / lifeTime;
	if (frac < 0)return 1;//delete
	if (frac > 1.0)frac = 1.0;
	frac = 1.0 - frac;

	gl.glColor4f(1, 1, 1, 1);

	gl.glEnable(GL_BLEND);
	gl.glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	float ex = x + RAINDROP_SIZE;
	float ey = y + RAINDROP_SIZE;

	gl.glActiveTextureARB(GL_TEXTURE0_ARB);
	gl.glBindTexture(GL_TEXTURE_2D, lowtexture);

	float xc, yc, xec, yec;
	xc = (float)x / (float)ScreenWidth;
	yc = (float)y / (float)ScreenHeight;
	xec = (float)ex / (float)ScreenWidth;
	yec = (float)ey / (float)ScreenHeight;
	yc = 1.0 - yc;
	yec = 1.0 - yec;

	float r = 1.0 - frac;
	r *= 0.4;
	gl.glColor4f(r,r,r, 1.0 - frac);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 1);
	gl.glMultiTexCoord2fARB(GL_TEXTURE1_ARB, xc, yec);
	glVertex3f(x, y, 0);
	glTexCoord2f(0, 0);
	gl.glMultiTexCoord2fARB(GL_TEXTURE1_ARB, xc, yc);
	glVertex3f(x, ey, 0);
	glTexCoord2f(1, 0);
	gl.glMultiTexCoord2fARB(GL_TEXTURE1_ARB, xec, yc);
	glVertex3f(ex, ey, 0);
	glTexCoord2f(1, 1);
	gl.glMultiTexCoord2fARB(GL_TEXTURE1_ARB, xec, yec);
	glVertex3f(ex, y, 0);
	glEnd();

	return 0;
}

void CRainDropsEffect::ProcessRainDrops()
{
	vec3_t vecStart;
	cl_entity_t *player = gEngfuncs.GetLocalPlayer();
	if (!player)
		return;

	if (nextAdd > gEngfuncs.GetClientTime())
		return;

	nextAdd = gEngfuncs.GetClientTime() + 0.1f;

	vecStart[0] = player->origin.x;
	vecStart[1] = player->origin.y;
	vecStart[2] = player->origin.z;

	pmtrace_t pmtrace;
	gEngfuncs.pEventAPI->EV_SetTraceHull(2);
	gEngfuncs.pEventAPI->EV_PlayerTrace(vecStart, vecStart + vec3_t(0,0,4096), PM_STUDIO_IGNORE, -1, &pmtrace);

	if (pmtrace.startsolid)
		return;

	if (gEngfuncs.PM_PointContents(pmtrace.endpos, NULL) != CONTENTS_SKY)
		return;
	
	float chance = player->curstate.angles.x+15*-1;

	chance *= -5;

//	gEngfuncs.Con_Printf("angles %f - chance %f\n", player->curstate.angles.x, chance);

	float generatedChance = gEngfuncs.pfnRandomLong(1, 100);

	
	if (generatedChance > chance && chance <= 100)
		return;

	if (chance > 95)
		nextAdd = gEngfuncs.GetClientTime() + 0.01f;

	float mintime = 0.2;
	float maxtime = 3.0;

	if (chance > 95)
	{	
		mintime = 1.0;
		maxtime = 6.0;
	}

	CRainDrop *drop = new CRainDrop(gRaindrops.rainTextures[gEngfuncs.pfnRandomLong(0, 24)],
		gRaindrops.rainTexturesLow[gEngfuncs.pfnRandomLong(0, 24)],XRES(gEngfuncs.pfnRandomLong(-1, 640-1)), YRES(gEngfuncs.pfnRandomLong(-1, 640-1)), gEngfuncs.pfnRandomFloat(mintime,maxtime));
	gRaindrops.AddObject(drop);
}