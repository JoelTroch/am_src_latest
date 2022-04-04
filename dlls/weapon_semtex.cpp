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
// Semtex explosive charge
//

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "gamerules.h"

class CSemtexCharge : public CGrenade
{
	void Spawn( void );
	void Precache( void );
	void BounceSound( void );
	void EXPORT SemtexSlide( CBaseEntity *pOther );
	void EXPORT SemtexThink( void );
public:
	void Deactivate( void );
};
LINK_ENTITY_TO_CLASS( monster_semtex, CSemtexCharge );
LINK_ENTITY_TO_CLASS( monster_satchel, CSemtexCharge ); // Half-Life backward compatibilty

//=========================================================
// Deactivate - do whatever it is we do to an orphaned
// semtex when we don't want it in the world anymore.
//=========================================================
void CSemtexCharge::Deactivate( void )
{
	pev->solid = SOLID_NOT;
	UTIL_Remove( this );
}

void CSemtexCharge::Spawn( void )
{
	Precache();
	pev->movetype = MOVETYPE_BOUNCE;
	pev->solid = SOLID_BBOX;
	SET_MODEL( ENT( pev ), "models/weapons/semtex/w_semtex.mdl" );
	UTIL_SetSize( pev, Vector( -4, -4, -4 ), Vector( 4, 4, 4 ) );
	UTIL_SetOrigin( pev, pev->origin );
	SetTouch( &CSemtexCharge::SemtexSlide );
	SetUse( &CSemtexCharge::DetonateUse );
	SetThink( &CSemtexCharge::SemtexThink );
	pev->nextthink = gpGlobals->time + 0.1;
	pev->gravity = 0.5;
	pev->friction = 0.8;
	pev->dmg = gSkillData.plrDmgSemtex;
	pev->sequence = 1;
}

void CSemtexCharge::SemtexSlide( CBaseEntity *pOther )
{
	entvars_t *pevOther = pOther->pev;

	// Don't hit the guy that launched this grenade
	if ( pOther->edict() == pev->owner )
		return;

	pev->gravity = 1; // Normal gravity now
	// HACKHACK - On ground isn't always set, so look for ground underneath
	TraceResult tr;
	UTIL_TraceLine( pev->origin, pev->origin - Vector( 0, 0, 10), ignore_monsters, edict(), &tr );
	if ( tr.flFraction < 1.0 )
	{
		// Add a bit of static friction
		pev->velocity = pev->velocity * 0.95;
		pev->avelocity = pev->avelocity * 0.9;
		// Play sliding sound, volume based on velocity
	}
	if ( !( pev->flags & FL_ONGROUND ) && pev->velocity.Length2D() > 10 )
		BounceSound();

	StudioFrameAdvance();
}

void CSemtexCharge::SemtexThink( void )
{
	StudioFrameAdvance();
	pev->nextthink = gpGlobals->time + 0.1;
	if ( !IsInWorld() )
	{
		UTIL_Remove( this );
		return;
	}
	if (pev->waterlevel == 3)
	{
		pev->movetype = MOVETYPE_FLY;
		pev->velocity = pev->velocity * 0.8;
		pev->avelocity = pev->avelocity * 0.9;
		pev->velocity.z += 8;
	}
	else if (pev->waterlevel == 0)
		pev->movetype = MOVETYPE_BOUNCE;
	else
		pev->velocity.z -= 8;

}

void CSemtexCharge::Precache( void )
{
	PRECACHE_MODEL( "models/weapons/semtex/w_semtex.mdl" );
	PRECACHE_SOUND( "weapons/g_bounce1.wav" );
	PRECACHE_SOUND( "weapons/g_bounce2.wav" );
	PRECACHE_SOUND( "weapons/g_bounce3.wav" );
}

void CSemtexCharge::BounceSound( void )
{
	switch ( RANDOM_LONG( 0, 2 ) )
	{
	case 0: EMIT_SOUND_FMOD_3D( ENT( pev ), "weapons/g_bounce1.wav", 2048.0f, -1.0, this->entindex() ); break;
	case 1: EMIT_SOUND_FMOD_3D( ENT( pev ), "weapons/g_bounce2.wav", 2048.0f, -1.0, this->entindex() ); break;
	case 2: EMIT_SOUND_FMOD_3D( ENT( pev ), "weapons/g_bounce3.wav", 2048.0f, -1.0, this->entindex() ); break;
	}
}

//=========================================================
// DeactivateSatchels - removes all semtexs owned by
// the provided player. Should only be used upon death.
//
// Made this global on purpose.
//=========================================================

// Shepard : In order to preserve maximum compatibility with standard Half-Life maps, I added a "secondary check" for "monster_satchel"

void DeactivateSatchels( CBasePlayer *pOwner, bool bCheckOldEntityName )
{
	char *cClassname = "monster_semtex";
	if ( bCheckOldEntityName )
		cClassname = "monster_satchel";

	edict_t *pFind;
	pFind = FIND_ENTITY_BY_CLASSNAME( NULL, cClassname );
	while ( !FNullEnt( pFind ) )
	{
		CBaseEntity *pEnt = CBaseEntity::Instance( pFind );
		CSemtexCharge *pSemtex = (CSemtexCharge *)pEnt;
		if ( pSemtex )
		{
			if ( pSemtex->pev->owner == pOwner->edict() )
				pSemtex->Deactivate();
		}
		pFind = FIND_ENTITY_BY_CLASSNAME( pFind, cClassname );
	}
	// Shepard : Make a secondary check for "monster_satchel", just to be sure
	if ( !bCheckOldEntityName )
		DeactivateSatchels( pOwner, true );
}

