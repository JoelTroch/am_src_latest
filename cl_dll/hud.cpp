/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
//
// hud.cpp
//
// implementation of CHud class
//

#include "hud.h"
#include "cl_util.h"
#include <string.h>
#include <stdio.h>
#include "parsemsg.h"
#include "hud_servers.h"
#include "vgui_int.h"
#include "vgui_TeamFortressViewport.h"
#include "pm_defs.h"

#include "demo.h"
#include "demo_api.h"
#include "vgui_ScorePanel.h"

#include "glmanager.h" // buz
#include "gl_renderer.h" // buz
#include "gl_texloader.h"

#include "event_api.h"
#include "event_args.h"
#include "blur.h"
#include "fog.h"
#include "ColorCorrection.h"
#include "vgui_hud.h" // buz

#include "water.h"
#include "raindrops.h"
#include "particle_engine.h"
#include "gui/cgui.h"

// Shepard : FMOD stuff
#include "fmod_engine.h"
extern CFMODEngine gFMOD;

// Shepard : SoHL rain
#include "rain.h"
#include "tri_rope.h"
#include "gl_shaders.h"

#include "lensFlare.h"
#include "SSAO.h"

hud_player_info_t	 g_PlayerInfoList[MAX_PLAYERS+1];	   // player info from the engine
extra_player_info_t  g_PlayerExtraInfo[MAX_PLAYERS+1];   // additional player info sent directly to the client dll

float LnrIntrpltn(float a, float b, float t)//zbs
{
	float x = b - (b - a)*t;
	return x;
}


class CHLVoiceStatusHelper : public IVoiceStatusHelper
{
public:
	virtual void GetPlayerTextColor(int entindex, int color[3])
	{
		color[0] = color[1] = color[2] = 255;

		if( entindex >= 0 && entindex < sizeof(g_PlayerExtraInfo)/sizeof(g_PlayerExtraInfo[0]) )
		{
			int iTeam = g_PlayerExtraInfo[entindex].teamnumber;

			if ( iTeam < 0 )
			{
				iTeam = 0;
			}

			iTeam = iTeam % iNumberOfTeamColors;

			color[0] = iTeamColors[iTeam][0];
			color[1] = iTeamColors[iTeam][1];
			color[2] = iTeamColors[iTeam][2];
		}
	}

	virtual void UpdateCursorState()
	{
		gViewPort->UpdateCursorState();
	}

	virtual int	GetAckIconHeight()
	{
		return ScreenHeight - gHUD.m_iFontHeight*3 - 6;
	}

	virtual bool			CanShowSpeakerLabels()
	{
		if( gViewPort && gViewPort->m_pScoreBoard )
			return !gViewPort->m_pScoreBoard->isVisible();
		else
			return false;
	}
};
static CHLVoiceStatusHelper g_VoiceStatusHelper;


extern client_sprite_t *GetSpriteList(client_sprite_t *pList, const char *psz, int iRes, int iCount);

extern cvar_t *sensitivity;
cvar_t *cl_lw = NULL;

void ShutdownInput (void);
#include "specMessages.h"

extern vec3_t ev_punchangle;
void V_PunchAxis(int, float);
void fireWeapon(int weapon, int entity,vec3_t endPos,int material);

int __MsgFunc_AddRope(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	vec3_t start_source;
	vec3_t end_source;

	int index = READ_BYTE();

	start_source.x = READ_COORD();
	start_source.y = READ_COORD();
	start_source.z = READ_COORD();

	end_source.x = READ_COORD();
	end_source.y = READ_COORD();
	end_source.z = READ_COORD();

	char *datafile = READ_STRING();
	gRopeRender.CreateRope(datafile, start_source, end_source, index);
	return 0;
}

int __MsgFunc_SetBody(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);
	gHUD.m_iBody = READ_BYTE();
	cl_entity_s *view = gEngfuncs.GetViewModel();
	view->curstate.body = gHUD.m_iBody;
	return 1;
}

int __MsgFunc_SetSkin(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);
	gHUD.m_iSkin = READ_BYTE();
	cl_entity_s *view = gEngfuncs.GetViewModel();
	view->curstate.skin = gHUD.m_iSkin;
	return 1;
}

