#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"schedule.h"
#include	"squadmonster.h"
#include	"cbasehuman.h"
#include	"cbasehumanfollower.h"
#include	"defaultai.h"
#include	"scripted.h"
#include	"soundent.h"
#include	"animation.h"
#include	"weapons.h"
#include	"decals.h"



#define NUM_HEADS 1

//=====================
// Animation Events
//=====================

class CAgencyMember : public CBaseHumanFollower
{
public:
	void Spawn( );
	void Precache();
	int Classify( void ) { return CLASS_PLAYER_ALLY; };

	void GibMonster();

	void PickUpGun( int gun );

	float GetDuration( const char *pszSentence );

	CUSTOM_SCHEDULES;
};

LINK_ENTITY_TO_CLASS( monster_agency_member, CAgencyMember );

//=========================================================
// Precache
//=========================================================

void CAgencyMember :: Precache()
{
	if (pev->model)
	PRECACHE_MODEL( (char*)STRING(pev->model) );
	else
	PRECACHE_MODEL("models/AGMember.mdl");
	
	PRECACHE_SOUND("items/gunpickup1.wav");
	
	if ( pev->frags == HUMAN_WEAPON_NONE )	// If no weapons precache all possible weapon sounds because we could pick up any one
	{
		// m16
		PRECACHE_SOUND("weapons/m16_burst.wav");
		PRECACHE_SOUND("weapons/m16_clipinsert1.wav");

		// m60
		PRECACHE_SOUND("weapons/m60_fire.wav");
		PRECACHE_SOUND("weapons/m60_reload_full.wav");

		// 870
		PRECACHE_SOUND("weapons/870_buckshot.wav");
		PRECACHE_SOUND("weapons/870_pump.wav");
		PRECACHE_SOUND("weapons/reload1.wav");
		PRECACHE_SOUND("weapons/reload2.wav");
		PRECACHE_SOUND("weapons/reload3.wav");

		m_iBrassShell = PRECACHE_MODEL ("models/shell.mdl");// brass shell
		m_iShotgunShell = PRECACHE_MODEL ("models/shotgunshell.mdl"); //shotgun shell
	}
	
	m_szFriends[0] = "monster_agency_member";
	m_szFriends[1] = "monster_AGMEMBER_medic";
	m_szFriends[2] = "monster_barney";
	m_szFriends[3] = "player";
	m_szFriends[4] = "monster_peasant";
	m_nNumFriendTypes = 5;

	strcpy( m_szSpeechLabel, "MF_");

	CBaseHumanFollower::Precache();
}

//=========================================================
// Spawn
//=========================================================

void CAgencyMember::Spawn()
{
    Precache( );

	if (pev->model)
		SET_MODEL(ENT(pev), STRING(pev->model)); //LRC
	else
		SET_MODEL(ENT(pev), "models/AGMember.mdl");

	if (pev->health == 0) //LRC
	pev->health			= 100;

	m_flFieldOfView		= VIEW_FIELD_WIDE;	// NOTE: we need a wide field of view so npc will notice player and say hello

	//just in case
	bTerrorist			= FALSE;
	
	CBaseHumanFollower::Spawn();
}

//=========================================================
// PickUpGun - Anim event for episode d prison scripts
//=========================================================

void CAgencyMember :: PickUpGun( int gun )
{
	pev->frags = gun;

	EMIT_SOUND( ENT(pev), CHAN_ITEM, "items/gunpickup1.wav", 1, ATTN_NORM );

	m_cAmmoLoaded = m_cClipSize;
	m_afCapability |= bits_CAP_RANGE_ATTACK1;
}

//=========================================================
// GetDuration - Lengths of sentences
//=========================================================

float CAgencyMember :: GetDuration( const char *pszSentence )
{
	if ( !strcmp( pszSentence, "CHARGE" ) ) return 4;
	if ( !strcmp( pszSentence, "COVER" ) ) return 5;
	if ( !strcmp( pszSentence, "ZOMBIE" ) ) return 10;
	if ( !strcmp( pszSentence, "TAUNT" ) ) return 7;
	if ( !strcmp( pszSentence, "KILL" ) ) return 11;
	if ( !strcmp( pszSentence, "WOUND" ) ) return 9;
	if ( !strcmp( pszSentence, "MORTAL" ) ) return 8;
	if ( !strcmp( pszSentence, "SMELL" ) ) return 9;
	if ( !strcmp( pszSentence, "HURTA" ) ) return 7;
	if ( !strcmp( pszSentence, "HURTB" ) ) return 4;
	if ( !strcmp( pszSentence, "DEAD" ) ) return 7;
	if ( !strcmp( pszSentence, "STARE" ) ) return 15;
	if ( !strcmp( pszSentence, "QUESTION" ) ) return 9;
	if ( !strcmp( pszSentence, "ANSWER" ) ) return 14;
	if ( !strcmp( pszSentence, "IDLE" ) ) return 10;
	if ( !strcmp( pszSentence, "MEDIC" ) ) return 7;
	if ( !strcmp( pszSentence, "HEAL" ) ) return 5;
	if ( !strcmp( pszSentence, "HEALED" ) ) return 7;
	if ( !strcmp( pszSentence, "HELP" ) ) return 7;
	if ( !strcmp( pszSentence, "SEARCH" ) ) return 7;
	if ( !strcmp( pszSentence, "RETREAT" ) ) return 5;
	if ( !strcmp( pszSentence, "UNUSE" ) ) return 7;
	if ( !strcmp( pszSentence, "PIDLE" ) ) return 14;
	if ( !strcmp( pszSentence, "POK" ) ) return 5;
	if ( !strcmp( pszSentence, "SHOT" ) ) return 16;

	return CBaseHumanFollower::GetDuration( pszSentence );
}


