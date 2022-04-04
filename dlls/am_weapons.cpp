/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
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

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "soundent.h"
#include "gamerules.h"

// ==================================================
// Weapons entities
// ==================================================

// Melee and items
LINK_ENTITY_TO_CLASS( weapon_knife, CKnife );
LINK_ENTITY_TO_CLASS( weapon_crowbar, CKnife ); // Half-Life backward compatibility

LINK_ENTITY_TO_CLASS( weapon_flash, CFlashlight );

LINK_ENTITY_TO_CLASS( weapon_fists, CFists );

LINK_ENTITY_TO_CLASS( weapon_pda, CPDA );

LINK_ENTITY_TO_CLASS( weapon_semtex, CSemtex );
LINK_ENTITY_TO_CLASS( weapon_satchel, CSemtex ); // Half-Life backward compatibility

LINK_ENTITY_TO_CLASS( weapon_rose, CRose );

// Pistols
LINK_ENTITY_TO_CLASS( weapon_deagle, CDeagle );

LINK_ENTITY_TO_CLASS( weapon_1911, C1911 );

LINK_ENTITY_TO_CLASS( weapon_ber92f, CBeretta );

LINK_ENTITY_TO_CLASS( weapon_glock18, CGlock );
LINK_ENTITY_TO_CLASS( weapon_9mmhandgun, CGlock ); // Half-Life backward compatibility
LINK_ENTITY_TO_CLASS( weapon_glock, CGlock ); // Half-Life backward compatibility

LINK_ENTITY_TO_CLASS( weapon_anaconda, CAnaconda );
LINK_ENTITY_TO_CLASS( weapon_357, CAnaconda ); // Half-Life backward compatibility
LINK_ENTITY_TO_CLASS( weapon_python, CAnaconda ); // Half-Life backward compatibility

// SMGs
LINK_ENTITY_TO_CLASS( weapon_mp5, CMP5 );
LINK_ENTITY_TO_CLASS( weapon_9mmAR, CMP5 ); // Half-Life backward compatibility

LINK_ENTITY_TO_CLASS( weapon_p90, CP90 );

LINK_ENTITY_TO_CLASS( weapon_bizon, CBizon );

// Rifles
LINK_ENTITY_TO_CLASS( weapon_famas, CFAMAS );
LINK_ENTITY_TO_CLASS( weapon_gauss, CFAMAS ); // Half-Life backward compatibility

LINK_ENTITY_TO_CLASS( weapon_m4a1, CM4A1 );
LINK_ENTITY_TO_CLASS( weapon_egon, CM4A1 ); // Half-Life backward compatibility

LINK_ENTITY_TO_CLASS( weapon_ak47, CAK47 );
LINK_ENTITY_TO_CLASS( weapon_hornetgun, CAK47 ); // Half-Life backward compatibility

LINK_ENTITY_TO_CLASS( weapon_m249, CM249 );
LINK_ENTITY_TO_CLASS( weapon_tripmine, CM249 ); // Half-Life backward compatibility

// Shotguns
LINK_ENTITY_TO_CLASS( weapon_spas12, CSPAS12 );
LINK_ENTITY_TO_CLASS( weapon_shotgun, CSPAS12 ); // Half-Life backward compatibility

LINK_ENTITY_TO_CLASS( weapon_shotgun_non_lethal, CLessLethal );

LINK_ENTITY_TO_CLASS( weapon_usas, CUSAS );
LINK_ENTITY_TO_CLASS( weapon_snark, CUSAS ); // Half-Life backward compatibility

// Snipers
LINK_ENTITY_TO_CLASS( weapon_m14, CM14 );
LINK_ENTITY_TO_CLASS( weapon_crossbow, CM14 ); // Half-Life backward compatibility

LINK_ENTITY_TO_CLASS( weapon_l96a1, CL96A1 );

// Explosives
LINK_ENTITY_TO_CLASS( weapon_rpg, CRPG ); // Half-Life backward compatibility

// ==================================================
// Flashlight
// ==================================================

