//////////////////////////////////////////////////////////////////////
//
// Human.h: interface for the CBaseHuman class.
//
//////////////////////////////////////////////////////////////////////

#ifndef HUMAN_H
#define HUMAN_H

#ifndef MONSTERS_H
#include "monsters.h"
#include "effects.h"//CSprite
#endif

#define		ROACH_IDLE				0
#define		ROACH_BORED				1
#define		ROACH_SCARED_BY_ENT		2
#define		ROACH_SCARED_BY_LIGHT	3
#define		ROACH_SMELL_FOOD		4
#define		ROACH_EAT				5

#define DOOR		1
#define LADDER		2
#define BREKABLE	3
#define	TRAIN		4		
#define	PLAT		5

//=====================
// bits so we don't repeat key sentences
//=====================

#define bit_saidDamageLight		(1<<0)
#define bit_saidDamageMedium	(1<<1)
#define bit_saidDamageHeavy		(1<<2)
#define bit_saidHello			(1<<3)
#define bit_saidWoundLight		(1<<4)
#define bit_saidWoundHeavy		(1<<5)
#define bit_saidHeard			(1<<6)
#define bit_saidSmelled			(1<<7)
#define bit_saidQuestion		(1<<8)
#define bit_saidAnswer			(1<<9)
#define bit_saidIdle			(1<<10)
#define bit_saidDead			(1<<11)
#define bit_saidStare			(1<<12)

#define TALKRANGE_MIN 500.0				// don't talk to anyone farther away than this

#define HUMAN_ATTN ATTN_NORM

#define NO_RESISTENCE				0
#define LOW_RESISTENCE				50
#define MED_RESISTENCE				100
#define HIGH_RESISTENCE				250
#define SUPER_RESISTENCE			500

//=====================
// Squad Commands
// Priority ordered - lowest number is lowest priority
//=====================

typedef enum
{
	SQUADCMD_NONE = 0,				// Just do your own thing - lowest priority, the group is more important!
	SQUADCMD_CHECK_IN,				// Check in - i.e. inform squad leader if I have an enemy or not
	SQUADCMD_SEARCH_AND_DESTROY,	// Search for a target to attack - more immediate goals like attack are more important
	SQUADCMD_OUTTA_MY_WAY,			// Bugger off - unlikely to be life-threatening so low priority
	SQUADCMD_FOUND_ENEMY,			// I have found an enemy
	SQUADCMD_DISTRESS,				// When I'm hurt or injured or something
	SQUADCMD_COME_HERE,				// Come to me - not as important as attacking or retreating
	SQUADCMD_SURPRESSING_FIRE,		// Fire at a designated place - not as important as attack but still important
	SQUADCMD_ATTACK,				// Attack the leader's target, if you are not attacking something else
	SQUADCMD_DEFENSE,				// Defend squad member with lowest health - we look after each other so highest priority
	SQUADCMD_RETREAT,				// Run away from whatever you are attacking - high priority as we want to live
	SQUADCMD_GET_DOWN,				// Crouch - can have high priority as it doesn't alter m_nLastSquadCommand
	SQUADCMD_BEHIND_YOU,			// Turn around - can have high priority as it doesn't alter m_nLastSquadCommand
} SquadCommand;

#define SQUAD_COMMAND_MEMORY_TIME 30	// How long I remember it for
#define SQUAD_COMMAND_RANGE 1024		// How close they have to be in order to recieve a command
#define	PLAYER_MINIMUN_LIGHT			10// el valor de luz minimo permitido (a partir de ese valor para abajo no puede ver el player)

//=====================
// Weapons
//=====================
enum
{
	HUMAN_WEAPON_NONE = 0,

