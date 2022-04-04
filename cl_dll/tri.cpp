//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

// Triangle rendering, if any

#include "hud.h"
#include "cl_util.h"

// Triangle rendering apis are in gEngfuncs.pTriAPI

#include "const.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "triangleapi.h"

#include "gl_renderer.h" // p_render
#include "blur.h"
#include "fog.h"
#include "colorcorrection.h"
#include "water.h"
#include "raindrops.h"
#include "r_studioint.h"
extern engine_studio_api_t IEngineStudio;

#include "com_model.h"
#include "studio.h"
#include "particle_engine.h"

#include "StudioModelRenderer.h"
#include "GameStudioModelRenderer.h"

// Shepard : SoHL rain
#include "rain.h"
#include "studio_util.h"
#include "glStuff.h"

#include "gui/cgui.h"
#include "tri_rope.h"
#include "lensFlare.h"
#include "SSAO.h"

#define DLLEXPORT __declspec( dllexport )
extern CGameStudioModelRenderer g_StudioRenderer;

extern "C"
{
	void DLLEXPORT HUD_DrawNormalTriangles( void );
	void DLLEXPORT HUD_DrawTransparentTriangles( void );
};

/*
=================
HUD_DrawNormalTriangles

Non-transparent triangles-- add them here
=================
*/
extern float g_lastFOV;
void DLLEXPORT HUD_DrawNormalTriangles( void )
{
	gHUD.currentPVS = GetCurrentPVS();
	int GetCurrentVisFrame();
	gl.m_iVisFrame = GetCurrentVisFrame();

	gHUD.m_Spectator.DrawOverview();
	if (gGui)
		gGui->think(gEngfuncs.GetClientTime());

	if (gHUD.m_iNewFOV <= 0)
		gHUD.m_iNewFOV = 90.0;
	if (gHUD.m_iFOV <= 0)
		gHUD.m_iFOV = 90.0;
	//g_lastFOV = 90.0;

	if (gHUD.m_iFOV != gHUD.m_iNewFOV) // HAWK : Update Fov
	{
		gHUD.m_iFOV -= (float)(gHUD.m_iFOV - gHUD.m_iNewFOV) / 4.0f;
		g_lastFOV = gHUD.m_iFOV;
		gHUD.lastFOVchange = gEngfuncs.GetClientTime();
	}
	if ((gHUD.lastFOVchange + 1 < gEngfuncs.GetClientTime()) && gHUD.m_iNewFOV >= 85 && ((gHUD.m_iFOV != 90) || (g_lastFOV != 90)))
	{
	//	gEngfuncs.Con_Printf("resetting fov %i\n", gHUD.m_iFOV);
		g_lastFOV = 90;
		gHUD.m_iFOV = 90;
		gHUD.lastFOVchange = gEngfuncs.GetClientTime();
	}

	if (gHUD.lastFOVchange + 0.1 < gEngfuncs.GetClientTime())
	{
		g_lastFOV = gHUD.m_iNewFOV;
		gHUD.m_iFOV = gHUD.m_iNewFOV;
		gHUD.lastFOVchange = gEngfuncs.GetClientTime();
	}

	RendererDrawNormal(); 

	// Render particles
	gParticleEngine.DrawSpecialParticles();
	gParticleEngine.DrawParticles();

	gWater.CaptureScreen();
	gWater.DrawWater();
	
	gl.SaveStates();
	
	extern unsigned int m_iScreen;
	glBindTexture(GL_TEXTURE_2D, m_iScreen);
	glCopyTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, 0,0, ScreenWidth, ScreenHeight );

	gl.glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	float gldepthmin, gldepthmax;
	gldepthmin = 0;
	gldepthmax = 1;

	gl.glMatrixMode(GL_MODELVIEW);
	gl.glDepthRange(gldepthmin, gldepthmin + 0.3f * (gldepthmax - gldepthmin));
	g_StudioRenderer.StudioDrawViewmodel(STUDIO_EVENTS);
	gl.glDepthRange(gldepthmin, gldepthmax);
	gl.RestoreStates();

}