int __MsgFunc_SpecMsg(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int type;
	int val1,val2,val3;
	int idx;
	float fVal;
	float p, y, r, w,t;
	pmtrace_t tr;
	p = y = r = w = 1.0;

	type = READ_BYTE();
	cl_entity_t *ent;
	vec3_t tVec;

	switch (type)
	{
	case CL_HUDTYPE:
		val1 = READ_BYTE();
//		SetHudType(val1);
		break;
	case CL_FOG:
		val1 = READ_BYTE();
		val2 = READ_BYTE();
		val3 = READ_BYTE();
		p = READ_COORD();
		y = READ_COORD();

		gHUD.fogValues[0] = (float)val1/255.0f;
		gHUD.fogValues[1] = (float)val2 / 255.0f;
		gHUD.fogValues[2] = (float)val3 / 255.0f;
		gHUD.fogValues[3] = p;
		gHUD.fogValues[4] = y;

		gEngfuncs.Con_Printf("Fog: %f %f\n", p, y);
		break;
	case CL_SHOTPARTICLE:
		val1 = READ_BYTE();
		idx = READ_SHORT();
		p = READ_COORD();
		y = READ_COORD();
		r = READ_COORD();
		val2 = READ_CHAR();
		fireWeapon(val1,idx, Vector(p,y,r),val2);
		break;
	case CL_IRON_ENABLE:
		val1 = READ_BYTE();
		gHUD.useiron = val1;
		gHUD.ironalpha = 1.0;
		break;
	case CL_PLAYERTEAM:
		idx = READ_BYTE();
		val1 = READ_BYTE();
		break;
	case CL_CLIPYAW:
		val1 = READ_SHORT();
		gHUD.yaw_clip = val1 - 1;
		break;
	case CL_SHOWNONVGUIMENU:
		gHUD.iUseMenu = READ_BYTE();
		break;
	case CL_HUDCOLOR:
		gHUD.hudcolor[0] = READ_BYTE();
		gHUD.hudcolor[1] = READ_BYTE();
		gHUD.hudcolor[2] = READ_BYTE();
		break;
	case CL_BARREL:
		gHUD.wpbarrel[0] = READ_COORD();
		gHUD.wpbarrel[1] = READ_COORD();
		break;
	case CL_IRONSIGHT:
		gHUD.ironsight[0] = READ_COORD();
		gHUD.ironsight[1] = READ_COORD();
		gHUD.ironsight[2] = READ_COORD();
		gEngfuncs.Con_Printf("Got ironsight: %f %f %f\n",
			gHUD.ironsight[0], gHUD.ironsight[1], gHUD.ironsight[2]);
		break;
	case CL_RELOADING:
		gHUD.reloading = READ_BYTE();
		gHUD.reload_time = READ_COORD();
		gEngfuncs.GetViewAngles((float *)gHUD.oldangles);
/*	case CL_PUNCH:
		p = READ_COORD();
		y = READ_COORD();
		r = READ_COORD();

		if (p + y + r == -768)
		{
			ev_punchangle[0] = ev_punchangle[1] = ev_punchangle[2] = 0;
			gEngfuncs.Con_Printf("Resetting punchangle\n;");
			break;
		}

		V_PunchAxis(0, p);
		V_PunchAxis(1, y);
		V_PunchAxis(2, r);
		break;*/
	case CL_CLIPSIZE:
		gHUD.clip_size = READ_BYTE();
		break;
	case CL_HURTIMAGE:
		gHUD.hurt = READ_BYTE();
		break;
	case CL_FRAGS:
		extern extra_player_info_t  g_PlayerExtraInfo[MAX_PLAYERS + 1];   // additional player info sent directly to the client dll
		idx = READ_BYTE();
		val1 = READ_SHORT();
		g_PlayerExtraInfo[idx].frags = val1;
		break;
	case CL_TEAM:
		idx = READ_BYTE();
		val1 = READ_BYTE();
		g_PlayerExtraInfo[idx].teamnumber = val1;
		break;
	case CL_DEATHS:
		extern extra_player_info_t  g_PlayerExtraInfo[MAX_PLAYERS + 1];   // additional player info sent directly to the client dll
		idx = READ_BYTE();
		val1 = READ_SHORT();
		g_PlayerExtraInfo[idx].deaths = val1;
		break;
	case CL_BLURSTATE:
		p = READ_COORD();
		y = READ_COORD();
		r = READ_COORD();
		gBlur.SetNew(p, y, r);
		break;
	case CL_SUNPARAMS:
		p = READ_COORD();
		y = READ_COORD();
		r = READ_COORD();
		w = READ_COORD();//pitch
		gLensFlares.SetSunParams(vec3_t(p, y, r), w);
	case CL_DEAD:
		gHUD.deadflags = READ_BYTE();
		break;
	case CL_CCORRECTION:
		p = (float)READ_SHORT() / 100.0f;
		y = (float)READ_SHORT() / 100.0f;
		r = (float)READ_SHORT() / 100.0f;
		w = (float)READ_SHORT() / 100.0f;
		t = READ_COORD();

		gColorCorrection.SetNew(p, y, r, w, t);
		break;
	}


	return 0;
}

