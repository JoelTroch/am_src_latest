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
#include "gamerules.h"

#define	ZOMBIECLAW_BODYHIT_VOLUME 128
#define	ZOMBIECLAW_WALLHIT_VOLUME 512

LINK_ENTITY_TO_CLASS(weapon_zombieclaw, CZombieClaw);

#define ZOMBIECLAW_DAMAGE 70
#define ZOMBIECLAW_PUNCH 40
#define ZOMBIECLAW_PUNCHDAMAGE 10

enum zombieclaw_e {
	CROWBAR_IDLE = 0,
	CROWBAR_SLASHMISS1,
	CROWBAR_SLASHMISS2,
	CROWBAR_DEPLOY,
	CROWBAR_STABMISS,
	CROWBAR_STAB,
	CROWBAR_SLASH1,
	CROWBAR_SLASH2
};

void CZombieClaw::Spawn()
{
	Precache();
	m_iId = WEAPON_ZOMBIECLAW;
	m_iClip = -1;

	FallInit();// get ready to fall down.
}


void CZombieClaw::Precache(void)
{
	PRECACHE_MODEL("models/v_zombieclaws.mdl");
	PRECACHE_SOUND("weapons/zombieclaw/miss_slash_01.wav");
	PRECACHE_SOUND("weapons/zombieclaw/miss_slash_02.wav");
	PRECACHE_SOUND("weapons/zombieclaw/miss_wall_01.wav");
	PRECACHE_SOUND("weapons/zombieclaw/miss_wall_02.wav");
	PRECACHE_SOUND("weapons/zombieclaw/miss_wall_03.wav");
}

int CZombieClaw::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = NULL;
	p->iMaxAmmo1 = -1;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 0;
	p->iPosition = 0;
	p->iId = WEAPON_ZOMBIECLAW;
	p->iWeight = KNIFE_WEIGHT;
	return 1;
}

BOOL CZombieClaw::Deploy()
{
	return DefaultDeploy("models/v_zombieclaws.mdl", "models/p_crowbar.mdl", CROWBAR_DEPLOY, "crowbar");
}

void FindHullIntersection(const Vector &vecSrc, TraceResult &tr, float *mins, float *maxs, edict_t *pEntity);
void CZombieClaw::PrimaryAttack()
{
	if (!Swing(1))
	{
		SetThink(&CZombieClaw::SwingAgain);
		pev->nextthink = gglobals->time + 0.1;
	}
}

void CZombieClaw::SecondaryAttack()
{
	//secondaryattack does punch.
	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecEnd = vecSrc + gglobals->v_forward * 64;
	TraceResult tr;

	UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

	if (tr.flFraction >= 1.0)
	{
		UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT(m_pPlayer->pev), &tr);
		if (tr.flFraction < 1.0)
		{
			// Calculate the point of intersection of the line (or hull) and the object we hit
			// This is and approximation of the "best" intersection
			CBaseEntity *pHit = CBaseEntity::Instance(tr.pHit);
			if (!pHit || pHit->IsBSPModel())
				FindHullIntersection(vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, m_pPlayer->edict());
			vecEnd = tr.vecEndPos;	// This is the point on the actual surface (the hull could have hit space)
		}
	}

	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
	if (tr.flFraction >= 1.0)
	{
		// miss
		m_flNextSecondaryAttack = GetNextAttackDelay(0.5);
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);

		//TODO: play sound.
		switch (RANDOM_LONG(0, 2))
		{
		case 0:
			SendWeaponAnim(CROWBAR_SLASHMISS1); 
			EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/zombieclaws/miss_slash_01.wav", 1.0, ATTN_NORM); 
			break;
		case 1:
			SendWeaponAnim(CROWBAR_SLASHMISS2); 
			EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/zombieclaws/miss_slash_02.wav", 1.0, ATTN_NORM); 
			break;
		}

		return;
	}
	else
	{
		switch (RANDOM_LONG(0, 1))
		{
		case 0:
			SendWeaponAnim(CROWBAR_SLASH1); break;
		case 1:
			SendWeaponAnim(CROWBAR_SLASH2); break;
		}

		CBaseEntity *pEntity = CBaseEntity::Instance(tr.pHit);
		ClearMultiDamage();
		pEntity->TraceAttack(m_pPlayer->pev, ZOMBIECLAW_PUNCHDAMAGE, gglobals->v_forward, &tr, DMG_CLUB);
		ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

		if (pEntity->Classify() == CLASS_PLAYER)
		{
			//TODO: play sound.
			switch (RANDOM_LONG(0, 2))
			{
				case 0: EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/zombieclaws/miss_wall_01.wav", 1.0, ATTN_NORM); break;
				case 1: EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/zombieclaws/miss_wall_02.wav", 1.0, ATTN_NORM); break;
				case 2: EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/zombieclaws/miss_wall_03.wav", 1.0, ATTN_NORM); break;
			}
		}

		m_flNextSecondaryAttack = GetNextAttackDelay(0.25);
	}

}