// Shepard : SoHL rain

// Make a reference to SetPoint from glows.cpp
extern void SetPoint( float x, float y, float z, float( *matrix )[4] );

/*
=================================
DrawRain

draw raindrips and snowflakes
=================================
*/
extern cl_drip FirstChainDrip;
extern rain_properties Rain;

void DrawRain( void )
{
	float proj[16];

	if (FirstChainDrip.p_Next == NULL)
	{
		gl.glGetFloatv(GL_PROJECTION_MATRIX, proj);
		gl.glMatrixMode(GL_PROJECTION);
		gl.glLoadIdentity();
		gl.glOrtho(0, ScreenWidth, ScreenHeight, 0, -99999, 99999);
		gl.glViewport(0, 0, ScreenWidth, ScreenHeight);

		gl.glMatrixMode(GL_MODELVIEW);
		gl.glPushMatrix();
		gl.glLoadIdentity();

		gl.glDepthMask(GL_FALSE);
		gl.glDisable(GL_DEPTH_TEST);
		gl.glDisable(GL_CULL_FACE);

		glColor4f(1, 1, 1, 1);
		gFog.DrawEffect();

		gl.glDepthMask(GL_TRUE);
		gl.glEnable(GL_DEPTH_TEST);
		gl.glEnable(GL_CULL_FACE);

		gl.glMatrixMode(GL_MODELVIEW);
		gl.glPopMatrix();

		gl.glMatrixMode(GL_PROJECTION);
		gl.glLoadMatrixf(proj);
		return; // no drips to draw
	}

//	gEngfuncs.Con_Printf("=== tri.cpp - DrawRain ===");

	HLSPRITE hsprTexture;
	const model_s *pTexture;
	float visibleHeight = Rain.globalHeight - SNOWFADEDIST;

	if ( Rain.weatherMode == 0 )
		hsprTexture = LoadSprite( "sprites/hi_rain.spr" ); // load rain sprite
	else
		hsprTexture = LoadSprite( "sprites/snowflake.spr" ); // load snow sprite

	if (Rain.weatherMode == 0) // draw rain
		gRaindrops.CaptureScreen();

	if (Rain.weatherMode == 0) // draw rain
	{
		gl.glGetFloatv(GL_PROJECTION_MATRIX, proj);

		gl.glMatrixMode(GL_PROJECTION);
		gl.glLoadIdentity();
		gl.glOrtho(0, ScreenWidth, ScreenHeight, 0, -99999, 99999);
		gl.glViewport(0, 0, ScreenWidth, ScreenHeight);

		gl.glMatrixMode(GL_MODELVIEW);
		gl.glPushMatrix();
		gl.glLoadIdentity();

		gl.glDepthMask(GL_FALSE);
		gl.glDisable(GL_DEPTH_TEST);
		gl.glDisable(GL_CULL_FACE);

		glColor4f(0.5, 0.5, 1.0, 1);
		gGlStuff.DrawQuad(0, 0, 0, ScreenWidth, ScreenHeight, false);

		gl.glDepthMask(GL_TRUE);
		gl.glEnable(GL_DEPTH_TEST);
		gl.glEnable(GL_CULL_FACE);

		gl.glMatrixMode(GL_MODELVIEW);
		gl.glPopMatrix();

		gl.glMatrixMode(GL_PROJECTION);
		gl.glLoadMatrixf(proj);

		gl.glEnable(GL_BLEND);
		gl.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		gl.glActiveTextureARB(GL_TEXTURE0_ARB);
		gl.glBindTexture(GL_TEXTURE_2D, gRaindrops.rainSplash[0]);
		gl.glDisable(GL_CULL_FACE);
	}

	gl.glDepthMask(GL_FALSE);
	// go through drips list
	cl_drip* Drip = FirstChainDrip.p_Next;
	cl_entity_t *player = gEngfuncs.GetLocalPlayer();

	if ( Rain.weatherMode == 0 ) // draw rain
	{
		while ( Drip != NULL )
		{
			cl_drip* nextdDrip = Drip->p_Next;

			Vector2D toPlayer;
			toPlayer.x = player->origin[0] - Drip->origin[0];
			toPlayer.y = player->origin[1] - Drip->origin[1];
			toPlayer = toPlayer.Normalize();

			toPlayer.x *= DRIP_SPRITE_HALFWIDTH;
			toPlayer.y *= DRIP_SPRITE_HALFWIDTH;

			float shiftX = (Drip->xDelta / DRIPSPEED) * DRIP_SPRITE_HALFHEIGHT;
			float shiftY = (Drip->yDelta / DRIPSPEED) * DRIP_SPRITE_HALFHEIGHT;

			gl.glColor4f(1, 1, 1, 1);
			gl.glBegin(GL_TRIANGLES);

			gl.glTexCoord2f(0, 0);
			gl.glVertex3f(Drip->origin[0] - toPlayer.y - shiftX, Drip->origin[1] + toPlayer.x - shiftY, Drip->origin[2] + DRIP_SPRITE_HALFHEIGHT);

			gl.glTexCoord2f(0.5, 1);
			gl.glVertex3f(Drip->origin[0] + shiftX, Drip->origin[1] + shiftY, Drip->origin[2] - DRIP_SPRITE_HALFHEIGHT);

			gl.glTexCoord2f(1, 0);
			gl.glVertex3f(Drip->origin[0] + toPlayer.y - shiftX, Drip->origin[1] - toPlayer.x - shiftY, Drip->origin[2] + DRIP_SPRITE_HALFHEIGHT);

			gl.glEnd();
			Drip = nextdDrip;
		}

	gl.glBindTexture(GL_TEXTURE_2D, gRaindrops.rainScreenTexture);
	gl.glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, ScreenWidth, ScreenHeight, 0);

	gl.glGetFloatv(GL_PROJECTION_MATRIX, proj);

	gl.glMatrixMode(GL_PROJECTION);
	gl.glLoadIdentity();
	gl.glOrtho(0, ScreenWidth, ScreenHeight, 0, -99999, 99999);
	gl.glViewport(0, 0, ScreenWidth, ScreenHeight);

	gl.glMatrixMode(GL_MODELVIEW);
	gl.glPushMatrix();
	gl.glLoadIdentity();

	gl.glDepthMask(GL_FALSE);
	gl.glDisable(GL_DEPTH_TEST);
	gl.glDisable(GL_CULL_FACE);

	glColor4f(1,1,1,1);
	/*
	gl.glViewport(0, 0, 400, 300);
	gl.glBindTexture(GL_TEXTURE_2D, gRaindrops.rainScreenTexture);

	//draw our screen quad in small VP.
	void DrawQuadS(float a, float b, int c, float d, float e, bool f);
	DrawQuadS(0, 0, gRaindrops.rainScreenTexture, ScreenWidth, ScreenHeight, false);

	//and copy it into small texture.
	gl.glBindTexture(GL_TEXTURE_2D, gRaindrops.rainScreenTextureLow);
	gl.glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, 400, 300, 0);

	gl.glViewport(0, 0, ScreenWidth, ScreenHeight);
	*/
	gGlStuff.DrawQuad(ScreenWidth, 0, gRaindrops.screentex, 0, ScreenHeight, false);
	gRaindrops.DrawRaindrops();
	gFog.DrawEffect();

	gl.glDepthMask(GL_TRUE);
	gl.glEnable(GL_DEPTH_TEST);
	gl.glEnable(GL_CULL_FACE);

	gl.glMatrixMode(GL_MODELVIEW);
	gl.glPopMatrix();

	gl.glMatrixMode(GL_PROJECTION);
	gl.glLoadMatrixf(proj);
	}

	Drip = FirstChainDrip.p_Next;
	if (Rain.weatherMode != 0)
	{
		vec3_t normal;
		gEngfuncs.GetViewAngles((float*)normal);

		float matrix[3][4];
		AngleMatrix(normal, matrix);	// calc view matrix

		while (Drip != NULL)
		{
			cl_drip* nextdDrip = Drip->p_Next;

			matrix[0][3] = Drip->origin[0]; // write origin to matrix
			matrix[1][3] = Drip->origin[1];
			matrix[2][3] = Drip->origin[2];

			// apply start fading effect
			float alpha = (Drip->origin[2] <= visibleHeight) ? Drip->alpha : ((Rain.globalHeight - Drip->origin[2]) / (float)SNOWFADEDIST) * Drip->alpha;

			// --- draw quad --------------------------
			gEngfuncs.pTriAPI->Color4f(1.0, 1.0, 1.0, alpha);
			gEngfuncs.pTriAPI->Begin(TRI_QUADS);

			gEngfuncs.pTriAPI->TexCoord2f(0, 0);
			SetPoint(0, SNOW_SPRITE_HALFSIZE, SNOW_SPRITE_HALFSIZE, matrix);

			gEngfuncs.pTriAPI->TexCoord2f(0, 1);
			SetPoint(0, SNOW_SPRITE_HALFSIZE, -SNOW_SPRITE_HALFSIZE, matrix);

			gEngfuncs.pTriAPI->TexCoord2f(1, 1);
			SetPoint(0, -SNOW_SPRITE_HALFSIZE, -SNOW_SPRITE_HALFSIZE, matrix);

			gEngfuncs.pTriAPI->TexCoord2f(1, 0);
			SetPoint(0, -SNOW_SPRITE_HALFSIZE, SNOW_SPRITE_HALFSIZE, matrix);

			gEngfuncs.pTriAPI->End();
			// --- draw quad end ----------------------

			Drip = nextdDrip;
		}
	}
	gl.glDepthMask(GL_TRUE);
}