//=========================================================
// GibMonster
//=========================================================

void CAgencyMember :: GibMonster( void )
{
	CBaseHumanFollower::GibMonster();
}


//=========================================================
// Victory Dance
// Over-rides base because we want mike to drink whisky
//=========================================================

Task_t tlMikeForceVictoryDance[] =
{
	{ TASK_STOP_MOVING,			0							},
	{ TASK_HUMAN_UNCROUCH,		(float)0					},
	{ TASK_PLAY_SEQUENCE,		(float)ACT_VICTORY_DANCE	},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE				},
	{ TASK_HUMAN_SOUND_VICTORY,	(float)0					},
	{ TASK_SUGGEST_STATE,		(float)MONSTERSTATE_IDLE	},
};

Schedule_t slMikeForceVictoryDance[] =
{
	{
		tlMikeForceVictoryDance,
		ARRAYSIZE( tlMikeForceVictoryDance ),
		bits_COND_NEW_ENEMY			|
		bits_COND_SEE_FEAR			|
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_HEAR_SOUND |
		bits_COND_PROVOKED,
		bits_SOUND_DANGER,
		"MikeForce Victory Dance"
	},
};
		
DEFINE_CUSTOM_SCHEDULES( CAgencyMember )
{
	slMikeForceVictoryDance,
};

IMPLEMENT_CUSTOM_SCHEDULES( CAgencyMember, CBaseHumanFollower );


//=========================================================
// MIKEFORCE REPEL
//=========================================================

class CAgencyMemberRepel : public CBaseHumanRepel
{
public:
	virtual char * EntityName() { return "monster_agency_member"; };
};

LINK_ENTITY_TO_CLASS( monster_agency_member_repel, CAgencyMemberRepel );


//=========================================================
// DEAD MikeForce PROP
//=========================================================

class CDeadMikeForce : public CBaseMonster
{
public:
	void Spawn( void );
	int	Classify ( void ) { return	CLASS_PLAYER_ALLY; }

	void KeyValue( KeyValueData *pkvd );

	int	m_iPose;// which sequence to display	-- temporary, don't need to save
	static char *m_szPoses[8];
};

char *CDeadMikeForce::m_szPoses[] = {	"lying_on_back",	"lying_on_side",	"lying_on_stomach", 
										"hanging_byfeet",	"hanging_byarms",	"hanging_byneck",
										"deadsitting",		"deadseated"	};

void CDeadMikeForce::KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "pose"))
	{
		m_iPose = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else 
		CBaseMonster::KeyValue( pkvd );
}

LINK_ENTITY_TO_CLASS( monster_agency_member_dead, CDeadMikeForce );


//=========================================================
// ********** DeadMikeForce SPAWN **********
//=========================================================

void CDeadMikeForce :: Spawn( void )
{
	PRECACHE_MODEL("models/namGrunt.mdl");
	SET_MODEL(ENT(pev), "models/namGrunt.mdl");

	pev->effects		= 0;
	pev->yaw_speed		= 8;
	pev->sequence		= 0;
	m_bloodColor		= BLOOD_COLOR_RED;

	pev->sequence = LookupSequence( m_szPoses[m_iPose] );

	if (pev->sequence == -1)
	{
		ALERT ( at_console, "Dead MikeForce with bad pose\n" );
	}

	int nHeadNum;
	if ( pev->body == -1 )
	{
		nHeadNum = RANDOM_LONG( 0, NUM_HEADS - 1 ); 
	}
	else 
	{
		nHeadNum = pev->body;
	}

	pev->body = 0;
	SetBodygroup( 0, nHeadNum );
	SetBodygroup( 0, nHeadNum );
	SetBodygroup( 0, 0 );
	SetBodygroup( 0, 0 );
	SetBodygroup( 0, 0 );
	SetBodygroup( 0, FALSE );


	// Corpses have less health
	pev->health			= 8;

	MonsterInitDead();

	if ( m_iPose >=3 || m_iPose <6 ) 
	{
		pev->movetype = MOVETYPE_NONE;
		pev->framerate = 1;
	}
}