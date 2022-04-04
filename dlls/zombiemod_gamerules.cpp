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
// teamplay_gamerules.cpp
//
#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"player.h"
#include	"weapons.h"
#include	"gamerules.h"
#include	"teamplay_gamerules.h"
#include	"zombiemod_gamerules.h"
#include	"game.h"

extern DLL_GLOBAL BOOL		g_fGameOver;

CZombieMod::CZombieMod()
{
	zombieLifes = 1;//at least one
	roundFullEnd = -1;
	nextAutoBalance = gglobals->time;
	teamscore[0] = teamscore[1] = 0;
}

#include "voice_gamemgr.h"
extern CVoiceGameMgr	g_VoiceGameMgr;
extern cvar_t timeleft;

void CZombieMod::Think(void)
{
	///// Check game rules /////
	static int last_time;
	int time_remaining = 0;

	g_VoiceGameMgr.Update(gglobals->frametime);
	CheckRoundEnd();
	RoundEndFull();

	if (g_fGameOver)   // someone else quit the game already
	{
		CHalfLifeMultiplay::Think();
		return;
	}

	float flTimeLimit = CVAR_GET_FLOAT("mp_timelimit") * 60;
	time_remaining = (int)(flTimeLimit ? (flTimeLimit - gglobals->time) : 0);

	if (flTimeLimit != 0 && gglobals->time >= flTimeLimit)
	{
		GoToIntermission();
		return;
	}

	// Updates once per second
	if (timeleft.value != last_time)
	{
		g_engfuncs.pfnCvar_DirectSet(&timeleft, UTIL_VarArgs("%i", time_remaining));
	}

	last_time = time_remaining;
}

void CZombieMod::RoundEndFull()
{
	if (roundFullEnd < 0)
		return;

	RoundStart();
}

void CZombieMod::RoundEndDelay()
{
	roundFullEnd = gglobals->time + 3.0;//3s delay before round restart.
}

void CZombieMod::RoundStart()
{
	RespawnAllAmmo();
	RespawnAllWeapons();
}

void CZombieMod::TeamWin(int team)
{

}

void CZombieMod::CheckRoundEnd()
{
	if (CountZombies(false) > 0 && CountZombies(true) == 0 && zombieLifes <= 0)
	{ 
		TeamWin(1);//humans.
		RoundEndDelay();
		return;
	}
}


//=========================================================
//=========================================================
void CZombieMod::PlayerSpawn(CBasePlayer *pPlayer)
{
	BOOL		addDefault;
	CBaseEntity	*pWeaponEntity = NULL;

	pPlayer->pev->weapons |= (1 << WEAPON_SUIT);
	addDefault = TRUE;

	while (pWeaponEntity = UTIL_FindEntityByClassname(pWeaponEntity, "game_player_equip"))
	{
		pWeaponEntity->Touch(pPlayer);
		addDefault = FALSE;
	}

	if ((pPlayer->pev->team == 1) || (pPlayer->pev->team == 2))
	{
		pPlayer->m_iHideHUD &= ~HIDEHUD_WEAPONS;
		pPlayer->m_iHideHUD &= ~HIDEHUD_FLASHLIGHT;
		pPlayer->m_iHideHUD &= ~HIDEHUD_HEALTH;
		pPlayer->m_afPhysicsFlags &= ~PFLAG_OBSERVER;
	}
	else if (pPlayer->pev->team == 3)
	{
		pPlayer->pev->classname = MAKE_STRING("spectator");
		pPlayer->pev->flags |= FL_NOTARGET;
		pPlayer->pev->solid = SOLID_NOT;
		pPlayer->pev->takedamage = DAMAGE_NO;
		pPlayer->pev->movetype = MOVETYPE_NOCLIP;
		pPlayer->pev->effects |= EF_NODRAW;
		pPlayer->m_iHideHUD |= HIDEHUD_WEAPONS | HIDEHUD_FLASHLIGHT | HIDEHUD_HEALTH;

		addDefault = FALSE;
	}
	else
	{
		// Make player dead to the game
		pPlayer->pev->classname = MAKE_STRING("observer");
		pPlayer->pev->solid = SOLID_NOT;
		pPlayer->pev->takedamage = DAMAGE_NO;
		pPlayer->pev->movetype = MOVETYPE_NONE;
		pPlayer->pev->flags |= FL_NOTARGET;
		pPlayer->pev->effects |= EF_NODRAW;
		pPlayer->m_afPhysicsFlags |= PFLAG_OBSERVER;
		pPlayer->m_iHideHUD |= HIDEHUD_WEAPONS | HIDEHUD_FLASHLIGHT | HIDEHUD_HEALTH;

		addDefault = FALSE;
	}

	if (addDefault && pPlayer->pev->team == 1)
	{
		pPlayer->GiveNamedItem("weapon_knife");
		pPlayer->GiveNamedItem("weapon_fists");
		pPlayer->GiveNamedItem("weapon_1911");
		pPlayer->GiveAmmo(28, "45", _9MM_MAX_CARRY);// 4 full reloads
	}
	else if (pPlayer->pev->team == 2)
	{
		pPlayer->GiveNamedItem("weapon_zombieclaw");
	}
}