void DrawRainLow()
{
	if (FirstChainDrip.p_Next == NULL)
		return; // no drips to draw

	HLSPRITE hsprTexture;
	const model_s *pTexture;
	float visibleHeight = Rain.globalHeight - SNOWFADEDIST;

	if (Rain.weatherMode == 0)
		hsprTexture = LoadSprite("sprites/hi_rain.spr"); // load rain sprite
	else
		hsprTexture = LoadSprite("sprites/snowflake.spr"); // load snow sprite

	gl.glEnable(GL_BLEND);
	gl.glBlendFunc(GL_ONE,GL_ONE);
	gl.glActiveTextureARB(GL_TEXTURE0_ARB);
	gl.glBindTexture(GL_TEXTURE_2D, gRaindrops.rainSplash[1]);
	gl.glDisable(GL_CULL_FACE);

	gl.glDepthMask(GL_FALSE);

	// go through drips list
	cl_drip* Drip = FirstChainDrip.p_Next;
	cl_entity_t *player = gEngfuncs.GetLocalPlayer();

	if (Rain.weatherMode == 0) // draw rain
	{
		while (Drip != NULL)
		{
			cl_drip* nextdDrip = Drip->p_Next;

			Vector2D toPlayer;
			toPlayer.x = player->origin[0] - Drip->origin[0];
			toPlayer.y = player->origin[1] - Drip->origin[1];
			toPlayer = toPlayer.Normalize();

			toPlayer.x *= DRIP_SPRITE_HALFWIDTH;
			toPlayer.y *= DRIP_SPRITE_HALFWIDTH;

			float shiftX = (Drip->xDelta / DRIPSPEED) * DRIP_SPRITE_HALFHEIGHT;
			float shiftY = (Drip->yDelta / DRIPSPEED) * DRIP_SPRITE_HALFHEIGHT;

			gl.glColor4f(1, 1, 1, 1);
			gl.glBegin(GL_TRIANGLES);

			gl.glTexCoord2f(0, 0);
			gl.glVertex3f(Drip->origin[0] - toPlayer.y - shiftX, Drip->origin[1] + toPlayer.x - shiftY, Drip->origin[2] + DRIP_SPRITE_HALFHEIGHT);

			gl.glTexCoord2f(0.5, 1);
			gl.glVertex3f(Drip->origin[0] + shiftX, Drip->origin[1] + shiftY, Drip->origin[2] - DRIP_SPRITE_HALFHEIGHT);

			gl.glTexCoord2f(1, 0);
			gl.glVertex3f(Drip->origin[0] + toPlayer.y - shiftX, Drip->origin[1] - toPlayer.x - shiftY, Drip->origin[2] + DRIP_SPRITE_HALFHEIGHT);

			gl.glEnd();
			Drip = nextdDrip;
		}
	}
	else	// draw snow
	{
		vec3_t normal;
		gEngfuncs.GetViewAngles((float*)normal);

		float matrix[3][4];
		AngleMatrix(normal, matrix);	// calc view matrix

		while (Drip != NULL)
		{
			cl_drip* nextdDrip = Drip->p_Next;

			matrix[0][3] = Drip->origin[0]; // write origin to matrix
			matrix[1][3] = Drip->origin[1];
			matrix[2][3] = Drip->origin[2];

			// apply start fading effect
			float alpha = (Drip->origin[2] <= visibleHeight) ? Drip->alpha : ((Rain.globalHeight - Drip->origin[2]) / (float)SNOWFADEDIST) * Drip->alpha;

			// --- draw quad --------------------------
			gEngfuncs.pTriAPI->Color4f(1.0, 1.0, 1.0, alpha);
			gEngfuncs.pTriAPI->Begin(TRI_QUADS);

			gEngfuncs.pTriAPI->TexCoord2f(0, 0);
			SetPoint(0, SNOW_SPRITE_HALFSIZE, SNOW_SPRITE_HALFSIZE, matrix);

			gEngfuncs.pTriAPI->TexCoord2f(0, 1);
			SetPoint(0, SNOW_SPRITE_HALFSIZE, -SNOW_SPRITE_HALFSIZE, matrix);

			gEngfuncs.pTriAPI->TexCoord2f(1, 1);
			SetPoint(0, -SNOW_SPRITE_HALFSIZE, -SNOW_SPRITE_HALFSIZE, matrix);

			gEngfuncs.pTriAPI->TexCoord2f(1, 0);
			SetPoint(0, -SNOW_SPRITE_HALFSIZE, SNOW_SPRITE_HALFSIZE, matrix);

			gEngfuncs.pTriAPI->End();
			// --- draw quad end ----------------------

			Drip = nextdDrip;
		}
	}

	gl.glDepthMask(GL_TRUE);
}
/*
=================================
DrawFXObjects
=================================
*/
extern cl_rainfx FirstChainFX;