	//Terrorists/AG weapons
	HUMAN_WEAPON_MP5,		//1
	HUMAN_WEAPON_SHOTGUN,	//2
	HUMAN_WEAPON_LAW,		//3
	HUMAN_WEAPON_AWP,		//4
	HUMAN_WEAPON_ASSAULT,	//5
	HUMAN_WEAPON_PISTOL,	//6
	HUMAN_WEAPON_DESERT,	//7
	HUMAN_WEAPON_M249,		//8

	//HL2 npc weapons
	HUMAN_WEAPON_MP7,		//9
	HUMAN_WEAPON_IRGUN,		//10
	HUMAN_WEAPON_357,		//11
	HUMAN_WEAPON_USP,		//12

	HUMAN_WEAPON_UZI,		//13
	HUMAN_WEAPON_USAS		//14
};

//=====================
// BodyGroups
//=====================
enum 
{
	HUMAN_BODYGROUP_BODY = 0,
	HUMAN_BODYGROUP_HEAD,
	HUMAN_BODYGROUP_WEAPON
};

enum
{
	HUMAN_BODY_WEAPON_SMG = 0,
	HUMAN_BODY_WEAPON_SPAS12,//1
	HUMAN_BODY_WEAPON_LAW,//2
	HUMAN_BODY_WEAPON_SNIPER,//3
	HUMAN_BODY_WEAPON_ASSAULT,//4
	HUMAN_BODY_WEAPON_PISTOL,//5
	HUMAN_BODY_WEAPON_DEAGLE,//6
	HUMAN_BODY_WEAPON_M249,//7
	HUMAN_BODY_WEAPON_NONE,//8
	HUMAN_BODY_WEAPON_NONE2,//9
	HUMAN_BODY_WEAPON_USAS,//10
	HUMAN_BODY_WEAPON_UZI//11
};

#define HUMAN_RIFLE_RANGE 8192//2048
#define HUMAN_KICK_RANGE 64

#define HUMAN_EXPLOSIVE_MAX_RANGE 4096
#define HUMAN_EXPLOSIVE_MIN_RANGE 256

//=====================
// Animation Events
//=====================

#define		HUMAN_AE_RELOAD			( 2 )
#define		HUMAN_AE_KICK			( 3 )

#define		HUMAN_AE_BURST1			( 4 )
#define		HUMAN_AE_BURST2			( 5 )
#define		HUMAN_AE_BURST3			( 6 )
 
#define		HUMAN_AE_GREN_TOSS		( 7 )
#define		HUMAN_AE_GREN_LAUNCH	( 8 )
#define		HUMAN_AE_GREN_DROP		( 9 )

#define		HUMAN_AE_CAUGHT_ENEMY	( 10) // grunt established sight with an enemy (player only) that had previously eluded the squad.
#define		HUMAN_AE_DROP_GUN		( 11) // grunt (probably dead) is dropping his mp5.

#define		HUMAN_AE_DIE_SHOOT		( 22 )//NOT USED
#define		HUMAN_AE_DRAW			( 25 )

//#define	HUMAN_AE_HEAL			( 30 )//NOT USED
#define		HUMAN_AE_HEAL_CONTINOUS ( 33 )

#define		HUMAN_AE_KAMIKAZE		( 30 )//define when the monster explodes

#define		HUMAN_AE_PICKITEM		( 40 )

//i put it here to prevent errors
#define		LAST_HUMAN_ANIM_EVENT ( 99 )

//=====================
// Spawn Flags
//=====================

//determines if the monster uses grenades or not. We will check later grenade's number.
#define SF_HUMAN_HANDGRENADES	0x0064
#define SF_SLEEPING				512//defines is the mosnters is sleeping


//=========================================================
// monster-specific schedule types
// These are schedules that only this monster does - NOT
// schedules that this monster does slightly differently
// from other monsters - those can be merely over-ridden
// in GetScheduleofType
//=========================================================

