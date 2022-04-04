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
// RPG-7
//

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "gamerules.h"

LINK_ENTITY_TO_CLASS( rpg_rocket, CRPGRocket );

CRPGRocket *CRPGRocket::CreateRPGRocket( Vector vecOrigin, Vector vecAngles, CBaseEntity *pOwner, CRPG *pLauncher )
{
	CRPGRocket *pRocket = GetClassPtr( (CRPGRocket *)NULL );

	UTIL_SetOrigin( pRocket->pev, vecOrigin );
	pRocket->pev->angles = vecAngles;
	pRocket->Spawn();
	pRocket->SetTouch( &CRPGRocket::RocketTouch );
	pRocket->pev->owner = pOwner->edict();

	return pRocket;
}

void CRPGRocket::Spawn( void )
{
	Precache();

	// Motor
	pev->movetype = MOVETYPE_BOUNCE;
	pev->solid = SOLID_BBOX;

	SET_MODEL( ENT( pev ), "models/weapons/rpg/rpg_rocket.mdl" );
	UTIL_SetSize( pev, Vector( 0, 0, 0 ), Vector( 0, 0, 0 ) );
	UTIL_SetOrigin( pev, pev->origin );

	pev->classname = MAKE_STRING( "rpg_rocket" );

	SetThink( &CRPGRocket::IgniteThink );
	SetTouch( &CRPGRocket::ExplodeTouch );

	pev->angles.x -= 30;
	UTIL_MakeVectors( pev->angles );
	pev->angles.x = -(pev->angles.x + 30);

	pev->velocity = gpGlobals->v_forward * 250;
	pev->gravity = 0.5;

	pev->nextthink = gpGlobals->time + 0.4;

	pev->dmg = gSkillData.plrDmgRPG;
}

void CRPGRocket::RocketTouch( CBaseEntity *pOther )
{
	STOP_SOUND( edict(), CHAN_VOICE, "weapons/rocket1.wav" );
	ExplodeTouch( pOther );
}

void CRPGRocket::Precache( void )
{
	PRECACHE_MODEL( "models/weapons/rpg/rpg_rocket.mdl" );
	m_iTrail = PRECACHE_MODEL( "sprites/smoke.spr" );
	PRECACHE_SOUND( "weapons/rocket1.wav" );
}

void CRPGRocket::IgniteThink( void )
{
	pev->movetype = MOVETYPE_FLY;
	pev->effects |= EF_LIGHT;

	// Make rocket sound
	EMIT_SOUND( ENT( pev ), CHAN_VOICE, "weapons/rocket1.wav", 1, 0.5 );

	// Rocket trail
	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_BEAMFOLLOW );
		WRITE_SHORT( entindex() );
		WRITE_SHORT( m_iTrail );
		WRITE_BYTE( 40 );
		WRITE_BYTE( 5 );
		WRITE_BYTE( 224 );
		WRITE_BYTE( 224 );
		WRITE_BYTE( 255 );
		WRITE_BYTE( 255 );
	MESSAGE_END();

	m_flIgniteTime = gpGlobals->time;

	// Set think
	SetThink( &CRPGRocket::RocketThink );
	pev->nextthink = gpGlobals->time + 0.1;
}

void CRPGRocket::RocketThink( void )
{
	Vector vecTarget = gpGlobals->v_forward;
	UTIL_MakeAimVectors( pev->angles );

	// This acceleration and turning math is totally wrong, but it seems to respond well so don't change it.
	float flSpeed = pev->velocity.Length();
	if ( gpGlobals->time - m_flIgniteTime < 1.0 )
	{
		pev->velocity = pev->velocity * 0.2 + vecTarget * (flSpeed * 0.8 + 400);
		if ( pev->waterlevel == 3 )
		{
			// Go slow underwater
			if ( pev->velocity.Length() > 300 )
				pev->velocity = pev->velocity.Normalize() * 300;

			UTIL_BubbleTrail( pev->origin - pev->velocity * 0.1, pev->origin, 4 );
		}
		else
		{
			if ( pev->velocity.Length() > 2000 )
				pev->velocity = pev->velocity.Normalize() * 2000;
		}
	}
	else
	{
		if ( pev->effects & EF_LIGHT )
		{
			pev->effects = 0;
			STOP_SOUND( ENT( pev ), CHAN_VOICE, "weapons/rocket1.wav" );
		}
		pev->velocity = pev->velocity * 0.2 + vecTarget * flSpeed * 0.798;
		if ( pev->waterlevel == 0 && pev->velocity.Length() < 1500 )
			Detonate();
	}
	pev->nextthink = gpGlobals->time + 0.1;
}

//==================================================
//==================================================
//==================================================

BOOL CRPG::Deploy( void )
{
	BOOL bDeployState = CBaseAMWeapon::Deploy();
	if ( bDeployState )
	{
		SetBody( (m_iClip == 0) ? 0 : 1 );
		return TRUE;
	}
	return FALSE;
}

void CRPG::Precache( void )
{
	UTIL_PrecacheOther( "rpg_rocket" );
	CBaseAMWeapon::Precache();
}

void CRPG::PrimaryAttack( void )
{
	if ( m_iClip )
	{
		m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

		// Player "shoot" animation
		m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

		UTIL_MakeVectors( m_pPlayer->pev->v_angle );
		Vector vecSrc = m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 16 + gpGlobals->v_right * 8 + gpGlobals->v_up * -8;

		CRPGRocket *pRocket = CRPGRocket::CreateRPGRocket( vecSrc, m_pPlayer->pev->v_angle, m_pPlayer, this );

		UTIL_MakeVectors( m_pPlayer->pev->v_angle ); // RPGRocket::Create stomps on globals, so remake.
		pRocket->pev->velocity = pRocket->pev->velocity + gpGlobals->v_forward * DotProduct( m_pPlayer->pev->velocity, gpGlobals->v_forward );
		EMIT_SOUND_FMOD_3D( ENT( m_pPlayer->pev ), GetFireSound(), 2048.0f, 4096.0f, m_pPlayer->entindex() );
		SendWeaponAnim( FireAnimation() );
		SetBody( 0 );

#ifdef DEBUG
		if ( CVAR_GET_FLOAT( "am_wp_infinite_ammo" ) != 1 )
#endif
			m_iClip--;

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + AttackDelay();
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + AttackDelay() + 0.5;
	}
	else
		PlayEmptySound();
}

void CRPG::Reload( void )
{
	int iResult = CBaseAMWeapon::DefaultReload( GetMaxClipAmmo(), ReloadAnimation(), ReloadTime() );
	if ( iResult )
		SetBody( 1 );
}

void CRPG::WeaponIdle( void )
{
	CBaseAMWeapon::WeaponIdle();
	SetBody( (m_iClip == 0) ? 0 : 1 );
}