void DrawFXObjects( void )
{
	if ( FirstChainFX.p_Next == NULL )
		return; // no objects to draw

	float curtime = gEngfuncs.GetClientTime();

	// usual triapi stuff
	HLSPRITE hsprTexture;
	const model_s *pTexture;
	hsprTexture = LoadSprite( "sprites/waterring.spr" ); // load water ring sprite
	pTexture = gEngfuncs.GetSpritePointer( hsprTexture );
	gEngfuncs.pTriAPI->SpriteTexture( (struct model_s *)pTexture, 0 );
	gEngfuncs.pTriAPI->RenderMode( kRenderTransAdd );
	gEngfuncs.pTriAPI->CullFace( TRI_NONE );

	// go through objects list
	cl_rainfx* curFX = FirstChainFX.p_Next;
	while ( curFX != NULL )
	{
		cl_rainfx* nextFX = curFX->p_Next;

		// fadeout
		float alpha = ((curFX->birthTime + curFX->life - curtime) / curFX->life) * curFX->alpha;
		float size = (curtime - curFX->birthTime) * MAXRINGHALFSIZE;

		// --- draw quad --------------------------
		gEngfuncs.pTriAPI->Color4f( 1.0, 1.0, 1.0, alpha );
		gEngfuncs.pTriAPI->Begin( TRI_QUADS );

		gEngfuncs.pTriAPI->TexCoord2f( 0, 0 );
		gEngfuncs.pTriAPI->Vertex3f( curFX->origin[0] - size, curFX->origin[1] - size, curFX->origin[2] );

		gEngfuncs.pTriAPI->TexCoord2f( 0, 1 );
		gEngfuncs.pTriAPI->Vertex3f( curFX->origin[0] - size, curFX->origin[1] + size, curFX->origin[2] );

		gEngfuncs.pTriAPI->TexCoord2f( 1, 1 );
		gEngfuncs.pTriAPI->Vertex3f( curFX->origin[0] + size, curFX->origin[1] + size, curFX->origin[2] );

		gEngfuncs.pTriAPI->TexCoord2f( 1, 0 );
		gEngfuncs.pTriAPI->Vertex3f( curFX->origin[0] + size, curFX->origin[1] - size, curFX->origin[2] );

		gEngfuncs.pTriAPI->End();
		// --- draw quad end ----------------------

		curFX = nextFX;
	}
}