enum
{
	SCHED_HUMAN_SURPRESS = LAST_COMMON_SCHEDULE + 1,	// Lay down surpressing fire
	SCHED_HUMAN_ESTABLISH_LINE_OF_FIRE,// move to a location to set up an attack against the enemy. (usually when a friendly is in the way).
	SCHED_HUMAN_COVER_AND_RELOAD,	// Run away and reload
	SCHED_HUMAN_RELOAD,				// Reload
	SCHED_HUMAN_SWEEP,				// Keep constantly turning round in a paranoid fashion
	SCHED_HUMAN_FOUND_ENEMY,		// Inform squad when he's found an enemy
	SCHED_HUMAN_WAIT_FACE_ENEMY,	// Once in cover, wait to attack something
	SCHED_HUMAN_TAKECOVER_FAILED,	// Special schedule type that forces analysis of conditions and picks the best possible schedule to recover from this type of failure.
	SCHED_HUMAN_ELOF_FAIL,			// Establish line of fire fails
	SCHED_HUMAN_POPUP_ATTACK,		// If crouching behind a waist-high obstacle, pop up, shoot, and duck
	SCHED_HUMAN_HEAR_SOUND,			// Crouch and say "shhh"
	SCHED_HUMAN_REPEL,				// repel down
	SCHED_HUMAN_REPEL_ATTACK,		// repel while shooting
	SCHED_HUMAN_REPEL_LAND,			// stop repelling
	SCHED_HUMAN_REPEL_LAND_SEARCH_AND_DESTROY, // stop repelling and go searching and destroying
	SCHED_HUMAN_FIND_MEDIC,			// Try and find a medic
	SCHED_HUMAN_FIND_MEDIC_COMBAT,	// Signal squad to defend you, try and find a medic, if you can't then just run away
	SCHED_HUMAN_SIGNAL_ATTACK,		// Fire at target and signal group to do likewise
	SCHED_HUMAN_SIGNAL_SURPRESS,	// Fire at target and signal group to do likewise
	SCHED_HUMAN_SIGNAL_SEARCH_AND_DESTROY, // Signal to go looking for hostiles
	SCHED_HUMAN_SIGNAL_RETREAT,		// Signal the retreat, fire at target for a bit and then run away
	SCHED_HUMAN_SIGNAL_COME_TO_ME,	// Holler for the squad to come to me
	SCHED_HUMAN_SIGNAL_CHECK_IN,	// Find out if anyone else in my squad is fighting
	SCHED_HUMAN_UNCROUCH,			// Get up, and probably make some comment or other
	SCHED_HUMAN_FOLLOW,				// Follow to within 128 of target
	SCHED_HUMAN_FOLLOW_CLOSE,		// Follow to within 64 of target
	SCHED_HUMAN_TAKE_COVER_FROM_ENEMY_NO_GRENADE, // Take cover without dropping any grenades
	SCHED_HUMAN_SEARCH_AND_DESTROY,	// Go on a wide loop to find an enemy
	SCHED_HUMAN_IDLE_RESPONSE,		// Respond to a question some damn fool asked
	SCHED_HUMAN_TURN_ROUND,			// Spin round very fast to see what's behind me
	SCHED_HUMAN_MOVE_TO_ENEMY_LKP,	// Move within range of enemy last known position in order to lay down surpressing fire
	SCHED_HUMAN_EXPLOSION_DIE,		// Fly through the air and die after being hit by an explosion
	SCHED_HUMAN_WAIT_HEAL,			// Wait until doc has injected you
	SCHED_HUMAN_MELEE_ELOF,			// Get to a position where I can kick the enemy
	SCHED_HUMAN_UNCROUCH_SCRIPT,	// Uncrouch and then perform a scripted sequence
	SCHED_HUMAN_RETREAT,			// in response to order, say yes sir, and run away
	SCHED_HUMAN_CHECK_IN,			// Say "All Clear"

