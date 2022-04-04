//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#if !defined ( EV_HLDMH )
#define EV_HLDMH

// bullet types
typedef	enum
{
	BULLET_NONE = 0,
	// Melee
	BULLET_PLAYER_KNIFE,
	BULLET_PLAYER_FISTS,
	// Pistols
	BULLET_PLAYER_DEAGLE,
	BULLET_PLAYER_1911,
	BULLET_PLAYER_BERETTA,
	BULLET_PLAYER_GLOCK,
	BULLET_PLAYER_ANACONDA,
	// SMGs
	BULLET_PLAYER_MP5,
	BULLET_PLAYER_P90,
	BULLET_PLAYER_BIZON,
	// Rifles
	BULLET_PLAYER_FAMAS,
	BULLET_PLAYER_M4A1,
	BULLET_PLAYER_AK47,
	BULLET_PLAYER_M249,
	// Shotguns
	BULLET_PLAYER_SPAS12,
	BULLET_PLAYER_LESSLETHAL,
	BULLET_PLAYER_USAS,
	// Snipers
	BULLET_PLAYER_M14,
	BULLET_PLAYER_L96A1,
	// Monsters bullets
	BULLET_MONSTER_9MM,
	BULLET_MONSTER_MP5,
	BULLET_MONSTER_12MM,
} Bullet;

void EV_HLDM_GunshotDecalTrace( pmtrace_t *pTrace, char *decalName );
void EV_HLDM_DecalGunshot( pmtrace_t *pTrace, int iBulletType );
int EV_HLDM_CheckTracer( int idx, float *vecSrc, float *end, float *forward, float *right, int iBulletType, int iTracerFreq, int *tracerCount );
void EV_HLDM_FireBullets( int idx, float *forward, float *right, float *up, int cShots, float *vecSrc, float *vecDirShooting, float flDistance, int iBulletType, int iTracerFreq, int *tracerCount, float flSpreadX, float flSpreadY );

#endif // EV_HLDMH