/*
=================
HUD_DrawTransparentTriangles

Render any triangles with transparent rendermode needs here
=================
*/

void DrawGlows(); // p_render

void DLLEXPORT HUD_DrawTransparentTriangles( void )
{
	// p_render
	RendererDrawTransparent();
	gRopeRender.DrawRopes(1);

	DrawGlows();
	// p_render

	float proj[16];
	float model[16];

	// Shepard : SoHL rain
	ProcessFXObjects();
	ProcessRain();

	if (r_rainmode->value == 0 || r_rainmode->value == 3)
	{
		DrawRainLow();
	}
	else
		DrawRain();

	DrawFXObjects();

	gl.glGetFloatv(GL_PROJECTION_MATRIX, proj);
	gl.glGetFloatv(GL_MODELVIEW_MATRIX, proj);

	gl.glMatrixMode(GL_PROJECTION);
	gl.glLoadIdentity();
	gl.glOrtho(0, ScreenWidth, ScreenHeight, 0, -99999, 99999);
	gl.glViewport(0, 0, ScreenWidth, ScreenHeight);

	gl.glMatrixMode(GL_MODELVIEW);
	gl.glPushMatrix();
	gl.glLoadIdentity();

	gl.glDepthMask(GL_FALSE);
	gl.glDisable(GL_DEPTH_TEST);
	gl.glDisable(GL_CULL_FACE);
	
	if (r_rainmode->value == 0 || r_rainmode->value == 3){
		gFog.DrawEffect();
		gRaindrops.DrawRaindrops();	
	}

	if (gGui)
		gGui->draw(gEngfuncs.GetClientTime());

	gSSAO.DrawEffect();
 
	gLensFlares.DrawEffect();
//	gColorCorrection.DrawEffect();
	
	gBlur.DrawEffect();
	gGlow.DrawEffect();
	gColorCorrection.DrawEffect();

	gGlStuff.DrawScreenOV();
	//gGlStuff.DrawQuad(ScreenWidth, 0, gParticleEngine.m_uiDepthMap, 0, ScreenHeight);

	gl.glDepthMask(GL_TRUE);
	gl.glEnable(GL_DEPTH_TEST);
	gl.glEnable(GL_CULL_FACE);

	gl.glMatrixMode(GL_MODELVIEW);
	gl.glPopMatrix();

	gl.glMatrixMode(GL_PROJECTION);
	gl.glLoadMatrixf(proj);

}