	//specific schedules for our terrorists
	SCHED_HUMAN_SURRENDER,			// Surrender
	SCHED_HUMAN_DRAW_WEAPON,		// Deploy weapon
	SCHED_HUMAN_SIDESTEP_RIGHT,
	SCHED_HUMAN_SIDESTEP_LEFT,
	SCHED_HUMAN_DROP_WEAPON,		// Draw weapon
	SCHED_GO_TO_AI_MARKER,
	SCHED_GO_TO_ITEM,
	SCHED_GO_TO_AI_HOTSPOT,
	SCHED_HUMAN_WAKEUP,
	SCHED_HUMAN_FIREUP,

	LAST_HUMAN_SCHEDULE,
};

//=========================================================
// monster-specific tasks
//=========================================================

enum 
{
	TASK_HUMAN_FACE_TOSS_DIR = LAST_COMMON_TASK + 1,	// project a point along the toss vector and turn to face that point
	TASK_HUMAN_FIND_MEDIC,			// Find a medic and set him to be your new target entity
	TASK_HUMAN_CHECK_FIRE,			// Is it okay to fire?
	TASK_HUMAN_FORGET_SQUAD_COMMAND,// Set the last squad command you heard (if you have completed it or whatever)
	TASK_HUMAN_CROUCH,				// Go into crouch mode
	TASK_HUMAN_UNCROUCH,			// Go out of crouch mode
	TASK_HUMAN_SOUND_HEAR,			// For when you hear a noise that could mean trouble
	TASK_HUMAN_SOUND_GRENADE,		// For when you hear a grenade near you
	TASK_HUMAN_SOUND_VICTORY,		// For when you have killed something
	TASK_HUMAN_SOUND_MEDIC,			// For when you are hurt and need medical assistance
	TASK_HUMAN_SOUND_HELP,			// For when you are being trashed and need medical assistance and defense
	TASK_HUMAN_SOUND_ATTACK,		// For when you are about to attack something
	TASK_HUMAN_SOUND_THROW,			// For when you are about to throw a grenade
	TASK_HUMAN_SOUND_COVER,			// For when you are about to run to cover
	TASK_HUMAN_SOUND_SURPRESS,		// For when you are ordering your troops to lay down surpressing fire
	TASK_HUMAN_SOUND_SURPRESSING,	// For when you are about to lay down surpressing fire
	TASK_HUMAN_SOUND_SEARCH_AND_DESTROY, // For when you are ordering your troops to search and destroy
	TASK_HUMAN_SOUND_SEARCHING,		// For when you are about to search and destroy
	TASK_HUMAN_SOUND_RETREAT,		// For when you are ordering your troops to retreat
	TASK_HUMAN_SOUND_RETREATING,	// For when you are about to retreat
	TASK_HUMAN_SOUND_AFFIRMATIVE,	// Indicating you have recieved an order
	TASK_HUMAN_SOUND_COME_TO_ME,	// For when you are ordering your troops to come to you
	TASK_HUMAN_SOUND_RESPOND,		// Reply to question
	TASK_HUMAN_SOUND_CHARGE,		// For when you are running to find a target
	TASK_HUMAN_SOUND_TAUNT,			// For when you are in a standoff
	TASK_HUMAN_SOUND_FOUND_ENEMY,	// For when you have found a hostile
	TASK_HUMAN_SOUND_HEALED,		// For when you have just been healed
	TASK_HUMAN_SOUND_CHECK_IN,		// For when you are ordering your troops to check in
	TASK_HUMAN_SOUND_CLEAR,			// For when you are checking in
	TASK_HUMAN_SOUND_EXPL,			// For when you are flying through the air after being hit by an explosion
	TASK_HUMAN_EYECONTACT,			// Turn head to face talk target
	TASK_HUMAN_IDEALYAW,			// Set ideal yaw to face talk target
	TASK_HUMAN_EXPLOSION_FLY,		// Fly through the air waving arms around like idiot
	TASK_HUMAN_EXPLOSION_LAND,		// Hit ground and crumple into pathetic heap
	TASK_HUMAN_GET_EXPLOSIVE_PATH_TO_ENEMY,	// Get a path to a place I can fire explosives at enemy without hitting myself
	TASK_HUMAN_GET_MELEE_PATH_TO_ENEMY,	// Get a path to a place where I can kick the enemy
	TASK_HUMAN_RETREAT_FACE,		// Face the enemy, or if he is not visible just spin 180 degrees
	TASK_HUMAN_WAIT_GOAL_VISIBLE,	// Wait till you can see the enemy LKP

