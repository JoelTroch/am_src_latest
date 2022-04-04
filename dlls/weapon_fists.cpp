/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*	Use, distribution, and modification of this source code and/or resulting
*	object code is restricted to non-commercial enhancements to products from
*	Valve LLC.  All other use, distribution, or modification is prohibited
*	without written permission from Valve LLC.
*
****/

//
// Fists
//

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "gamerules.h"

extern void FindHullIntersection( const Vector &vecSrc, TraceResult &tr, float *mins, float *maxs, edict_t *pEntity );

void CFists::Precache( void )
{
	PRECACHE_MODEL( GetPModel() );
	PRECACHE_MODEL( GetVModel() );
	PRECACHE_MODEL( GetWModel() );

	PRECACHE_SOUND( GetDeploySound() );
	PRECACHE_SOUND( GetFireSound() );
	PRECACHE_SOUND( GetFire2Sound() );
	PRECACHE_SOUND( GetSilencedFireSound() );
	PRECACHE_SOUND( GetSilencedFire2Sound() );
}

void CFists::PrimaryAttack( void )
{
	// Don't attack if we are covering ourselves
	if ( m_fInAttack > 0 )
		return;

	// Some setup
	TraceResult tr;
	UTIL_MakeVectors( m_pPlayer->pev->v_angle );
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecEnd = vecSrc + gpGlobals->v_forward * 32;
	UTIL_TraceLine( vecSrc, vecEnd, dont_ignore_monsters, ENT( m_pPlayer->pev ), &tr );

	// Animations, sound and view punch
	int iAnim = FireAnimation() + RANDOM_LONG( 0, (FireVariations() - 1) );
	float flPunch = 5.0;
	if ( iAnim >= 4 )
		flPunch = -5.0;

	switch ( RANDOM_LONG( 0, 1 ) )
	{
	case 0: EMIT_SOUND_FMOD_3D( ENT( m_pPlayer->pev ), GetSilencedFireSound(), 512.0f, 2048.0f, m_pPlayer->entindex() ); break;
	case 1: EMIT_SOUND_FMOD_3D( ENT( m_pPlayer->pev ), GetSilencedFire2Sound(), 512.0f, 2048.0f, m_pPlayer->entindex() ); break;
	}
	SendWeaponAnim( iAnim, UseDecrement(), 0 );
	m_pPlayer->ViewPunch( 0.0, flPunch, 0.0 );
	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

	// If we didn't hit something
	if ( tr.flFraction >= 1.0 )
	{
		UTIL_TraceHull( vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT( m_pPlayer->pev ), &tr );

		// If we hit the world
		if ( tr.flFraction < 1.0 )
		{
			// Calculate the point of intersection of the line (or hull) and the object we hit
			// This is and approximation of the "best" intersection
			CBaseEntity *pHit = CBaseEntity::Instance( tr.pHit );
			if ( !pHit || pHit->IsBSPModel() )
				FindHullIntersection( vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, m_pPlayer->edict() );

			vecEnd = tr.vecEndPos;	// This is the point on the actual surface (the hull could have hit space)
		}

		// Missed
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + AttackDelay();
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + AttackDelay() + 0.5;
	}
	else
	{
	    // Damage stuff
		CBaseEntity *pEntity = CBaseEntity::Instance( tr.pHit );
		ClearMultiDamage();
		pEntity->TraceAttack( m_pPlayer->pev, gSkillData.plrDmgFists, gpGlobals->v_forward, &tr, DMG_CLUB | DMG_NEVERGIB );
		ApplyMultiDamage( m_pPlayer->pev, m_pPlayer->pev );
		int fHitWorld = TRUE;
		float fvolbar = 1.0;

		// We hit an entity
		if ( pEntity )
		{
			// It is flesh
			if ( pEntity->Classify() != CLASS_NONE && pEntity->Classify() != CLASS_MACHINE )
			{
				// Set the volume
				m_pPlayer->m_iWeaponVolume = MELEE_BODYHIT_VOLUME;
				fHitWorld = FALSE;
			}
		}

		// Play texture hit sound
		// UNDONE: Calculate the correct point of intersection when we hit with the hull instead of the line
		if ( fHitWorld )
		{
			// Override the volume here, cause we play texture sounds in singleplayer,
			// and fvolbar is going to be 1 from the above call.
			if ( !g_pGameRules->IsMultiplayer() )
				fvolbar = TEXTURETYPE_PlaySound( &tr, vecSrc, vecSrc + ( vecEnd - vecSrc ) * 2, BulletType() );

			// Set the sound volume
			m_pPlayer->m_iWeaponVolume = MELEE_WALLHIT_VOLUME;
		}

		// Play hit sound
		switch ( RANDOM_LONG( 0, 1 ) )
		{
		case 0: EMIT_SOUND_FMOD_3D( ENT( m_pPlayer->pev ), GetFireSound(), 512.0f, 2048.0f, m_pPlayer->entindex() ); break;
		case 1: EMIT_SOUND_FMOD_3D( ENT( m_pPlayer->pev ), GetFire2Sound(), 512.0f, 2048.0f, m_pPlayer->entindex() ); break;
		}

		// Delays
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + AttackDelay();
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + AttackDelay() + 0.5;
	}
}

void CFists::SecondaryAttack( void )
{
	if ( m_fInAttack == 0 ) // Start covering
	{
		SendWeaponAnim( ReloadAnimation(), UseDecrement(), 0 );
		m_fInAttack = 1;
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.75;
	}
	else if ( m_fInAttack == 1 ) // Covering loop
	{
		if ( m_flTimeWeaponIdle < UTIL_WeaponTimeBase() )
		{
			SendWeaponAnim( (ReloadAnimation() + 1), UseDecrement(), 0 );
			m_fInAttack = 2;
		}
	}
	else { } // NULL
}

void CFists::WeaponIdle( void )
{
	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	if ( m_fInAttack != 0 ) // Cover out
	{
		SendWeaponAnim( (ReloadAnimation() + 2), UseDecrement(), 0 );
		m_fInAttack = 0;
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.1;
	}
	else
		SendWeaponAnim( IdleAnimation(), UseDecrement(), 0 );
}

void CFists::Holster( int skiplocal )
{
	m_fInAttack = 0;
}