void CFlashlight::PrimaryAttack( void )
{
	if ( !( m_pPlayer->m_afButtonPressed & IN_ATTACK ) )
		return;

	if ( m_pPlayer->FlashlightIsOn() )
		m_pPlayer->FlashlightTurnOff();
	else
		m_pPlayer->FlashlightTurnOn();

	SendWeaponAnim( FireAnimation(), UseDecrement(), 0 );
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + AttackDelay();
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + AttackDelay() + 0.5;
}

void CFlashlight::Holster( int skiplocal )
{
	if ( m_pPlayer->FlashlightIsOn() )
		m_pPlayer->FlashlightTurnOff();

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.5;
}

// ==================================================
// PDA
// ==================================================

// Shepard's notes
// In an older Revenge of Love's build, I implemented Rainbows and the PDA (which was a smartphone)
// and I've found the following bugs :
//
// - It's still possible to move while having a PC opened, the fix would be to freeze the player.
// - It's possible to open a PC underwater with a trick, press the fire button on the ground and
// during the opening process, go underwater.
// - Being killed (or even hurt) while using a PC doesn't close it.

//extern int gmsgOS;

void CPDA::PrimaryAttack( void )
{
	if ( !( m_pPlayer->m_afButtonPressed & IN_ATTACK ) )
		return;

	if ( m_pPlayer->pev->waterlevel == 3 )
	{
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + AttackDelay();
		return;
	}
	SendWeaponAnim( FireAnimation(), UseDecrement(), 0 );
	SetThink( &CPDA::OpenOS );
	pev->nextthink = m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + AttackDelay();
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + AttackDelay() + 0.5;
}

void CPDA::OpenOS( void )
{
	// TODO : Implement Rainbows back
	ALERT( at_console, "TODO : Implement Rainbows back\n" );
	/*char *cCurrentMap = (char *)STRING( gpGlobals->mapname );
	MESSAGE_BEGIN( MSG_ONE, gmsgOS, NULL, m_pPlayer->pev );
		WRITE_STRING( cCurrentMap );
		WRITE_BYTE( 1 );
	MESSAGE_END();*/
	SetThink( NULL );
	pev->nextthink = gpGlobals->time;
}

// ==================================================
// Rose
// ==================================================

void CRose::PrimaryAttack( void )
{
	// TODO : Play sniff animation
	// SendWeaponAnim( FireAnimation() );
	// m_flNextPrimaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + AttackDelay();
}

// ==================================================
// M249 5.56
// ==================================================

BOOL CM249::Deploy( void )
{
	BOOL bDeployState = CBaseAMWeapon::Deploy();
	if ( bDeployState )
	{
		SetBody( (m_iClip > 10) ? 10 : m_iClip );
		return TRUE;
	}
	return FALSE;
}

void CM249::PrimaryAttack( void )
{
	CBaseAMWeapon::PrimaryAttack();
	SetBody( (m_iClip > 10) ? 10 : m_iClip );
}

void CM249::ReloadSetBelt( void )
{
	// Avoid memory leak
	if ( m_pPlayer->m_pActiveItem == NULL )
	{
		SetThink( NULL );
		return;
	}
	// Don't do that if player is no more holding the M249 or reloading it
	if ( m_pPlayer->m_pActiveItem->m_iId != WEAPON_M249 || m_fInReload == FALSE )
	{
		SetThink( NULL );
		return;
	}
	// Everything is fine, do it
	int iBullets = min( GetMaxClipAmmo() - m_iClip, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] );
	if ( iBullets > 10 )
		iBullets = 10;

	SetBody( iBullets );
	SetThink( NULL );
}

void CM249::Reload( void )
{
	int iResult = CBaseAMWeapon::DefaultReload( GetMaxClipAmmo(), (m_iClip == 0) ? ReloadEmptyAnimation() : ReloadAnimation(),
		(m_iClip == 0) ? ReloadEmptyTime() : ReloadTime() );
	if ( iResult )
	{
		SetThink( &CM249::ReloadSetBelt );
		pev->nextthink = gpGlobals->time + 3.0;
	}
}

void CM249::WeaponIdle( void )
{
	CBaseAMWeapon::WeaponIdle();
	SetBody( (m_iClip > 10) ? 10 : m_iClip );
}
