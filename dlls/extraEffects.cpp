#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "customentity.h"
#include "effects.h"
#include "weapons.h"
#include "decals.h"
#include "func_break.h"
#include "shake.h"
#include "player.h"

//=================================================================
// env_waterpuddle: water puddle for simulate raindrop refractions.
//=================================================================

class CEnvWaterPuddle : public CBaseAnimating
{

public:
	void Spawn(void);
	void Precache(void);
	virtual int	ObjectCaps(void) { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

};

LINK_ENTITY_TO_CLASS(env_waterpuddle, CEnvWaterPuddle);

void CEnvWaterPuddle::Spawn(void)
{
	Precache();
	UTIL_SetOrigin(pev, pev->origin);
	pev->solid = SOLID_NOT;
	DROP_TO_FLOOR(ENT(pev));
	pev->origin.z += 0.5;

	SET_MODEL(ENT(pev), STRING(pev->model));
	SetModelCollisionBox();

	Vector min, max;
	min = pev->mins;
	max = pev->maxs;
	min.z -= 15.0;
	max.z += 15.0;
	pev->skin = CONTENTS_LAVA;
	UTIL_SetSize(pev, min, max);
}

void CEnvWaterPuddle::Precache(void)
{
	PRECACHE_MODEL((char*)STRING(pev->model));
}

// Shepard : FMOD stuff
#define SF_REMOVE_ON_FIRE 1
#define SF_LOOPED 2

extern int gmsgFMODStop;
extern int gmsgFMODSwitch;
extern int gmsgFMODMusicAction;
extern int gmsgFMODMusicStealth;

// pev->body = Are we playing a music ?
// pev->skin = Entity's behavior
class CFMODStream : public CPointEntity
{
public:
	void Spawn( void );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
};
LINK_ENTITY_TO_CLASS( ambient_fmodstream, CFMODStream );
LINK_ENTITY_TO_CLASS( trigger_mp3audio, CFMODStream );

void CFMODStream::Spawn( void )
{
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	pev->body = 0;
}

void CFMODStream::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	CBasePlayer *pPlayer = (CBasePlayer *)UTIL_PlayerByIndex( 1 );
	if ( pev->skin != 2 && pev->body == 0 ) // Play specific music
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgFMODMusicStealth, NULL, ENT( pPlayer->pev ) );
			WRITE_STRING( STRING( pev->message ) );
			WRITE_BYTE( FBitSet( pev->spawnflags, SF_LOOPED ) ? 1 : 0 );
		MESSAGE_END();
		MESSAGE_BEGIN( MSG_ONE, gmsgFMODSwitch, NULL, ENT( pPlayer->pev ) );
			WRITE_BYTE( 0 );
		MESSAGE_END();
		pev->body = 1;
		pPlayer->m_bMusicIsAllowedToSwitchToAction = FALSE;
	}
	else if (pev->skin == 0 && pev->body == 1) // Restore map music
	{
		char cStream[64];
		CGameMusic *pEntity = (CGameMusic *)UTIL_FindEntityByClassname(NULL, "game_music");

		if (pEntity)
		{
			if (FStringNull(pEntity->m_szMusicStealth) && FStringNull(pEntity->m_szMusicAction))
			{
				//do nothing!
			}
			else
			{
				if (pEntity)
					sprintf(cStream, "%s", (char *)STRING(pEntity->m_szMusicStealth));
				else
					sprintf(cStream, "%s.ogg", (char *)STRING(gpGlobals->mapname));


				MESSAGE_BEGIN(MSG_ONE, gmsgFMODMusicStealth, NULL, ENT(pPlayer->pev));
				WRITE_STRING(cStream);
				WRITE_BYTE(1);
				MESSAGE_END();
				MESSAGE_BEGIN(MSG_ONE, gmsgFMODSwitch, NULL, ENT(pPlayer->pev));
				WRITE_BYTE((pPlayer->m_bMusicIsAction) ? 1 : 0);
				MESSAGE_END();
				pev->body = 0;
				pPlayer->m_bMusicIsAllowedToSwitchToAction = TRUE;
			}
		}
	}
	else if ( pev->skin == 1 && pev->body == 1 ) // Stop the music
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgFMODStop, NULL, ENT( pPlayer->pev ) );
			WRITE_BYTE( 0 );
		MESSAGE_END();
		pev->body = 0;
		pPlayer->m_bMusicIsAllowedToSwitchToAction = TRUE;
	}
	else if ( pev->skin == 2 ) // Fred's voice
		EMIT_SOUND_FMOD_2D_ONE( ENT( pPlayer->pev ), STRING( pev->message ), 1, 1);

	if ( FBitSet( pev->spawnflags, SF_REMOVE_ON_FIRE ) )
		UTIL_Remove( this );
}