	//new tasks to handle monster's actions
	TASK_HUMAN_SURRENDER,	//only for terrorist
	TASK_HUMAN_SIDESTEP_RIGHT,	//only for terrorists
	TASK_HUMAN_SIDESTEP_LEFT,	//only for terrorists
	TASK_HUMAN_PICK_WEAPON,	//I find a amazing weapon, pick it up
	TASK_HUMAN_PICK_ITEM,	//I find a item (healht or something useful) pick up
	TASK_HUMAN_DROP_WEAPON,	//only for terrorist
	TASK_REMOVE_ITEM,
	TASK_HUMAN_WAKEUP,
	TASK_HUMAN_ONFIRE,

	LAST_HUMAN_TASK,
};




//=========================================================
// Monster-specific definitions
//=========================================================

#define CROUCH_TIME 3	// Time spent crouching even if nothing is apparently going on
#define MEDIC_SEARCH_TIME 30	// After I've called for a medic, how soon I can called again
#define KICK_INTERVAL 10	// Min interval between kicks

#define EXPL_THINK_INTERVAL 0.01; // Intervals between checks for FL_ONGROUND when I am flying through air from explosion

#define HUMAN_GIB_HEALTH_VALUE -60

// Friendlies can push each other out of their way
#define	bits_COND_PUSHED ( bits_COND_SPECIAL1 )

#define bits_MEMORY_HUMAN_NO_COVER ( bits_MEMORY_CUSTOM2 )

class CBaseHuman : public CSquadMonster  
{
public:
	int	ObjectCaps(void) { return CSquadMonster::ObjectCaps() | (m_cAmmoLoaded ? FCAP_CONTINUOUS_USE : 0); }

	void EXPORT DeadUse(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);

	BOOL _STRAFE_CODE_TEST;

	void ShowLine( Vector vecStart, Vector vecEnd, int iColor );

	//PATROL TEST
	//PATROL TEST
	//PATROL TEST
	void Move ( float flInterval );
	void PickNewDest ( int iCondition );

	float	m_flNextSmellTime;
	void DetectEnviroment();

	// UNDONE: These don't necessarily need to be save/restored, but if we add more data, it may
	BOOL	m_fLightHacked;
	int		m_iMode;
	//PATROL TEST
	//PATROL TEST
	//PATROL TEST
	virtual BOOL IsFacing( entvars_t *pevTest, const Vector &reference );

	float m_fNextMarkerTime;
	float m_fNextCheckHotspotTime;
	void CreateMarker( void );

	// General functions
	virtual void Spawn( void );
	virtual void Precache( void );
	virtual void StartMonster ( void );
	virtual CBaseHuman *MyHumanPointer( void ) { return this; };

	virtual Vector Center();

	// Animation related functions
	virtual void HandleAnimEvent( MonsterEvent_t *pEvent );
	virtual Activity GetDeathActivity ( void );
	virtual Activity GetSmallFlinchActivity ( void );
	virtual void SetTurnActivity ( void );
	virtual void SetActivity ( Activity NewActivity );
    virtual void SetYawSpeed ( void );

	virtual void MonsterThink ( void );
	
