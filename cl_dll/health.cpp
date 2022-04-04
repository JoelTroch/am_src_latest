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
// Health.cpp
//
// implementation of CHudHealth class
//

#include "stdio.h"
#include "stdlib.h"
#include "math.h"

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include <string.h>
#include "gl_renderer.h"
#include "glStuff.h"

#include "vgui_TeamFortressViewport.h" // buz
#include "vgui_hud.h" // buz

#include "ammohistory.h"//WeaponsResource

DECLARE_MESSAGE(m_Health, Health )
DECLARE_MESSAGE(m_Health, Damage )

#define PAIN_NAME "sprites/%d_pain.spr"
#define DAMAGE_NAME "sprites/%d_dmg.spr"

int giDmgHeight, giDmgWidth;

int giDmgFlags[NUM_DMG_TYPES] = 
{
	DMG_POISON,
	DMG_ACID,
	DMG_FREEZE|DMG_SLOWFREEZE,
	DMG_DROWN,
	DMG_BURN|DMG_SLOWBURN,
	DMG_NERVEGAS, 
	DMG_RADIATION,
	DMG_SHOCK,
	DMG_CALTROP,
	DMG_TRANQ,
	DMG_CONCUSS,
	DMG_HALLUC
};

int CHudHealth::Init(void)
{
	HOOK_MESSAGE(Health);
	HOOK_MESSAGE(Damage);
	m_iHealth = 100;
	m_fFade = 0;
	m_iFlags = 0;
	m_bitsDamage = 0;
	m_fAttackFront = m_fAttackRear = m_fAttackRight = m_fAttackLeft = 0;
	giDmgHeight = 0;
	giDmgWidth = 0;

	crossmaxsize = 15;
	crossminsize = 4;
	crosslength = 0;
	crosssize = 0;
	crosstype = 0;
	crossstate = 0;

	memset(m_dmg, 0, sizeof(DAMAGE_IMAGE) * NUM_DMG_TYPES);

	m_fFade = 0;
	gHUD.AddHudElem(this);
	return 1;
}

void CHudHealth::Reset( void )
{
	crossmaxsize = 15;
	crossminsize = 4;
	crosslength = 0;
	crosssize = 0;
	crosstype = 0;
	crossstate = 0;

	// make sure the pain compass is cleared when the player respawns
	m_fAttackFront = m_fAttackRear = m_fAttackRight = m_fAttackLeft = 0;
	m_fFade = 0;

	// force all the flashing damage icons to expire
	m_bitsDamage = 0;
	for ( int i = 0; i < NUM_DMG_TYPES; i++ )
	{
		m_dmg[i].fExpire = 0;
	}
}

int CHudHealth::VidInit(void)
{
	crossmaxsize = 10;
	crossminsize = 4;
	crosslength = 0;
	crosssize = 0;
	crosstype = 0;
	crossstate = 0;

	m_hSprite = 0;
	m_fFade = 0;

	m_HUD_dmg_bio = gHUD.GetSpriteIndex( "dmg_bio" ) + 1;

	giDmgHeight = gHUD.GetSpriteRect(m_HUD_dmg_bio).right - gHUD.GetSpriteRect(m_HUD_dmg_bio).left;
	giDmgWidth = gHUD.GetSpriteRect(m_HUD_dmg_bio).bottom - gHUD.GetSpriteRect(m_HUD_dmg_bio).top;
	return 1;
}

int CHudHealth:: MsgFunc_Health(const char *pszName,  int iSize, void *pbuf )
{
	// TODO: update local health data
	BEGIN_READ( pbuf, iSize );
	int x = READ_BYTE();

	m_iFlags |= HUD_ACTIVE;

	// Only update the fade if we've changed health
	if (x != m_iHealth)
	{
		m_fFade = gEngfuncs.GetClientTime();
		m_iHealth = x;
	}

	gViewPort->m_pHud2->UpdateHealth(x); // buz

	return 1;
}


int CHudHealth:: MsgFunc_Damage(const char *pszName,  int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	int armor = READ_BYTE();	// armor
	int damageTaken = READ_BYTE();	// health
	long bitsDamage = READ_LONG(); // damage bits

	vec3_t vecFrom;

	for ( int i = 0 ; i < 3 ; i++)
		vecFrom[i] = READ_COORD();

	UpdateTiles(gHUD.m_flTime, bitsDamage);

	// Actually took damage?
	if ( damageTaken > 0 || armor > 0 )
		CalcDamageDirection(vecFrom);

	return 1;
}