//==================================================
//==================================================
//==================================================

BOOL CSemtex::CanDeploy( void )
{
	if ( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0 )
		return TRUE;
	else if ( m_chargeReady != 0 )
		return TRUE;
	else
		return FALSE;
}

BOOL CSemtex::Deploy( void )
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.0;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
	if ( m_chargeReady != 0 )
		return CBasePlayerWeapon::DefaultDeploy( "models/weapons/pda/v_pda.mdl", GetPModel(), (DeployAnimation() - 1), "hive" );
	else
		return CBasePlayerWeapon::DefaultDeploy( GetVModel(), GetPModel(), DeployAnimation(), GetExtAnim() );
}

void CSemtex::Precache( void )
{
	PRECACHE_MODEL( GetPModel() );
	PRECACHE_MODEL( GetVModel() );
	PRECACHE_MODEL( "models/weapons/pda/v_pda.mdl" );
	PRECACHE_MODEL( GetWModel() );
	PRECACHE_SOUND( GetDeploySound() );
	UTIL_PrecacheOther( "monster_semtex" );
}

void CSemtex::PrimaryAttack( void )
{
	if ( m_chargeReady == 0 )
	{
		if ( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] )
		{
			bool bSemtexHasBeenUsed = false;
			CBaseEntity *pSemtexZone = NULL;
			while ( (pSemtexZone = UTIL_FindEntityInSphere( pSemtexZone, m_pPlayer->pev->origin, 1.0 )) != NULL )
			{
				if ( (FClassnameIs( pSemtexZone->pev, "func_semtex_zone" )) && (pSemtexZone->pev->speed == 1.0) )
				{
					Vector vecSrc = m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_forward * 16;
					Vector vecThrow = gpGlobals->v_forward + m_pPlayer->pev->velocity;

					CBaseEntity *pSemtex = Create( "monster_semtex", vecSrc, Vector( 0, 0, 0 ), m_pPlayer->edict() );
					pSemtex->pev->velocity = vecThrow;
					pSemtex->pev->avelocity.y = 10;
					m_pPlayer->pev->viewmodel = MAKE_STRING( "models/weapons/pda/v_pda.mdl" );
					SendWeaponAnim( (DeployAnimation() - 1), UseDecrement(), 0 );
					m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
					m_chargeReady = 1;
#ifdef DEBUG
					if ( CVAR_GET_FLOAT( "am_wp_infinite_ammo" ) != 1 )
#endif
						m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;

					m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + AttackDelay();
					bSemtexHasBeenUsed = true;
					break;
				}
			}
			if ( !bSemtexHasBeenUsed )
				ClientPrint( m_pPlayer->pev, HUD_PRINTCENTER, "#SemtexCanOnlyUsedInSemtexZone" );
		}
	}
	else
	{
		SendWeaponAnim( FireAnimation(), UseDecrement(), 0 );
		edict_t *pPlayer = m_pPlayer->edict();
		CBaseEntity *pSemtex = NULL;
		while ( (pSemtex = UTIL_FindEntityInSphere( pSemtex, m_pPlayer->pev->origin, 4096 )) != NULL )
		{
			if ( FClassnameIs( pSemtex->pev, "monster_semtex" ) || FClassnameIs( pSemtex->pev, "monster_satchel" ) )
			{
				if ( pSemtex->pev->owner == pPlayer )
					pSemtex->Use( m_pPlayer, m_pPlayer, USE_ON, 0 );
			}
		}
		m_chargeReady = 2;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + AttackDelay();
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + AttackDelay();
	}
}

void CSemtex::WeaponIdle( void )
{
	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	if ( m_chargeReady == 2 )
	{
		if ( !m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] )
		{
			m_chargeReady = 0;
			RetireWeapon();
			return;
		}
		m_pPlayer->pev->viewmodel = MAKE_STRING( GetVModel() );
		m_pPlayer->pev->weaponmodel = MAKE_STRING( GetPModel() );
		SendWeaponAnim( DeployAnimation(), UseDecrement(), 0 );
		strcpy( m_pPlayer->m_szAnimExtention, "trip" );
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + (AttackDelay() / 2);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + AttackDelay();
		m_chargeReady = 0;
	}
	else
	{
		if ( m_chargeReady == 1 )
			strcpy( m_pPlayer->m_szAnimExtention, "hive" );
		else
			strcpy( m_pPlayer->m_szAnimExtention, GetExtAnim() );

		SendWeaponAnim( IdleAnimation() + 1, UseDecrement(), 0 );
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
	}
}

void CSemtex::Holster( int skiplocal )
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	if ( !m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] && !m_chargeReady )
	{
		m_pPlayer->pev->weapons &= ~(1 << WEAPON_SEMTEX);
		SetThink( &CSemtex::DestroyItem );
		pev->nextthink = gpGlobals->time + 0.1;
	}
}
