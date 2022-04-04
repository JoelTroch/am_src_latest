#ifndef __ZM_H__
#define __ZM_H__

class CZombieMod : public CHalfLifeTeamplay
{
public:
	CZombieMod();

	virtual BOOL ClientCommand(CBasePlayer *pPlayer, const char *pcmd);

	virtual BOOL FPlayerCanRespawn(CBasePlayer *pPlayer);
	virtual float FlPlayerSpawnTime(CBasePlayer *pPlayer);

	virtual int IPointsForKill(CBasePlayer *pAttacker, CBasePlayer *pKilled);
	virtual void InitHUD(CBasePlayer *pl);
	virtual const char *GetGameDescription(void) { return "Zombie Contagion"; }  // this is the game name that gets seen in the server browser
	virtual void UpdateGameMode(CBasePlayer *pPlayer);  // the client needs to be informed of the current game mode
	virtual void PlayerKilled(CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor);
	virtual void Think(void);
	virtual int CountZombies(bool aliveOnly = false);
	virtual int CountHumans(bool aliveOnly = false);
	virtual void AutoBalance();
	virtual CBasePlayer *GetFirstActualPlayer();
	virtual edict_t *GetPlayerSpawnSpot(CBasePlayer *pPlayer);
	virtual void ForceAutobalance();
	virtual BOOL CanHavePlayerItem(CBasePlayer *pPlayer, CBasePlayerItem *pItem);
	virtual int AmmoShouldRespawn(CBasePlayerAmmo *pAmmo);
	virtual void RespawnAllAmmo();
	virtual void RespawnAllWeapons();
	virtual void CheckRoundEnd();
	virtual void RoundEndFull();
	virtual void RoundEndDelay();
	virtual void RoundStart();
	virtual void TeamWin(int team);
	virtual void PlayerSpawn(CBasePlayer *pl);

	float roundFullEnd;
	float nextAutoBalance;
	int zombieLifes;
	int human_scores;
	int teamscore[2];
};

#endif