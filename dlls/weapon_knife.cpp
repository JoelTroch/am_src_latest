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

//
// Knife
//

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "gamerules.h"

void CKnife::Precache( void )
{
	PRECACHE_MODEL( GetPModel() );
	PRECACHE_MODEL( GetVModel() );
	PRECACHE_MODEL( GetWModel() );

	PRECACHE_SOUND( GetDeploySound() );
	PRECACHE_SOUND( GetFireSound() );
	PRECACHE_SOUND( GetFire2Sound() );
	PRECACHE_SOUND( GetFire3Sound() );
	PRECACHE_SOUND( "weapons/knife/hit_world-1.wav" );
	PRECACHE_SOUND( "weapons/knife/hit_world-2.wav" );
	PRECACHE_SOUND( "weapons/knife/swing.wav" );
	PRECACHE_SOUND( GetSilencedFireSound() );
	PRECACHE_SOUND( GetSilencedFire2Sound() );
	PRECACHE_SOUND( "weapons/knife/stab_hit_world-1.wav" );
	PRECACHE_SOUND( "weapons/knife/stab_hit_world-2.wav" );
}

void FindHullIntersection( const Vector &vecSrc, TraceResult &tr, float *mins, float *maxs, edict_t *pEntity )
{
	int			i, j, k;
	float		distance;
	float		*minmaxs[2] = {mins, maxs};
	TraceResult tmpTrace;
	Vector		vecHullEnd = tr.vecEndPos;
	Vector		vecEnd;

	distance = 1e6f;

	vecHullEnd = vecSrc + ( ( vecHullEnd - vecSrc ) * 2 );
	UTIL_TraceLine( vecSrc, vecHullEnd, dont_ignore_monsters, pEntity, &tmpTrace );
	if ( tmpTrace.flFraction < 1.0 )
	{
		tr = tmpTrace;
		return;
	}

	for ( i = 0; i < 2; i++ )
	{
		for ( j = 0; j < 2; j++ )
		{
			for ( k = 0; k < 2; k++ )
			{
				vecEnd.x = vecHullEnd.x + minmaxs[i][0];
				vecEnd.y = vecHullEnd.y + minmaxs[j][1];
				vecEnd.z = vecHullEnd.z + minmaxs[k][2];

				UTIL_TraceLine( vecSrc, vecEnd, dont_ignore_monsters, pEntity, &tmpTrace );
				if ( tmpTrace.flFraction < 1.0 )
				{
					float thisDistance = (tmpTrace.vecEndPos - vecSrc).Length();
					if ( thisDistance < distance )
					{
						tr = tmpTrace;
						distance = thisDistance;
					}
				}
			}
		}
	}
}

