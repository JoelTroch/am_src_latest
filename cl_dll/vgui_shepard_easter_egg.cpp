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
// vgui_shepard_easter_egg.cpp
//
// My little easter egg for ARRANGEMENT ^^
// When dead, making the Konami Code (Up, Up, Down, Down, Left, Right, Left, Right, B, A)
// will play a Metal Gear Solid like death scene I made ^^
//

#include "hud.h"
#include "cl_util.h"
#include "fmod_engine.h"
#include "keydefs.h"
#include "vgui_TeamFortressViewport.h"
#include "vgui_shepard_easter_egg.h"

CShepardEasterEgg::CShepardEasterEgg()
{
	m_iState = 0;
}

void CShepardEasterEgg::Reset()
{
	m_iState = 0;
	// This thing is very interesting that I need to write this note here for
	// future reference.
	// Initially, the server ordered when to enable/disable the easter egg.
	// With further experiments, it seems that GoldSrc dislike when you send
	// messages just before a transition (map change/map command/whatever).
	// Morale : NEVER send messages if a map transition is involved just after.
	gHUD.m_iAllowShepardEasterEgg = 0;
	gEngfuncs.pfnClientCmd( "reload\n" );
}

int CShepardEasterEgg::KeyInput( int down, int keynum, const char *pszCurrentBinding )
{
	if ( !down )
		return 1; // Don't disable releasing of the keys

	switch ( keynum )
	{
		case 98: // B
		{
			if ( m_iState == 8 )
				m_iState++;
			else
				Reset();

			return 0;
		}
		case 113: // A
		{
			if ( m_iState == 9 )
			{
				gFMOD.PlayMusic( "am_training_005.ogg", false, false );
				m_iState++;
			}
			else
				Reset();

			return 0;
		}
		case K_UPARROW:
		{
			if ( m_iState <= 1 )
				m_iState++;
			else
				Reset();

			return 0;
		}
		case K_DOWNARROW:
		{
			if ( m_iState == 2 || m_iState == 3 )
				m_iState++;
			else
				Reset();

			return 0;
		}
		case K_LEFTARROW:
		{
			if ( m_iState == 4 || m_iState == 6 )
				m_iState++;
			else
				Reset();

			return 0;
		}
		case K_RIGHTARROW:
		{
			if ( m_iState == 5 || m_iState == 7 )
				m_iState++;
			else
				Reset();

			return 0;
		}
		default:
		{
			Reset();
			return 0;
		}
	}
	return 0;
}