//==================================================
//==================================================
//==================================================

extern int gmsgFMODReverb;

// pev->body = Reverberation type
class CFMODReverb : public CPointEntity
{
public:
	void Spawn( void );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
};
LINK_ENTITY_TO_CLASS( ambient_fmodreverb, CFMODReverb );

void CFMODReverb::Spawn( void )
{
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
}

void CFMODReverb::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( !pActivator->IsPlayer() )
		return;

	if ( pev->spawnflags & SF_REMOVE_ON_FIRE ) // It's not really "Remove on fire", it's "All players", saves a define ^^
		MESSAGE_BEGIN( MSG_ALL, gmsgFMODReverb );
	else
		MESSAGE_BEGIN( MSG_ONE, gmsgFMODReverb, NULL, ENT( pActivator->pev ) );

	WRITE_BYTE( pev->body );
	MESSAGE_END();
	CBasePlayer *pPlayer = (CBasePlayer *)pActivator;
	if ( pPlayer )
		pPlayer->m_iLastReverberationType = pev->body;
}

//==================================================
//==================================================
//==================================================

TYPEDESCRIPTION CGameMusic::m_SaveData[] =
{
	DEFINE_FIELD( CGameMusic, m_szMusicAction, FIELD_STRING ),
	DEFINE_FIELD( CGameMusic, m_szMusicStealth, FIELD_STRING )
};
IMPLEMENT_SAVERESTORE( CGameMusic, CPointEntity );
LINK_ENTITY_TO_CLASS( game_music, CGameMusic );

void CGameMusic::Spawn()
{
	pev->movetype = MOVETYPE_NONE;
	pev->solid = SOLID_NOT;
}

void CGameMusic::KeyValue( KeyValueData* pkvd )
{
	if ( FStrEq( pkvd->szKeyName, "m_szMusicAction" ) )
	{
		m_szMusicAction = ALLOC_STRING( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "m_szMusicStealth" ) )
	{
		m_szMusicStealth = ALLOC_STRING( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue( pkvd );
}


//=================================================================
// env_colorcorrection - color correction for screenspace.
//=================================================================

class CEnvColorCorrection : public CPointEntity
{

public:
	virtual void Spawn(void);
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);

	virtual int		Save(CSave &save);
	virtual int		Restore(CRestore &restore);
	virtual int		ObjectCaps(void) { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	static	TYPEDESCRIPTION m_SaveData[];

	virtual void KeyValue(KeyValueData *pkvd);
	int r, g, b, s;
	float f;
};

void CEnvColorCorrection::Use(CBaseEntity *pAct, CBaseEntity *pCall, USE_TYPE useType, float value){
	if (pAct && pAct->IsPlayer()){
		CBasePlayer *plr = (CBasePlayer*)pAct;
		plr->setColorCorrection(r, g, b, s, f, true);
		return;
	}

	if (pCall && pCall->IsPlayer()){
		CBasePlayer *plr = (CBasePlayer*)pAct;
		plr->setColorCorrection(r, g, b, s, f, true);
		return;
	}
}

TYPEDESCRIPTION CEnvColorCorrection::m_SaveData[] =
{
	DEFINE_FIELD(CEnvColorCorrection, r, FIELD_INTEGER),
	DEFINE_FIELD(CEnvColorCorrection, g, FIELD_INTEGER),
	DEFINE_FIELD(CEnvColorCorrection, b, FIELD_INTEGER),
	DEFINE_FIELD(CEnvColorCorrection, s, FIELD_INTEGER),
	DEFINE_FIELD(CEnvColorCorrection, f, FIELD_FLOAT),
};
IMPLEMENT_SAVERESTORE(CEnvColorCorrection, CPointEntity);
LINK_ENTITY_TO_CLASS(env_colorcorrection, CEnvColorCorrection);

void CEnvColorCorrection::Spawn(void)
{
	UTIL_SetOrigin(pev, pev->origin);
	pev->solid = SOLID_NOT;

}

void CEnvColorCorrection::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "r"))
	{
		r = atof(pkvd->szValue)*100.0;
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "g"))
	{
		g = atof(pkvd->szValue)*100.0;
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "b"))
	{
		b = atof(pkvd->szValue)*100.0;
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "s"))
	{
		s = atof(pkvd->szValue)*100.0;
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "fadetime"))
	{
		f = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue(pkvd);
}