// Returns back a color from the
// Green <-> Yellow <-> Red ramp
void CHudHealth::GetPainColor( int &r, int &g, int &b )
{
	int iHealth = m_iHealth;

	if (iHealth > 25)
		iHealth -= 25;
	else if ( iHealth < 0 )
		iHealth = 0;
#if 0
	g = iHealth * 255 / 100;
	r = 255 - g;
	b = 0;
#else
	if (m_iHealth > 25)
	{
		UnpackRGB(r,g,b, RGB_YELLOWISH);
	}
	else
	{
		r = 250;
		g = 0;
		b = 0;
	}
#endif 
}

int CHudHealth::Draw(float flTime)
{
	int r, g, b;
	int a = 0, x, y;
	int HealthWidth;

	if ((gHUD.m_iHideHUDDisplay & HIDEHUD_HEALTH) || gEngfuncs.IsSpectateOnly())
		return 1;

	int width = 180;
	int height = 45;
	x = 8;
	y = ScreenHeight - 212;

	// Only draw health if we have the suit.
	if (gHUD.m_iWeaponBits & (1 << (WEAPON_SUIT)))
	{
		/*	HealthWidth = gHUD.GetSpriteRect(gHUD.m_HUD_number_0).right - gHUD.GetSpriteRect(gHUD.m_HUD_number_0).left;
			int CrossWidth = gHUD.GetSpriteRect(m_HUD_cross).right - gHUD.GetSpriteRect(m_HUD_cross).left;

			y = ScreenHeight - gHUD.m_iFontHeight - gHUD.m_iFontHeight / 2;
			x = CrossWidth /2;

			SPR_Set(gHUD.GetSprite(m_HUD_cross), r, g, b);
			SPR_DrawAdditive(0, x, y, &gHUD.GetSpriteRect(m_HUD_cross));

			x = CrossWidth + HealthWidth / 2;

			x = gHUD.DrawHudNumber(x, y, DHN_3DIGITS | DHN_DRAWZERO, m_iHealth, r, g, b);

			x += HealthWidth/2;

			int iHeight = gHUD.m_iFontHeight;
			int iWidth = HealthWidth/10;
			FillRGBA(x, y, iWidth, iHeight, 255, 160, 0, a);*/
	}

	WEAPON *pw = gHUD.m_Ammo.m_pWeapon; // shorthand
	if (pw)
	{
	//	gEngfuncs.Con_Printf("iClip [%i]\n", pw->iClip);
		if (gHUD.m_Tbutton.m_iFlags & HUD_ACTIVE)
		{
			//draw nothing
		}
		else
		{
			if (pw->iAmmoType > 0)
				if (pw->iClip != 0)
					if (gHUD.m_iFOV >= 87)
						if (gHUD.m_iWeaponBits & (1 << (WEAPON_SUIT)))
							DrawCrosshair(flTime);
		}
	}

	DrawDamage(flTime);
	return DrawPain(flTime);
}
extern float velLen;
void CHudHealth::DrawCrosshair(float fTime)
{

	//ONLY FOR HAWK CS-LIKE CODE.
	//float maxspeed = CVAR_GET_FLOAT("cl_forwardspeed");

	//HAWK : For AM
	float maxspeed = 50;//walk speed
	float addSize = velLen / maxspeed;

	if (crosssize > crossmaxsize)
		crosssize = crossmaxsize;

	if (crosssize > crossminsize)
		crosssize -= gHUD.m_flTimeDelta * 10;


	if (crosssize <= crossminsize)
		crosssize = crossminsize;


	crosslength = crosssize*0.8 + 4;


	float fFinalAlpha = (7.2 - crosslength-1.5)*0.1;
//	gEngfuncs.Con_Printf("fFinalAlpha [%f]\n", fFinalAlpha);

	int posx, posy;
	posx = ScreenWidth / 2 - crosssize - addSize;
	posy = ScreenHeight / 2 - 1;

	//HAWK old green color
	//glColor4f(0,1,0,1);

	if (crossstate == 0)
		glColor4f(1, 1, 1, 1 + fFinalAlpha);
	else if (crossstate == 1)
		glColor4f(0, 1, 0, 0.5);
	else
		glColor4f(1, 0, 0, 0.5);

	gGlStuff.DrawQuad(posx, posy, 0, posx - crosslength, posy + 1, true);

	posx = ScreenWidth / 2 + crosssize + 1 + addSize;
	posy = ScreenHeight / 2 - 1;

	gGlStuff.DrawQuad(posx, posy, 0, posx + crosslength + 1, posy + 1, true);

	posx = ScreenWidth / 2;
	posy = ScreenHeight / 2 + crosssize + addSize;

	gGlStuff.DrawQuad(posx, posy, 0, posx + 1, posy + crosslength + 1, true);

	posx = ScreenWidth / 2;
	posy = ScreenHeight / 2 - crosssize - 1 - addSize;

	gGlStuff.DrawQuad(posx, posy, 0, posx + 1, posy - crosslength, true);

}