extern edict_t *ZMEntSelectSpawnPoint(CBaseEntity *p);
edict_t *CZombieMod::GetPlayerSpawnSpot(CBasePlayer *pPlayer)
{
	edict_t *pentSpawnSpot = ZMEntSelectSpawnPoint(pPlayer);

	pPlayer->pev->origin = VARS(pentSpawnSpot)->origin + Vector(0, 0, 1);
	pPlayer->pev->v_angle = g_vecZero;
	pPlayer->pev->velocity = g_vecZero;
	pPlayer->pev->angles = VARS(pentSpawnSpot)->angles;
	pPlayer->pev->punchangle = g_vecZero;
	pPlayer->pev->fixangle = TRUE;
	return pentSpawnSpot;
}

BOOL CZombieMod::ClientCommand(CBasePlayer *pPlayer, const char *pcmd)
{
	if (g_VoiceGameMgr.ClientCommand(pPlayer, pcmd))
		return TRUE;

	if (CHalfLifeTeamplay::ClientCommand(pPlayer, pcmd))
		return TRUE;

	return FALSE;
}

extern int gmsgGameMode;
extern int gmsgSayText;
extern int gmsgTeamInfo;
extern int gmsgScoreInfo;

void CZombieMod::UpdateGameMode(CBasePlayer *pPlayer)
{
	MESSAGE_BEGIN(MSG_ONE, gmsgGameMode, NULL, pPlayer->edict());
	WRITE_BYTE(2);
	MESSAGE_END();
}

void CZombieMod::InitHUD(CBasePlayer *pPlayer)
{
	CHalfLifeTeamplay::InitHUD(pPlayer);
}

//=========================================================
//=========================================================
void CZombieMod::PlayerKilled(CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor)
{
	if (pVictim->pev->team == 1)
	{
		zombieLifes++;
		ChangePlayerTeam(pVictim, 2, TRUE, FALSE);//human becomes zombie.
	}
	else if (pVictim->pev->team == 2 && zombieLifes > 0)
	{
		zombieLifes--;
	}

	CHalfLifeTeamplay::PlayerKilled(pVictim, pKiller, pInflictor);

	if (FPlayerCanRespawn(pVictim))
		pVictim->pev->deadflag = DEAD_RESPAWNABLE;//force respawn
}

BOOL CZombieMod::FPlayerCanRespawn(CBasePlayer *pPlayer)
{
	if (pPlayer->pev->team == 2 && zombieLifes <= 0)//zombie and can't respawn.
	{
		ChangePlayerTeam(pPlayer, 3, FALSE, FALSE);
		UTIL_ClientPrintAll(HUD_PRINTCONSOLE,"No more zombie lifes, making you spectator.");
		return FALSE;
	}

	return TRUE;
}