//DECLARE_MESSAGE(m_Logo, Logo)
int __MsgFunc_Logo(const char *pszName, int iSize, void *pbuf)
{
	return gHUD.MsgFunc_Logo(pszName, iSize, pbuf );
}

//DECLARE_MESSAGE(m_Logo, Logo)
int __MsgFunc_ResetHUD(const char *pszName, int iSize, void *pbuf)
{
	return gHUD.MsgFunc_ResetHUD(pszName, iSize, pbuf );
}

int __MsgFunc_InitHUD(const char *pszName, int iSize, void *pbuf)
{
	gHUD.MsgFunc_InitHUD( pszName, iSize, pbuf );
	return 1;
}

int __MsgFunc_ViewMode(const char *pszName, int iSize, void *pbuf)
{
	gHUD.MsgFunc_ViewMode( pszName, iSize, pbuf );
	return 1;
}

int __MsgFunc_SetFOV(const char *pszName, int iSize, void *pbuf)
{
	return gHUD.MsgFunc_SetFOV( pszName, iSize, pbuf );
}

int __MsgFunc_Concuss(const char *pszName, int iSize, void *pbuf)
{
	return gHUD.MsgFunc_Concuss( pszName, iSize, pbuf );
}

int __MsgFunc_GameMode(const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_GameMode( pszName, iSize, pbuf );
}

int __MsgFunc_Particles(const char *pszName, int iSize, void *pbuf)
{
	return gParticleEngine.MsgCreateSystem(pszName, iSize, pbuf);
}

int __MsgFunc_Recoil(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_Recoil(pszName, iSize, pbuf);
	return 0;
}
// TFFree Command Menu
void __CmdFunc_OpenCommandMenu(void)
{
	if ( gViewPort )
	{
		gViewPort->ShowCommandMenu( gViewPort->m_StandardMenu );
	}
}

// TFC "special" command
void __CmdFunc_InputPlayerSpecial(void)
{
	if ( gViewPort )
	{
		gViewPort->InputPlayerSpecial();
	}
}

void __CmdFunc_CloseCommandMenu(void)
{
	if ( gViewPort )
	{
		gViewPort->InputSignalHideCommandMenu();
	}
}

void __CmdFunc_ForceCloseCommandMenu( void )
{
	if ( gViewPort )
	{
		gViewPort->HideCommandMenu();
	}
}

void __CmdFunc_ToggleServerBrowser( void )
{
	if ( gViewPort )
	{
		gViewPort->ToggleServerBrowser();
	}
}

int __MsgFunc_MenuWindow(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_ShowWeaponMenuWindow(pszName, iSize, pbuf);
	return 0;
}