void CHudHealth::CalcDamageDirection(vec3_t vecFrom)
{
	vec3_t	forward, right, up;
	float	side, front;
	vec3_t vecOrigin, vecAngles;

	if (!vecFrom[0] && !vecFrom[1] && !vecFrom[2])
	{
		m_fAttackFront = m_fAttackRear = m_fAttackRight = m_fAttackLeft = 0;
		return;
	}


	memcpy(vecOrigin, gHUD.m_vecOrigin, sizeof(vec3_t));
	memcpy(vecAngles, gHUD.m_vecAngles, sizeof(vec3_t));


	VectorSubtract (vecFrom, vecOrigin, vecFrom);

	float flDistToTarget = vecFrom.Length();

	vecFrom = vecFrom.Normalize();
	AngleVectors (vecAngles, forward, right, up);

	front = DotProduct (vecFrom, right);
	side = DotProduct (vecFrom, forward);

	if (flDistToTarget <= 50)
	{
		m_fAttackFront = m_fAttackRear = m_fAttackRight = m_fAttackLeft = 1;
	}
	else 
	{
		if (side > 0)
		{
			if (side > 0.3)
				m_fAttackFront = max(m_fAttackFront, side);
		}
		else
		{
			float f = fabs(side);
			if (f > 0.3)
				m_fAttackRear = max(m_fAttackRear, f);
		}

		if (front > 0)
		{
			if (front > 0.3)
				m_fAttackRight = max(m_fAttackRight, front);
		}
		else
		{
			float f = fabs(front);
			if (f > 0.3)
				m_fAttackLeft = max(m_fAttackLeft, f);
		}
	}
}

int CHudHealth::DrawPain(float flTime)
{
	return 1;

	if (!(m_fAttackFront || m_fAttackRear || m_fAttackLeft || m_fAttackRight))
		return 1;

	int r, g, b;
	int x, y, a, shade;

	// TODO:  get the shift value of the health
	a = 255;	// max brightness until then

	float fFade = gHUD.m_flTimeDelta * 2;
	
	// SPR_Draw top
	if (m_fAttackFront > 0.4)
	{
		GetPainColor(r,g,b);
		shade = a * max( m_fAttackFront, 0.5 );
		ScaleColors(r, g, b, shade);
		SPR_Set(m_hSprite, r, g, b );

		x = ScreenWidth/2 - SPR_Width(m_hSprite, 0)/2;
		y = ScreenHeight/2 - SPR_Height(m_hSprite,0) * 3;
		SPR_DrawAdditive(0, x, y, NULL);
		m_fAttackFront = max( 0, m_fAttackFront - fFade );
	} else
		m_fAttackFront = 0;

	if (m_fAttackRight > 0.4)
	{
		GetPainColor(r,g,b);
		shade = a * max( m_fAttackRight, 0.5 );
		ScaleColors(r, g, b, shade);
		SPR_Set(m_hSprite, r, g, b );

		x = ScreenWidth/2 + SPR_Width(m_hSprite, 1) * 2;
		y = ScreenHeight/2 - SPR_Height(m_hSprite,1)/2;
		SPR_DrawAdditive(1, x, y, NULL);
		m_fAttackRight = max( 0, m_fAttackRight - fFade );
	} else
		m_fAttackRight = 0;

	if (m_fAttackRear > 0.4)
	{
		GetPainColor(r,g,b);
		shade = a * max( m_fAttackRear, 0.5 );
		ScaleColors(r, g, b, shade);
		SPR_Set(m_hSprite, r, g, b );

		x = ScreenWidth/2 - SPR_Width(m_hSprite, 2)/2;
		y = ScreenHeight/2 + SPR_Height(m_hSprite,2) * 2;
		SPR_DrawAdditive(2, x, y, NULL);
		m_fAttackRear = max( 0, m_fAttackRear - fFade );
	} else
		m_fAttackRear = 0;

	if (m_fAttackLeft > 0.4)
	{
		GetPainColor(r,g,b);
		shade = a * max( m_fAttackLeft, 0.5 );
		ScaleColors(r, g, b, shade);
		SPR_Set(m_hSprite, r, g, b );

		x = ScreenWidth/2 - SPR_Width(m_hSprite, 3) * 3;
		y = ScreenHeight/2 - SPR_Height(m_hSprite,3)/2;
		SPR_DrawAdditive(3, x, y, NULL);

		m_fAttackLeft = max( 0, m_fAttackLeft - fFade );
	} else
		m_fAttackLeft = 0;

	return 1;
}