//=========================================================
//=========================================================
float CZombieMod::FlPlayerSpawnTime(CBasePlayer *pPlayer)
{
	return gglobals->time;//now!
}

int CZombieMod::IPointsForKill(CBasePlayer *pAttacker, CBasePlayer *pKilled)
{
	if (!pKilled)
		return 0;

	if (!pAttacker)
		return 1;

	if (pAttacker != pKilled && PlayerRelationship(pAttacker, pKilled) == GR_TEAMMATE)
		return -1;

	return 1;
}

int CZombieMod::CountZombies(bool aliveOnly)
{
	int c = 0;
	for (int i = 1; i < gglobals->maxClients+1; i++)
	{
		CBaseEntity *p = UTIL_PlayerByIndex(i);
		if (!p)
			continue;
		if (p->pev->team != 2)
			continue;

		c++;

		if (!p->IsAlive() && aliveOnly)
			c--;
	}

	return c;
}
int CZombieMod::CountHumans(bool aliveOnly)
{
	int c = 0;
	for (int i = 1; i < gglobals->maxClients + 1; i++)
	{
		CBaseEntity *p = UTIL_PlayerByIndex(i);
		if (!p)
			continue;
		if (p->pev->team != 1)
			continue;

		c++;

		if (!p->IsAlive() && aliveOnly)
			c--;
	}

	return c;
}
CBasePlayer *CZombieMod::GetFirstActualPlayer()
{
	for (int i = 1; i < gglobals->maxClients + 1; i++)
	{
		CBaseEntity *e = UTIL_PlayerByIndex(i);
		if (!e)
			continue;
		if (!e->IsPlayer())
			continue;

		if (e->pev->team != 3)
			return (CBasePlayer*)e;
	}

	return (CBasePlayer*)UTIL_PlayerByIndex(1);
}

void CZombieMod::AutoBalance()
{
	int z, h;
	z = CountZombies();
	h = CountHumans();

	if (z <= 0)
	{
		CBasePlayer *p = GetFirstActualPlayer();
		ChangePlayerTeam(p, 2, TRUE, FALSE);
	}
	else if (h <= 0)
	{
		CBasePlayer *p = GetFirstActualPlayer();
		ChangePlayerTeam(p, 1, TRUE, FALSE);
	}

	if (nextAutoBalance > gglobals->time)
		return;

	nextAutoBalance = gglobals->time + CVAR_GET_FLOAT("mp_autobalance");//autobalance in mp_autobalance
}

int CZombieMod::AmmoShouldRespawn(CBasePlayerAmmo *pAmmo)
{
	return GR_AMMO_RESPAWN_NO;
}

//=========================================================
// CanHaveWeapon - returns FALSE if the player is not allowed
// to pick up this weapon
//=========================================================
BOOL CZombieMod::CanHavePlayerItem(CBasePlayer *pPlayer, CBasePlayerItem *pItem)
{
	if (pPlayer->pev->team == 2 && !strcmp("weapon_zombieclaw", STRING(pItem->pev->classname)))//forced true for zombies
		return TRUE;//zombies can have only weapon_zombieclaw

	if (pPlayer->pev->team == 2 || pPlayer->pev->team == 3)//zombie or spectator.
		return FALSE;

	return CHalfLifeMultiplay::CanHavePlayerItem(pPlayer, pItem);
}

void CZombieMod::RespawnAllAmmo()
{
	for (int i = 0; i < gglobals->maxEntities; i++)
	{
		CBaseEntity *e = CBaseEntity::Instance(INDEXENT(i));
		if (!e)
			continue;
		if (!e->IsAmmo())
			continue;

		e->Respawn();
	}
}

void CZombieMod::RespawnAllWeapons()
{
	for (int i = 0; i < gglobals->maxEntities; i++)
	{
		CBaseEntity *e = CBaseEntity::Instance(INDEXENT(i));
		if (!e)
			continue;
		if (!e->IsItem())
			continue;

		e->Respawn();
	}
}

void CZombieMod::ForceAutobalance()
{
	nextAutoBalance = 0;
}