// TFFree Command Menu Message Handlers
int __MsgFunc_ValClass(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_ValClass( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_Feign(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_Feign( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_Detpack(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_Detpack( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_VGUIMenu(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_VGUIMenu( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_MOTD(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_MOTD( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_BuildSt(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_BuildSt( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_RandomPC(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_RandomPC( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_ServerName(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_ServerName( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_ScoreInfo(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_ScoreInfo( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_TeamScore(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_TeamScore( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_TeamInfo(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_TeamInfo( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_Spectator(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_Spectator( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_SpecFade(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_SpecFade( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_ResetFade(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_ResetFade( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_AllowSpec(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_AllowSpec( pszName, iSize, pbuf );
	return 0;
}

// Shepard : SoHL rain
// G-Cont. rain message
int __MsgFunc_RainData( const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_RainData( pszName, iSize, pbuf );
}

// Shepard : FMOD stuff
int __MsgFunc_FMODMusicA( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	char *cStream = READ_STRING();
	bool bLoop = READ_BYTE();
	gFMOD.PlayMusic( cStream, bLoop, true );
	return 1;
}

int __MsgFunc_FMODMusicS( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	char *cStream = READ_STRING();
	bool bLoop = READ_BYTE();
	gFMOD.PlayMusic( cStream, bLoop, false );
	return 1;
}

int __MsgFunc_FMODReverb( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	int iReverb = READ_BYTE();
	gFMOD.SetReverb( iReverb );
	return 1;
}

int __MsgFunc_FMODStop( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	bool bAction = READ_BYTE();
	gFMOD.StopMusic( (bAction) ? true : false );
	return 1;
}

int __MsgFunc_FMODSwitch( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	bool bMode = READ_BYTE();
	gFMOD.m_bAction = bMode;
	return 1;
}

/*int __MsgFunc_FMODVoice( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	gFMOD.PlayFredVoice( READ_STRING() );
	return 1;
}*/

int __MsgFunc_FMOD2D( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	char *cSound = READ_STRING();
	float flPitch = READ_COORD();
	gFMOD.Play2DSound( cSound, flPitch );
	return 1;
}

int __MsgFunc_FMOD3D( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	float fPosX = READ_COORD();
	float fPosY = READ_COORD();
	float fPosZ = READ_COORD();
	char *cSample = READ_STRING();
	float fMinDistance = READ_COORD();
	float fMaxDistance = READ_COORD();
	int iEntityIndexToFollow = READ_LONG();
	float flPitch = READ_COORD();
	Vector vecPos( fPosX, fPosY, fPosZ );
	gFMOD.Play3DSoundAtPosition( vecPos, cSample, fMinDistance, fMaxDistance, iEntityIndexToFollow, flPitch );
	return 1;
}

int __MsgFunc_ShepardEE( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	gHUD.m_iAllowShepardEasterEgg = READ_BYTE();
	return 1;
}

// This is called every time the DLL is loaded
void CHud :: Init( void )
{
	gl.Init();
	RendererInit();
	void InitGlows();
	InitGlows();

	void RadioIconInit();
	RadioIconInit(); // buz
	void SubtitleInit();
	SubtitleInit(); // buz
	void RadioIconInit(); 
	RadioIconInit(); // buz

	HOOK_MESSAGE( Logo );
	HOOK_MESSAGE( ResetHUD );
	HOOK_MESSAGE( GameMode );
	HOOK_MESSAGE( InitHUD );
	HOOK_MESSAGE( ViewMode );
	HOOK_MESSAGE( SetFOV );
	HOOK_MESSAGE( Concuss );

	// TFFree CommandMenu
	HOOK_COMMAND( "+commandmenu", OpenCommandMenu );
	HOOK_COMMAND( "-commandmenu", CloseCommandMenu );
	HOOK_COMMAND( "ForceCloseCommandMenu", ForceCloseCommandMenu );
	HOOK_COMMAND( "special", InputPlayerSpecial );
	HOOK_COMMAND( "togglebrowser", ToggleServerBrowser );

	HOOK_MESSAGE( ValClass );
	HOOK_MESSAGE( Feign );
	HOOK_MESSAGE( Detpack );
	HOOK_MESSAGE( MOTD );
	HOOK_MESSAGE( BuildSt );
	HOOK_MESSAGE( RandomPC );
	HOOK_MESSAGE( ServerName );
	HOOK_MESSAGE( ScoreInfo );
	HOOK_MESSAGE( TeamScore );
	HOOK_MESSAGE( TeamInfo );
	HOOK_MESSAGE( Particles );

	HOOK_MESSAGE( Spectator );
	HOOK_MESSAGE( AllowSpec );
	HOOK_MESSAGE( SpecMsg );
	HOOK_MESSAGE( SetBody );
	HOOK_MESSAGE( SetSkin );

	HOOK_MESSAGE( SpecFade );
	HOOK_MESSAGE( ResetFade );
	HOOK_MESSAGE( AddRope );

	// VGUI Menus
	HOOK_MESSAGE( VGUIMenu );
	HOOK_MESSAGE(Recoil);
	HOOK_MESSAGE(MenuWindow);

	// Shepard : SoHL rain
	HOOK_MESSAGE( RainData ); // G-Cont. for rain control

	// Shepard : FMOD stuff
	HOOK_MESSAGE( FMODMusicA );
	HOOK_MESSAGE( FMODMusicS );
	HOOK_MESSAGE( FMODReverb );
	HOOK_MESSAGE( FMODStop );
	HOOK_MESSAGE( FMODSwitch );
	HOOK_MESSAGE( FMOD2D );
	HOOK_MESSAGE( FMOD3D );
	m_pCvarFMODDebugShowCPUUsage = CVAR_CREATE( "fmod_debug_show_cpu_usage", "0", FCVAR_ARCHIVE );
#ifdef WIN32
	m_pCvarFMODMuteWhenNotFocused = CVAR_CREATE( "fmod_mute_when_not_focused", "0", FCVAR_ARCHIVE );
#endif
	m_pCvarFMODOutput = CVAR_CREATE( "fmod_output_mode", "0", FCVAR_ARCHIVE );
	m_pCvarFMODSpeaker = CVAR_CREATE( "fmod_speaker_mode", "0", FCVAR_ARCHIVE );

	RainInfo = gEngfuncs.pfnRegisterVariable( "cl_raininfo", "0", 0 );

	CVAR_CREATE( "hud_classautokill", "1", FCVAR_ARCHIVE | FCVAR_USERINFO );		// controls whether or not to suicide immediately on TF class switch
	CVAR_CREATE( "hud_takesshots", "0", FCVAR_ARCHIVE );		// controls whether or not to automatically take screenshots at the end of a round

	HOOK_MESSAGE( ShepardEE );
	m_iAllowShepardEasterEgg = 0;

	m_iLogo = 0;
	m_iFOV = 0;

	CVAR_CREATE( "zoom_sensitivity_ratio", "1.2", 0 );
	default_fov = CVAR_CREATE( "default_fov", "90", 0 );
	m_pCvarStealMouse = CVAR_CREATE( "hud_capturemouse", "1", FCVAR_ARCHIVE );
	m_pCvarDraw = CVAR_CREATE( "hud_draw", "1", FCVAR_ARCHIVE );
	cl_lw = gEngfuncs.pfnGetCvarPointer( "cl_lw" );

	m_pSpriteList = NULL;
	clip_size = 1;
	// Clear any old HUD list
	if ( m_pHudList )
	{
		HUDLIST *pList;
		while ( m_pHudList )
		{
			pList = m_pHudList;
			m_pHudList = m_pHudList->pNext;
			free( pList );
		}
		m_pHudList = NULL;
	}

	// In case we get messages before the first update -- time will be valid
	m_flTime = 1.0;

	m_Ammo.Init();
	m_Health.Init();
	m_SayText.Init();
	m_Spectator.Init();
	m_Geiger.Init();
	m_Train.Init();
	m_Battery.Init();
	m_Message.Init();
	m_StatusBar.Init();
	m_DeathNotice.Init();
	m_AmmoSecondary.Init();
	m_TextMessage.Init();
	m_StatusIcons.Init();

	GetClientVoiceMgr()->Init(&g_VoiceStatusHelper, (vgui::Panel**)&gViewPort);

	m_Menu.Init();

	ServersInit();
	gParticleEngine.Init();
	MsgFunc_ResetHUD(0, 0, NULL );

	// Shepard : SoHL rain
	InitRain();
	initTextures();

	m_Tbutton.Init();
}

// CHud destructor
// cleans up memory allocated for m_rg* arrays
CHud :: ~CHud()
{
	delete [] m_rghSprites;
	delete [] m_rgrcRects;
	delete [] m_rgszSpriteNames;

	// Shepard : SoHL rain
	ResetRain();

	if ( m_pHudList )
	{
		HUDLIST *pList;
		while ( m_pHudList )
		{
			pList = m_pHudList;
			m_pHudList = m_pHudList->pNext;
			free( pList );
		}
		m_pHudList = NULL;
	}

	if (gGui)
	delete gGui;
	ServersShutdown();
}

// GetSpriteIndex()
// searches through the sprite list loaded from hud.txt for a name matching SpriteName
// returns an index into the gHUD.m_rghSprites[] array
// returns 0 if sprite not found
int CHud :: GetSpriteIndex( const char *SpriteName )
{
	// look through the loaded sprite name list for SpriteName
	for ( int i = 0; i < m_iSpriteCount; i++ )
	{
		if ( strncmp( SpriteName, m_rgszSpriteNames + (i * MAX_SPRITE_NAME_LENGTH), MAX_SPRITE_NAME_LENGTH ) == 0 )
			return i;
	}

	return -1; // invalid sprite
}

void CHud::InitEffects()
{
	if (effectsInited)
		return;

	effectsInited = true;
	LoadHDTextures();

}
// This is called every time the DLL is loaded
void LoadTextures();

void CHud :: VidInit( void )
{
	m_scrinfo.iSize = sizeof(m_scrinfo);
	GetScreenInfo(&m_scrinfo);
	gl.VidInit();
	gRopeRender.ResetRopes();
	gFMOD.Reset();

	if (!gGui)
		gGui = new CGui();

	effectsInited = false;
	RendererVidInit();
	void ResetGlows();
	ResetGlows();

	static wrect_t nullrc;
	SetCrosshair(0, nullrc, 0, 0, 0);

	//reset overlays
	if (gViewPort && gViewPort->m_pHud2)
		gViewPort->m_pHud2->ResetOverlays();

	// ----------
	// Load Sprites
	// ---------
//	m_hsprFont = LoadSprite("sprites/%d_font.spr");
	clip_size = 1;

	m_hsprLogo = 0;
	m_hsprCursor = 0;
	// Shepard : SoHL rain
	ResetRain();
	
	LoadTextures();

	if (ScreenWidth < 640)
		m_iRes = 320;
	else
		m_iRes = 640;

	// Only load this once
	if ( !m_pSpriteList )
	{
		// we need to load the hud.txt, and all sprites within
		m_pSpriteList = SPR_GetList("sprites/hud.txt", &m_iSpriteCountAllRes);

		if (m_pSpriteList)
		{
			// count the number of sprites of the appropriate res
			m_iSpriteCount = 0;
			client_sprite_t *p = m_pSpriteList;
			int j;
			for ( j = 0; j < m_iSpriteCountAllRes; j++ )
			{
				if ( p->iRes == m_iRes )
					m_iSpriteCount++;
				p++;
			}

			// allocated memory for sprite handle arrays
 			m_rghSprites = new HLSPRITE[m_iSpriteCount];
			m_rgrcRects = new wrect_t[m_iSpriteCount];
			m_rgszSpriteNames = new char[m_iSpriteCount * MAX_SPRITE_NAME_LENGTH];

			p = m_pSpriteList;
			int index = 0;
			for ( j = 0; j < m_iSpriteCountAllRes; j++ )
			{
				if ( p->iRes == m_iRes )
				{
					char sz[256];
					sprintf(sz, "sprites/%s.spr", p->szSprite);
					m_rghSprites[index] = SPR_Load(sz);
					m_rgrcRects[index] = p->rc;
					strncpy( &m_rgszSpriteNames[index * MAX_SPRITE_NAME_LENGTH], p->szName, MAX_SPRITE_NAME_LENGTH );

					index++;
				}

				p++;
			}
		}
	}
	else
	{
		// we have already have loaded the sprite reference from hud.txt, but
		// we need to make sure all the sprites have been loaded (we've gone through a transition, or loaded a save game)
		client_sprite_t *p = m_pSpriteList;
		int index = 0;
		for ( int j = 0; j < m_iSpriteCountAllRes; j++ )
		{
			if ( p->iRes == m_iRes )
			{
				char sz[256];
				sprintf( sz, "sprites/%s.spr", p->szSprite );
				m_rghSprites[index] = SPR_Load(sz);
				index++;
			}

			p++;
		}
	}

	// assumption: number_1, number_2, etc, are all listed and loaded sequentially
	m_HUD_number_0 = GetSpriteIndex( "number_0" );

	m_iFontHeight = m_rgrcRects[m_HUD_number_0].bottom - m_rgrcRects[m_HUD_number_0].top;

	m_Ammo.VidInit();
	m_Health.VidInit();
	m_Spectator.VidInit();
	m_Geiger.VidInit();
	m_Train.VidInit();
	m_Battery.VidInit();
	m_Message.VidInit();
	m_StatusBar.VidInit();
	m_DeathNotice.VidInit();
	m_SayText.VidInit();
	m_Menu.VidInit();
	m_AmmoSecondary.VidInit();
	m_TextMessage.VidInit();
	m_StatusIcons.VidInit();

	GetClientVoiceMgr()->VidInit();

	gGlow.ResetGlow();
	gGlow.InitGlow();
	gBlur.ResetBlur();
	gBlur.InitBlur();
	gFog.ResetFog();
	gFog.InitFog();
	gLensFlares.InitLensFlares();

	gColorCorrection.ResetColorCorrection();
	gColorCorrection.InitColorCorrection();
	gSSAO.InitSSAO();

	gRaindrops.ResetRain();
	gRaindrops.InitRain();

	gParticleEngine.VidInit();
	gWater.InitWater();

	m_Tbutton.VidInit();

	loadAllShaders();
}

int CHud::MsgFunc_Logo(const char *pszName,  int iSize, void *pbuf)
{
	BEGIN_READ( pbuf, iSize );

	// update Train data
	m_iLogo = READ_BYTE();

	return 1;
}

float g_lastFOV = 0.0;

/*
============
COM_FileBase
============
*/
// Extracts the base name of a file (no path, no extension, assumes '/' as path separator)
void COM_FileBase ( const char *in, char *out)
{
	int len, start, end;

	len = strlen( in );

	// scan backward for '.'
	end = len - 1;
	while ( end && in[end] != '.' && in[end] != '/' && in[end] != '\\' )
		end--;

	if ( in[end] != '.' )		// no '.', copy to end
		end = len-1;
	else
		end--;					// Found ',', copy to left of '.'


	// Scan backward for '/'
	start = len-1;
	while ( start >= 0 && in[start] != '/' && in[start] != '\\' )
		start--;

	if ( in[start] != '/' && in[start] != '\\' )
		start = 0;
	else
		start++;

	// Length of new sting
	len = end - start + 1;

	// Copy partial string
	strncpy( out, &in[start], len );
	// Terminate it
	out[len] = 0;
}

/*
=================
HUD_IsGame

=================
*/
int HUD_IsGame( const char *game )
{
	const char *gamedir;
	char gd[ 1024 ];

	gamedir = gEngfuncs.pfnGetGameDirectory();
	if ( gamedir && gamedir[0] )
	{
		COM_FileBase( gamedir, gd );
		if ( !stricmp( gd, game ) )
			return 1;
	}
	return 0;
}

/*
=====================
HUD_GetFOV

Returns last FOV
=====================
*/
float HUD_GetFOV( void )
{
	if ( gEngfuncs.pDemoAPI->IsRecording() )
	{
		// Write it
		int i = 0;
		unsigned char buf[ 100 ];

		// Active
		*( float * )&buf[ i ] = g_lastFOV;
		i += sizeof( float );

		Demo_WriteBuffer( TYPE_ZOOM, i, buf );
	}

	if ( gEngfuncs.pDemoAPI->IsPlayingback() )
	{
		g_lastFOV = g_demozoom;
	}
	return g_lastFOV;
}

int CHud::MsgFunc_SetFOV(const char *pszName,  int iSize, void *pbuf)
{
	BEGIN_READ( pbuf, iSize );

	int newfov = READ_BYTE();
	int def_fov = CVAR_GET_FLOAT("default_fov");

	//	g_lastFOV = m_iFOV;

	if (newfov == 0)
	{
		m_iNewFOV = def_fov;
	}
	else
	{
		m_iNewFOV = newfov;
	}

	// the clients fov is actually set in the client data update section of the hud

	// Set a new sensitivity
	if (m_iNewFOV == def_fov)
	{
		// reset to saved sensitivity
		m_flMouseSensitivity = 0;
	}
	else
	{
		// set a new sensitivity that is proportional to the change from the FOV default
		m_flMouseSensitivity = sensitivity->value * ((float)newfov / (float)def_fov) * CVAR_GET_FLOAT("zoom_sensitivity_ratio");
	}

	return 1;
}


void CHud::AddHudElem(CHudBase *phudelem)
{
	HUDLIST *pdl, *ptemp;

//phudelem->Think();

	if (!phudelem)
		return;

	pdl = (HUDLIST *)malloc(sizeof(HUDLIST));
	if (!pdl)
		return;

	memset(pdl, 0, sizeof(HUDLIST));
	pdl->p = phudelem;

	if (!m_pHudList)
	{
		m_pHudList = pdl;
		return;
	}

	ptemp = m_pHudList;

	while (ptemp->pNext)
		ptemp = ptemp->pNext;

	ptemp->pNext = pdl;
}

float CHud::GetSensitivity( void )
{
	return m_flMouseSensitivity;
}