void CZombieClaw::Smack()
{
	DecalGunshot(&m_trHit, BULLET_PLAYER_KNIFE);
}


void CZombieClaw::SwingAgain(void)
{
	Swing(0);
}


int CZombieClaw::Swing(int fFirst)
{
	int fDidHit = FALSE;

	TraceResult tr;

	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecEnd = vecSrc + gglobals->v_forward * 32;

	UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

	if (tr.flFraction >= 1.0)
	{
		UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT(m_pPlayer->pev), &tr);
		if (tr.flFraction < 1.0)
		{
			// Calculate the point of intersection of the line (or hull) and the object we hit
			// This is and approximation of the "best" intersection
			CBaseEntity *pHit = CBaseEntity::Instance(tr.pHit);
			if (!pHit || pHit->IsBSPModel())
				FindHullIntersection(vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, m_pPlayer->edict());
			vecEnd = tr.vecEndPos;	// This is the point on the actual surface (the hull could have hit space)
		}
	}

	if (tr.flFraction >= 1.0)
	{
		if (fFirst)
		{
			// miss
			m_flNextPrimaryAttack = GetNextAttackDelay(0.5);

			// player "shoot" animation
			m_pPlayer->SetAnimation(PLAYER_ATTACK1);
		}
	}
	else
	{
		switch (RANDOM_LONG(0, 1))
		{
		case 0:
			SendWeaponAnim(CROWBAR_SLASH1); break;
		case 1:
			SendWeaponAnim(CROWBAR_SLASH2); break;
		}

		// player "shoot" animation
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);

		// hit
		fDidHit = TRUE;
		CBaseEntity *pEntity = CBaseEntity::Instance(tr.pHit);

		ClearMultiDamage();
		pEntity->TraceAttack(m_pPlayer->pev, ZOMBIECLAW_DAMAGE, gglobals->v_forward, &tr, DMG_CLUB);

		ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

		// play thwack, smack, or dong sound
		float flVol = 1.0;
		int fHitWorld = TRUE;

		if (pEntity)
		{
			if (pEntity->Classify() != CLASS_NONE && pEntity->Classify() != CLASS_MACHINE)
			{
				//TODO: play sound.
				switch (RANDOM_LONG(0, 2))
				{
				case 0: EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/zombieclaws/miss_wall_01.wav", 1.0, ATTN_NORM); break;
				case 1: EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/zombieclaws/miss_wall_02.wav", 1.0, ATTN_NORM); break;
				case 2: EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/zombieclaws/miss_wall_03.wav", 1.0, ATTN_NORM); break;
				}
				m_pPlayer->m_iWeaponVolume = ZOMBIECLAW_BODYHIT_VOLUME;
				if (!pEntity->IsAlive())
					return TRUE;
				else
					flVol = 0.1;

				fHitWorld = FALSE;
			}
		}

		// play texture hit sound
		// UNDONE: Calculate the correct point of intersection when we hit with the hull instead of the line

		if (fHitWorld)
		{
			float fvolbar = TEXTURETYPE_PlaySound(&tr, vecSrc, vecSrc + (vecEnd - vecSrc) * 2, BULLET_PLAYER_KNIFE);

			if (g_pGameRules->IsMultiplayer())
			{
				// override the volume here, cause we don't play texture sounds in multiplayer, 
				// and fvolbar is going to be 0 from the above call.

				fvolbar = 1;
			}
			/*
			// also play crowbar strike
			switch (RANDOM_LONG(0, 1))
			{
			case 0:
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/cbar_hit1.wav", fvolbar, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
				break;
			case 1:
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/cbar_hit2.wav", fvolbar, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
				break;
			}
			*/
			// delay the decal a bit
			m_trHit = tr;
		}

		m_pPlayer->m_iWeaponVolume = flVol * ZOMBIECLAW_WALLHIT_VOLUME;
		m_flNextPrimaryAttack = GetNextAttackDelay(0.25);

		SetThink(&CZombieClaw::Smack);
		pev->nextthink = UTIL_WeaponTimeBase() + 0.2;


	}
	return fDidHit;
}