	// Speech related functions
	virtual int FOkToShout( void );
	virtual int FOkToSpeak( void );
	virtual void IdleSound( void );
	virtual void AlertSound( void );
	virtual void DeathSound( void );
	virtual void PainSound( void );
	virtual int	CanPlaySentence( BOOL fDisregardState );
	virtual void PlaySentence( const char *pszSentence, float duration, float volume, float attenuation, float pitch );
	virtual void PlayLabelledSentence( const char *pszSentence );
	virtual void PlayScriptedSentence( const char *pszSentence, float duration, float volume, float attenuation, BOOL bConcurrent, CBaseEntity *pListener );
	virtual int GetVoicePitch( void );
	virtual float GetDuration( const char *pszSentence ) { return 3; };
	virtual void SetAnswerQuestion( CBaseHuman *pSpeaker, float duration );
	virtual void Talk( float flDuration );
	virtual BOOL IsTalking( void );

	// Interaction with player
	virtual BOOL CanFollow( void );
	virtual BOOL IsFollowing( void ) { return IsFollowingHuman() || IsFollowingPlayer(); }
	virtual BOOL IsFollowingHuman( void ) { return m_hTargetEnt != NULL && m_hTargetEnt->MyHumanPointer(); }
	virtual BOOL IsFollowingPlayer( void ) { return m_hTargetEnt != NULL && m_hTargetEnt->IsPlayer(); }
	virtual void LimitFollowers( CBaseEntity *pCaller, int maxFollowers );
	virtual void StopFollowing( BOOL clearSchedule );
	virtual void StartFollowing( CBaseEntity *pLeader );
	virtual void IdleHeadTurn( Vector &vecFriend );

	//Interaction-with-friendlies functions
	virtual CBaseEntity	*FindNearestFriend( BOOL fPlayer );
	virtual CBaseEntity	*EnumFriends( CBaseEntity *pPrevious, int listNumber, BOOL bTrace );
	virtual void AlertFriends( void );
	virtual void ShutUpFriends( void );
	virtual void SquadReceiveCommand( SquadCommand Cmd );
	virtual void SquadIssueCommand ( SquadCommand Cmd );
	virtual BOOL SquadCmdLegalForNonLeader( SquadCommand Cmd );
	virtual BOOL SquadGetCommanderEnemy();
	virtual BOOL SquadGetCommanderLineOfSight();
	virtual BOOL SquadIsHealthy();
	virtual BOOL SquadGetWounded();
	virtual BOOL SquadGetMemberEnemy();
	virtual BOOL SquadIsScattered();
	virtual BOOL SquadAnyIdle();
	virtual BOOL IsMedic() { return FALSE; };

	// AI functions
	virtual int IRelationship( CBaseEntity *pTarget );
	virtual void RunTask( Task_t *pTask );
	virtual void StartTask( Task_t *pTask );
	virtual Schedule_t *GetScheduleOfType ( int Type );
	virtual Schedule_t *GetSchedule ( void );
	virtual Schedule_t *GetScheduleFromSquadCommand ( void );
	virtual MONSTERSTATE GetIdealState ( void );
	virtual void PrescheduleThink( void );
	virtual int ISoundMask( void );
	virtual void MoveExecute( CBaseEntity *pTargetEnt, const Vector &vecDir, float flInterval ) 
	{
		Forget( bits_MEMORY_HUMAN_NO_COVER );	// If I move it may become possible to find cover
		CBaseMonster::MoveExecute( pTargetEnt, vecDir, flInterval );
	};
	virtual BOOL SafeToChangeSchedule( void ) 
	{ 
		return ( !m_pCine && pev->movetype == MOVETYPE_STEP && pev->deadflag == DEAD_NO &&
		( m_MonsterState == MONSTERSTATE_IDLE || m_MonsterState == MONSTERSTATE_ALERT || m_MonsterState == MONSTERSTATE_COMBAT ) ); 
	};