int CHudHealth::DrawDamage(float flTime)
{
	return 1;

	int r, g, b, a;
	DAMAGE_IMAGE *pdmg;

	if (!m_bitsDamage)
		return 1;

	UnpackRGB(r,g,b, RGB_YELLOWISH);
	
	a = (int)( fabs(sin(flTime*2)) * 256.0);

	ScaleColors(r, g, b, a);

	// Draw all the items
	int i;
	for ( i = 0; i < NUM_DMG_TYPES; i++)
	{
		if (m_bitsDamage & giDmgFlags[i])
		{
			pdmg = &m_dmg[i];
			SPR_Set(gHUD.GetSprite(m_HUD_dmg_bio + i), r, g, b );
			SPR_DrawAdditive(0, pdmg->x, pdmg->y, &gHUD.GetSpriteRect(m_HUD_dmg_bio + i));
		}
	}


	// check for bits that should be expired
	for ( i = 0; i < NUM_DMG_TYPES; i++ )
	{
		DAMAGE_IMAGE *pdmg = &m_dmg[i];

		if ( m_bitsDamage & giDmgFlags[i] )
		{
			pdmg->fExpire = min( flTime + DMG_IMAGE_LIFE, pdmg->fExpire );

			if ( pdmg->fExpire <= flTime		// when the time has expired
				&& a < 40 )						// and the flash is at the low point of the cycle
			{
				pdmg->fExpire = 0;

				int y = pdmg->y;
				pdmg->x = pdmg->y = 0;

				// move everyone above down
				for (int j = 0; j < NUM_DMG_TYPES; j++)
				{
					pdmg = &m_dmg[j];
					if ((pdmg->y) && (pdmg->y < y))
						pdmg->y += giDmgHeight;

				}

				m_bitsDamage &= ~giDmgFlags[i];  // clear the bits
			}
		}
	}

	return 1;
}
 

void CHudHealth::UpdateTiles(float flTime, long bitsDamage)
{	
	DAMAGE_IMAGE *pdmg;

	// Which types are new?
	long bitsOn = ~m_bitsDamage & bitsDamage;
	
	for (int i = 0; i < NUM_DMG_TYPES; i++)
	{
		pdmg = &m_dmg[i];

		// Is this one already on?
		if (m_bitsDamage & giDmgFlags[i])
		{
			pdmg->fExpire = flTime + DMG_IMAGE_LIFE; // extend the duration
			if (!pdmg->fBaseline)
				pdmg->fBaseline = flTime;
		}

		// Are we just turning it on?
		if (bitsOn & giDmgFlags[i])
		{
			// put this one at the bottom
			pdmg->x = giDmgWidth/8;
			pdmg->y = ScreenHeight - giDmgHeight * 2;
			pdmg->fExpire=flTime + DMG_IMAGE_LIFE;
			
			// move everyone else up
			for (int j = 0; j < NUM_DMG_TYPES; j++)
			{
				if (j == i)
					continue;

				pdmg = &m_dmg[j];
				if (pdmg->y)
					pdmg->y -= giDmgHeight;

			}
			pdmg = &m_dmg[i];
		}	
	}	

	// damage bits are only turned on here;  they are turned off when the draw time has expired (in DrawDamage())
	m_bitsDamage |= bitsDamage;
}
