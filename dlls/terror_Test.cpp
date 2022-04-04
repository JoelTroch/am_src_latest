#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"schedule.h"
#include	"squadmonster.h"
#include	"CBaseHuman.h"
#include	"defaultai.h"
#include	"scripted.h"
#include	"soundent.h"
#include	"animation.h"
#include	"weapons.h"
#include	"decals.h"

#include	"weapons.h"//m_iDefaultAmmo
extern int gmsgParticles;//define external message

class CTerrorist : public CBaseHuman
{
public:
	void Spawn( );
	void Precache();
	int Classify( void );
	void KeyValue( KeyValueData *pkvd );

	float GetDuration( const char *pszSentence );

	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	int			m_iszSpeakAs;					// Change the prefix for all this monster's speeches LRC

private:
	const char* str1;
	char* str2;
	const char* result;
};

LINK_ENTITY_TO_CLASS( monster_terrorist, CTerrorist );
LINK_ENTITY_TO_CLASS( monster_camera, CTerrorist );
//monster_terrorist_female doesn't speaks, it a woman (a woman who doesn't speaks? who would have thought so?!)
LINK_ENTITY_TO_CLASS( monster_terrorist_female, CTerrorist );
LINK_ENTITY_TO_CLASS( monster_human_ak, CTerrorist );
//LINK_ENTITY_TO_CLASS( monster_human_grunt, CTerrorist );

void CTerrorist::KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "SpeakAs"))
	{
		m_iszSpeakAs = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_fHandGrenades"))
	{
		m_fHandGrenades = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iNumGrenades"))
	{
		m_fHandGrenades = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "iDistToShoot"))
	{
		iDistToShoot = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_bCanDropPrimWeapon"))
	{
		m_bCanDropPrimWeapon = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iBreakPoint"))
	{
		m_iBreakPoint = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_bUseArmor"))
	{
		m_bUseArmor = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "IdleAnim"))
	{
		m_szIdleAnimation = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else 
		CBaseHuman::KeyValue( pkvd );
}

TYPEDESCRIPTION	CTerrorist::m_SaveData[] = 
{
	DEFINE_FIELD( CTerrorist, m_iszSpeakAs, FIELD_STRING ), //LRC
};

IMPLEMENT_SAVERESTORE( CTerrorist, CBaseHuman );

//=========================================================
// Precache
//=========================================================

void CTerrorist :: Precache()
{
	if (pev->model)
	{
	/*	str1 = STRING(pev->model);
		str2 = "MODELS";

		result = strstr( str1, str2 );
		ALERT ( at_console, "PRECACHE_MODEL: %s\n", str1 );

		if( result == NULL )//this should never happen
		{
			ALERT ( at_console, "Could not find '%s' in '%s'\n", str2, str1 );

			//damn case sensitive...
			//try to match lower letters
			str2 = "models";
			result = strstr( str1, str2 );
			ALERT ( at_console, "PRECACHE_MODEL (second attempt): %s\n", str1 );

			if( result == NULL )//this should never never never happen
			ALERT ( at_console, "Could not find (second attempt) '%s' in '%s'\n", str2, str1 );
			else 
			ALERT ( at_console, "Found a substring: '%s'\n", result );
		}
		else 
		ALERT ( at_console, "Found a substring: '%s'\n", result );

		ALERT ( at_console, "PRECACHE_MODEL: changed to '%s'\n", result );

		PRECACHE_MODEL( (char *)result );*/

	PRECACHE_MODEL( (char*)STRING(pev->model) );
	}
	else
	PRECACHE_MODEL("models/terrorist.mdl");

//	PRECACHE_MODEL("models/terrorist_zo.mdl");

	/*
	PRECACHE_SOUND("gook/step1.wav");
	PRECACHE_SOUND("gook/step2.wav");
	PRECACHE_SOUND("gook/step3.wav");
	PRECACHE_SOUND("gook/step4.wav");
*/
	m_szFriends[0] = "monster_terrorist";
	m_szFriends[1] = "monster_terrorist_repel";
	m_szFriends[2] = "monster_terrorist_female";

	m_nNumFriendTypes = 3;
	
	UTIL_PrecacheOther( "monster_zombie" );

	if ( FClassnameIs( pev, "monster_terrorist_female" ) )
	strcpy( m_szSpeechLabel, "FM_");//no more than 3 chars!!!!
	else if (m_iszSpeakAs)
	strcpy( m_szSpeechLabel, STRING(m_iszSpeakAs));
	else
	strcpy( m_szSpeechLabel, "T_");//TB_

	CBaseHuman::Precache();
}    


//=========================================================
// Spawn
//=========================================================

void CTerrorist::Spawn()
{

    Precache( );

	if (pev->model)		
	{
	//	SET_MODEL( ENT(pev), result );

		SET_MODEL(ENT(pev), STRING(pev->model)); //LRC
	}
	else
		SET_MODEL(ENT(pev), "models/terrorist.mdl");

	if (pev->health == 0) //LRC
	pev->health			= 100;//100

//	m_flFieldOfView		= 0.2;

	bTerrorist			= TRUE;
	m_afCapability		= bits_CAP_SQUAD | bits_CAP_HEAR | bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP/* | bits_CAP_STRAFE*/;



	CBaseHuman::Spawn();

	SetBodygroup(0, 0);
	SetBodygroup(1, RANDOM_LONG(0, 4));
	SetBodygroup(0, 0);
	SetBodygroup(0, 0);
}

//=========================================================
// Classify - indicates this monster's place in the 
// relationship table.
//=========================================================

int	CTerrorist :: Classify ( void )
{
	if(m_bSurrender)
	return CLASS_NONE;
	else
	return m_iClass?m_iClass:CLASS_FACTION_C; //Terroristas odian a todos TODOS !
}


//=========================================================
// GetDuration - Lengths of sentences
//=========================================================

float CTerrorist :: GetDuration( const char *pszSentence )
{
	if ( !strcmp( pszSentence, "TAUNT" ) ) return 4;
	if ( !strcmp( pszSentence, "DEAD" ) ) return 7;
	if ( !strcmp( pszSentence, "HELP" ) ) return 5;
	if ( !strcmp( pszSentence, "MORTAL" ) ) return 6;
	if ( !strcmp( pszSentence, "MEDIC" ) ) return 4;
	if ( !strcmp( pszSentence, "HURTA" ) ) return 5;

	return CBaseHuman::GetDuration( pszSentence );
}
/*
//=========================================================
// BARNEY REPEL
//=========================================================

class CTerroristRepel : public CBaseHumanRepel
{
public:
	virtual char * EntityName() { return "monster_terrorist"; };
};

LINK_ENTITY_TO_CLASS( monster_terrorist_repel, CTerroristRepel );*/