	//Weapon related functions
	virtual void CheckAmmo ( void );
	virtual BOOL CheckRangeAttack1 ( float flDot, float flDist );
	virtual BOOL CheckRangeAttack2 ( float flDot, float flDist );
	virtual BOOL CheckMeleeAttack1 ( float flDot, float flDist );
	virtual BOOL CheckBulletAttack( float flDot, float flDist );
	virtual BOOL CheckRocketAttack( float flDot, float flDist );
	virtual BOOL CheckContactGrenadeAttack( float flDot, float flDist );
	virtual BOOL CheckTimedGrenadeAttack( float flDot, float flDist );
	virtual BOOL FCanCheckAttacks ( void );
	virtual CBaseEntity	*Kick( void );

	virtual char * WeaponEntityName();
	virtual Vector GetGunPosition( );
	virtual Vector GetGunEndPosition( );
	virtual void Fire ( int nBulletType, int Num, Vector vecAccuracy, int nShellModel, int nShellBounce, const char * szSnd );
	virtual void M79Fire();
	virtual void RPGFire();

	//Damage related functions
	virtual void TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType);
	virtual int TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType);
	virtual void Killed( entvars_t *pevAttacker, int iGib );
	virtual void GibMonster ( void );
	virtual BOOL ShouldGibMonster( int iGib );
	void EXPORT ExplFlyTouch( CBaseEntity *pOther );

	// Variables
	static float g_talkWaitTime;

	int	m_voicePitch;			// pitch of voice for this individual
	BOOL m_fHandGrenades;		// Do I have grenades?
	BOOL m_fCrouching;			// Am I crouching?
	BOOL m_fStopCrouching;		// Do I need to stop crouching in order to be able to shoot something?
	float m_flCrouchTime;		// How long have I been crouching for?
	int m_nLastSquadCommand;	// Last known command given by squad leader
	float m_flLastSquadCmdTime; // Time since last squad command was given
	BOOL m_fSquadCmdAcknowledged; // Have I replied yet?
	float m_painTime;			// Last time I was hit
	float m_flPlayerDamage;		// How much pain has the player inflicted on me?
	char m_szSpeechLabel[3];	// The first three letters of his speech groups in sentences.txt
	
	int m_szIdleAnimation;//int??

	float m_flNextAttack1Check;	// Time when we can next check if it's okay to use attack1 (shoot, usually)
	BOOL m_LastAttack1Check;	// Result of last check
	float m_flNextAttack2Check; // Time when we can next check if it's okay to use attack2 (grenade, usually)
	BOOL m_LastAttack2Check;	// Result of last check
	Vector m_vecTossVelocity;	// Grenade speed required to hit target
	int	m_cClipSize;			// Clip size
	char * m_szFriends[5];		// classnames of friendly monsters
	int m_nNumFriendTypes;		// Number of classnames of friendly monsters
	EHANDLE	m_hTalkTarget;		// who to look at while talking
	float m_flStopTalkTime;		// when in the future that I'll be done saying this sentence.
	int	m_bitsSaid;				// set bits for sentences we don't want repeated
	float m_flLastTalkTime;		// The last time I said something (idle chat only)
	int m_expldir;				// horrible variable whose existence I despise

	float m_flLastMedicSearch;	// Last time I searched for a medic
	float m_flLastKick;			// Last time I kicked someone

	int	m_iBrassShell;
	int	m_iShotgunShell;

	// Save functions
	virtual int	Save( CSave &save );
	virtual int	Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	BOOL HasHumanGibs( void ) { return TRUE; };
	void PossessEntity( void );
	BOOL CineCleanup();
	
	CBaseEntity* DropGun(char *szName, Vector origin, Vector angles);
	
	float yaw_adj;
	//NEW
	//NEW
	//NEW
	//NEW
	//NEW
	
	//shit, I need to modify nomenclatures... :(
	BOOL	m_bUseSideJump;//side jump
	BOOL	m_bDontReact;//do not react
	BOOL	m_fGunDrawn;//true when a weapon has been drawed
	BOOL	m_fStealthed;//the monster spawn as stealth
	BOOL	bCanGoMarker;//true when the monster can go to marker
	BOOL	bTerrorist;//true when the monster it's a terrorist (replaces weapons)
	BOOL	m_bDropped;//true when a primary weapon has been droped
	BOOL	m_fHasDeagle;//true if the monster has an deagle for secondary weapon
	BOOL	m_bCanDropPrimWeapon;//true if the monster can drop his weapon
	BOOL	bViolentHeadDamaged;//true if has a high head shot damage

	BOOL	m_bKamikaze;//the monster now doesn't have more weapons and it's prepared to suicide
	BOOL	m_bUseArmor;//saves headshots
	BOOL	m_bSurrenderStanding;//pick one surrender style animation (dont switch each other)

	BOOL	bPlayerVisible;//is the player hidden? can I see it?
	BOOL	b_FacingEnemyForShoot;//am i facing my enemy?
	BOOL	bSlow;
	BOOL	bOnFire;
	BOOL	bFistsReady;

	BOOL	m_bSideStepRightDone;
	BOOL	m_bSideStepLeftDone;