void CKnife::PrimaryAttack( void )
{
	// Some setup
	TraceResult tr;
	UTIL_MakeVectors( m_pPlayer->pev->v_angle );
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecEnd = vecSrc + gpGlobals->v_forward * 32;
	UTIL_TraceLine( vecSrc, vecEnd, dont_ignore_monsters, ENT( m_pPlayer->pev ), &tr );

	// Animations, sound and view punch
	int iAnim = FireAnimation() + RANDOM_LONG( 0, (FireVariations() - 1) );
	SendWeaponAnim( iAnim, UseDecrement(), 0 );
	EMIT_SOUND_FMOD_3D( ENT( m_pPlayer->pev ), "weapons/knife/swing.wav", 512.0f, 2048.0f, m_pPlayer->entindex() );
	m_pPlayer->ViewPunch( 0.0, 3.0, 0.0 );
	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

	// If we didn't hit something
	if ( tr.flFraction >= 1.0 )
	{
		UTIL_TraceHull( vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT( m_pPlayer->pev ), &tr );

		// If we hit the world
		if ( tr.flFraction < 1.0 )
		{
			// Calculate the point of intersection of the line (or hull) and the object we hit
			// This is an approximation of the "best" intersection
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
		pEntity->TraceAttack( m_pPlayer->pev, gSkillData.plrDmgKnife, gpGlobals->v_forward, &tr, DMG_SLASH | DMG_NEVERGIB );
		ApplyMultiDamage( m_pPlayer->pev, m_pPlayer->pev );
		int fHitWorld = TRUE;

		// We hit an entity
		if ( pEntity )
		{
			// It is flesh
			if ( pEntity->Classify() != CLASS_NONE && pEntity->Classify() != CLASS_MACHINE )
			{
				// Play flesh hit sound and set the volume
				switch ( RANDOM_LONG( 0, 2 ) )
				{
				case 0: EMIT_SOUND_FMOD_3D( ENT( m_pPlayer->pev ), GetFireSound(), 512.0f, 2048.0f, m_pPlayer->entindex() ); break;
				case 1: EMIT_SOUND_FMOD_3D( ENT( m_pPlayer->pev ), GetFire2Sound(), 512.0f, 2048.0f, m_pPlayer->entindex() ); break;
				case 2: EMIT_SOUND_FMOD_3D( ENT( m_pPlayer->pev ), GetFire3Sound(), 512.0f, 2048.0f, m_pPlayer->entindex() ); break;
				}
				m_pPlayer->m_iWeaponVolume = MELEE_BODYHIT_VOLUME;
				fHitWorld = FALSE;
			}
		}

		// Play texture hit sound
		// UNDONE: Calculate the correct point of intersection when we hit with the hull instead of the line
		if ( fHitWorld )
		{
			TEXTURETYPE_PlaySound( &tr, vecSrc, vecSrc + ( vecEnd - vecSrc ) * 2, BulletType() );

			// Also play world hit sound
			switch ( RANDOM_LONG( 0, 1 ) )
			{
			case 0: EMIT_SOUND_FMOD_3D( ENT( m_pPlayer->pev ), "weapons/knife/hit_world-1.wav", 512.0f, 2048.0f, m_pPlayer->entindex() ); break;
			case 1: EMIT_SOUND_FMOD_3D( ENT( m_pPlayer->pev ), "weapons/knife/hit_world-2.wav", 512.0f, 2048.0f, m_pPlayer->entindex() ); break;
			}

			// Delay the decal a bit and set the sound volume
			m_trHit = tr;
			m_pPlayer->m_iWeaponVolume = MELEE_WALLHIT_VOLUME;
		}

		// Delays
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + AttackDelay();
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + AttackDelay() + 0.5;

		// Decal
		// BUG : It doesn't work
		SetThink( &CKnife::Smack );
		pev->nextthink = UTIL_WeaponTimeBase() + AttackDelay();
	}
}

void CKnife::SecondaryAttack( void )
{
	// Some setup
	TraceResult tr;
	UTIL_MakeVectors( m_pPlayer->pev->v_angle );
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecEnd = vecSrc + gpGlobals->v_forward * 32;
	UTIL_TraceLine( vecSrc, vecEnd, dont_ignore_monsters, ENT( m_pPlayer->pev ), &tr );

	// Animations and sound
	EMIT_SOUND_FMOD_3D( ENT( m_pPlayer->pev ), "weapons/knife/swing.wav", 512.0f, 2048.0f, m_pPlayer->entindex() );
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
		m_pPlayer->ViewPunch( -3.0, 0.0, 0.0 );
		SendWeaponAnim( ReloadAnimation(), UseDecrement(), 0 );
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + AttackDelay();
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + AttackDelay() + 0.5;
	}
	else
	{
		// Damage stuff
		CBaseEntity *pEntity = CBaseEntity::Instance( tr.pHit );
		ClearMultiDamage();
		pEntity->TraceAttack( m_pPlayer->pev, gSkillData.plrDmgKnifeStab, gpGlobals->v_forward, &tr, DMG_SLASH | DMG_NEVERGIB );
		ApplyMultiDamage( m_pPlayer->pev, m_pPlayer->pev );
		int fHitWorld = TRUE;

		// We hit an entity
		if ( pEntity )
		{
			// It is flesh
			if ( pEntity->Classify() != CLASS_NONE && pEntity->Classify() != CLASS_MACHINE )
			{
				// Play flesh hit sound, set the volume, play the animation and view punch
				switch ( RANDOM_LONG( 0, 1 ) )
				{
				case 0: EMIT_SOUND_FMOD_3D( ENT( m_pPlayer->pev ), GetSilencedFireSound(), 512.0f, 2048.0f, m_pPlayer->entindex() ); break;
				case 1: EMIT_SOUND_FMOD_3D( ENT( m_pPlayer->pev ), GetSilencedFire2Sound(), 512.0f, 2048.0f, m_pPlayer->entindex() ); break;
				}
				m_pPlayer->ViewPunch( 0.0, 3.0, 0.0 );
				SendWeaponAnim( (ReloadAnimation() + 1), UseDecrement(), 0 );
				m_pPlayer->m_iWeaponVolume = MELEE_BODYHIT_VOLUME;
				fHitWorld = FALSE;
			}
		}

		// Play texture hit sound
		// UNDONE: Calculate the correct point of intersection when we hit with the hull instead of the line
		if ( fHitWorld )
		{
			TEXTURETYPE_PlaySound( &tr, vecSrc, vecSrc + (vecEnd - vecSrc) * 2, BulletType() );

			// Also play world hit sound, animation and view punch
			switch ( RANDOM_LONG( 0, 1 ) )
			{
			case 0: EMIT_SOUND_FMOD_3D( ENT( m_pPlayer->pev ), "weapons/knife/stab_hit_world-1.wav", 512.0f, 2048.0f, m_pPlayer->entindex() ); break;
			case 1: EMIT_SOUND_FMOD_3D( ENT( m_pPlayer->pev ), "weapons/knife/stab_hit_world-2.wav", 512.0f, 2048.0f, m_pPlayer->entindex() ); break;
			}
			m_pPlayer->ViewPunch( 0.0, 3.0, 0.0 );
			SendWeaponAnim( (ReloadAnimation() + 1), UseDecrement(), 0 );

			// Delay the decal a bit and set the sound volume
			m_trHit = tr;
			m_pPlayer->m_iWeaponVolume = MELEE_WALLHIT_VOLUME;
		}

		// Delays
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + AttackDelay();
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + AttackDelay() + 0.5;

		// Decal
		// BUG : It doesn't work
		SetThink( &CKnife::Smack );
		pev->nextthink = UTIL_WeaponTimeBase() + AttackDelay();
	}
}

void CKnife::Smack( void )
{
	DecalGunshot( &m_trHit, BulletType() );
}