//	BOOL	m_bWakeUp;

	int		m_iNumClips;//amount of clips
	int		m_iNumGrenades;//amount of grenades
	
	int		m_flNextSuspCheckTime;
	int		m_flNextReportTime;
	int		m_flNextReportEnemyWeaponTime;
//	int		m_iFrustration;//what kind and amount of damage the player inflicted on me
	//used cbase entity m_iFrustration
	//moved to cbase.h to access on agency_member.cpp
//	int		m_iBreakPoint;//some monsters can react so bad to player, others don't and they just surrender.
	int		iSlowmocounter;
	float	m_flLastLightLevel;

	void	PickUpGun( int gun );
	void	ReportEnemyPosition( void );
	void	ReportEnemyType( void );
	void	ReportEnemyFrustration( void );

	//don't this values: re calc'ed
	Vector m_angGun;
	int iDistToShoot;
	float m_flNextGrenadeCheck;
	int iTries;

//	bool bSchedSended;
	BOOL FValidateHintType ( short sHint );
	BOOL FCanActiveIdle ( void );

	//don't this values: re calc'ed
	//NEW
	//NEW
	//NEW
	//NEW
	//NEW

	CUSTOM_SCHEDULES;

private:
	CSprite		*pSprite;
};


//=========================================================
// CBaseHumanRepel - when triggered, spawns a human of some kind
// repelling down a line.
//=========================================================

class CBaseHumanRepel : public CBaseMonster
{
public:
	void Spawn( void );
	void Precache( void );
	void EXPORT RepelUse ( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	virtual char * EntityName() = 0;
	int m_iSpriteTexture;	// Don't save, precache
};
/*

//=========================================================
// CBaseHumanHead - a head that you can kick around
//=========================================================

class CBaseHumanHead : public CBaseMonster
{
public:
	void Spawn( const char *szHeadModel, int nBodyNum );
	int Classify( void ) { return CLASS_ALIEN_PASSIVE; };
	BOOL HasHumanGibs( void ) { return TRUE; };
	void EXPORT BounceGibTouch ( CBaseEntity *pOther );
	void EXPORT SolidThink( void );
	void EXPORT BounceUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

//	virtual int	ObjectCaps( void ) { return CBaseMonster :: ObjectCaps() | FCAP_IMPULSE_USE; }
	// Wargon: Юзать монстра можно только если он жив. Это нужно чтобы иконка юза не показывалась на мертвых монстрах.
	virtual int	ObjectCaps( void ) { if (pev->deadflag == DEAD_NO) return CBaseMonster :: ObjectCaps() | FCAP_IMPULSE_USE | FCAP_DISTANCE_USE; else return CBaseMonster::ObjectCaps(); }

	void Killed( entvars_t *pevAttacker, int iGib );
};
*/

#endif
