/***
*
*		   °°
*			 °     °°°°°
*			° °   °  °  °
*		   °   °  °  °  °
*		  °     ° °  °  °
*	   HALF-LIFE: ARRANGEMENT
*
*	AR Software (c) 2004-2007. ArrangeMent, S2P Physics, Spirit Of Half-Life and their
*	logos are the property of their respective owners.
*
*	You should have received a copy of the Developers Readme File
*   along with Arrange Mode's Source Code for Half-Life. If you
*	are going to copy, modify, translate or distribute this file
*	you should agree whit the terms of Developers Readme File.
*
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*
*	This product includes information related to Half-Life 2 licensed from Valve 
*	(c) 2004. 
*
*	All Rights Reserved.
*
*	Modifications by SysOp (sysop_axis@hotmail).
*
*	-------------------------------------------------------------------------------------
*	Based on Original code from Nam (Leader of Hearth of Evil Modification for Half-Life)
*	Modified following specifications on Readme file.
*	-------------------------------------------------------------------------------------
*
***/

//////////////////////////////////////////////////////////////////////
//
// Human.cpp: implementation of the CBaseHuman
//
//////////////////////////////////////////////////////////////////////

#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"schedule.h"
#include	"basemonster.h"
#include	"squadmonster.h"

#include	"CBaseHuman.h"

#include	"defaultai.h"
#include	"scripted.h"
#include	"soundent.h"
#include	"animation.h"
#include	"weapons.h"
#include	"monsters.h"
#include	"player.h"
#include	"nodes.h"

extern int gmsgParticles;//define external message

extern void ExplosionCreate( const Vector &center, const Vector &angles, edict_t *pOwner, int magnitude, BOOL doDamage );

#define GUN_NONE					9

#define SET_WEAPON(t)				pev->skin = t
//#define REMOVE_P_MODEL				pev->weaponmodel = NULL//this could be devil!
#define REMOVE_P_MODEL				pev->skin =88
#define SET_P_MODEL					pev->weaponmodel = MAKE_STRING("models/p_test.mdl")//this could be devil!

//TODO: THIS MUST BE SHARED WITH WORLD WEAPONS FOR THE PLAYER!
#define P_WEAPON_FAMAE	0
#define P_WEAPON_SPAS	1
#define P_WEAPON_LAW	2
#define P_WEAPON_SNIPER	3
#define P_WEAPON_AK47	4
#define P_WEAPON_GLOCK	5
#define P_WEAPON_DEAGLE 6
#define P_WEAPON_M249	7
#define P_WEAPON_USAS	8
#define P_WEAPON_P90	9
#define P_WEAPON_M4A1	10
#define P_WEAPON_1911	11

int iMuzzleFlash;

//=========================================================
//=========================================================
//
// CBaseHuman class functions
//
//=========================================================
//=========================================================

TYPEDESCRIPTION	CBaseHuman::m_SaveData[] = 
{
	DEFINE_FIELD( CBaseHuman, m_voicePitch, FIELD_INTEGER ),
	DEFINE_FIELD( CBaseHuman, m_fHandGrenades, FIELD_BOOLEAN ),
	DEFINE_FIELD( CBaseHuman, m_fCrouching, FIELD_BOOLEAN ),
	DEFINE_FIELD( CBaseHuman, m_fStopCrouching, FIELD_BOOLEAN ),
	DEFINE_FIELD( CBaseHuman, m_flCrouchTime, FIELD_TIME ),
	DEFINE_FIELD( CBaseHuman, m_nLastSquadCommand, FIELD_INTEGER ),
	DEFINE_FIELD( CBaseHuman, m_flLastSquadCmdTime, FIELD_TIME ),
	DEFINE_FIELD( CBaseHuman, m_fSquadCmdAcknowledged, FIELD_BOOLEAN ),
	DEFINE_FIELD( CBaseHuman, m_painTime, FIELD_TIME ),
	DEFINE_FIELD( CBaseHuman, m_flPlayerDamage, FIELD_FLOAT ),

	DEFINE_FIELD( CBaseHuman, m_szSpeechLabel, FIELD_STRING ),

	DEFINE_FIELD( CBaseHuman, m_szIdleAnimation, FIELD_STRING ),

	DEFINE_FIELD( CBaseHuman, m_flNextAttack1Check, FIELD_TIME ),
	DEFINE_FIELD( CBaseHuman, m_LastAttack1Check, FIELD_BOOLEAN ),
	DEFINE_FIELD( CBaseHuman, m_flNextAttack2Check, FIELD_TIME ),
	DEFINE_FIELD( CBaseHuman, m_LastAttack2Check, FIELD_BOOLEAN ),
	DEFINE_FIELD( CBaseHuman, m_vecTossVelocity, FIELD_VECTOR ),
	DEFINE_FIELD( CBaseHuman, m_cClipSize, FIELD_INTEGER ),
	DEFINE_FIELD( CBaseHuman, m_hTalkTarget, FIELD_EHANDLE ),
	DEFINE_FIELD( CBaseHuman, m_flStopTalkTime, FIELD_TIME ),
	DEFINE_FIELD( CBaseHuman, m_bitsSaid, FIELD_INTEGER ),
	DEFINE_FIELD( CBaseHuman, m_flLastTalkTime, FIELD_TIME ),
	DEFINE_FIELD( CBaseHuman, m_expldir, FIELD_INTEGER ),

	DEFINE_FIELD( CBaseHuman, m_flLastMedicSearch, FIELD_TIME ),
	DEFINE_FIELD( CBaseHuman, m_flLastKick, FIELD_TIME ),

//new
	DEFINE_FIELD( CBaseHuman, m_bSurrender, FIELD_BOOLEAN ),
	DEFINE_FIELD( CBaseHuman, bOnFire, FIELD_BOOLEAN ),
//	DEFINE_FIELD( CBaseHuman, m_bWakeUp, FIELD_BOOLEAN ),
	DEFINE_FIELD( CBaseHuman, m_bSurrenderStanding, FIELD_BOOLEAN ),

	DEFINE_FIELD( CBaseHuman, m_fGunDrawn, FIELD_BOOLEAN ),
	DEFINE_FIELD( CBaseHuman, m_fStealthed, FIELD_BOOLEAN ),
	DEFINE_FIELD( CBaseHuman, bCanGoMarker, FIELD_BOOLEAN ),
	DEFINE_FIELD( CBaseHuman, bTerrorist, FIELD_BOOLEAN ),
	DEFINE_FIELD( CBaseHuman, m_bDropped, FIELD_BOOLEAN ),
	DEFINE_FIELD( CBaseHuman, m_fHasDeagle, FIELD_BOOLEAN ),
	DEFINE_FIELD( CBaseHuman, m_bCanDropPrimWeapon, FIELD_BOOLEAN ),
//	DEFINE_FIELD( CBaseHuman, bViolentHeadDamaged, FIELD_BOOLEAN ),
	DEFINE_FIELD( CBaseHuman, bPlayerVisible, FIELD_BOOLEAN ),
	DEFINE_FIELD( CBaseHuman, b_FacingEnemyForShoot, FIELD_BOOLEAN ),

	DEFINE_FIELD( CBaseHuman, m_iNumClips, FIELD_INTEGER ),
	DEFINE_FIELD( CBaseHuman, m_iNumGrenades, FIELD_INTEGER ),
	DEFINE_FIELD( CBaseHuman, m_iFrustration, FIELD_INTEGER ),
	
	DEFINE_FIELD( CBaseHuman, m_flNextSuspCheckTime, FIELD_TIME ),
	DEFINE_FIELD( CBaseHuman, m_flNextReportTime, FIELD_TIME ),
	DEFINE_FIELD( CBaseHuman, m_flNextReportEnemyWeaponTime, FIELD_TIME ),
	DEFINE_FIELD( CBaseHuman, m_bKamikaze, FIELD_BOOLEAN ),
	DEFINE_FIELD( CBaseHuman, m_bUseArmor, FIELD_BOOLEAN ),

	DEFINE_FIELD( CBaseHuman, m_iBreakPoint, FIELD_INTEGER ),
	DEFINE_FIELD( CBaseHuman, m_bDontReact, FIELD_BOOLEAN ),

	DEFINE_FIELD( CBaseHuman, iSlowmocounter, FIELD_INTEGER ),

	DEFINE_FIELD( CBaseHuman, iDistToShoot, FIELD_INTEGER )
};

IMPLEMENT_SAVERESTORE( CBaseHuman, CSquadMonster );

float	CBaseHuman::g_talkWaitTime = 0;		// time delay until it's ok to speak: used so that two NPCs don't talk at once

//=========================================================
// Precache - called by inherited class spawn function
//=========================================================

void CBaseHuman :: Precache( void )
{
	m_iBrassShell = PRECACHE_MODEL("models/shell.mdl");// brass shell
	m_iShotgunShell = PRECACHE_MODEL("models/shotgunshell.mdl");
	iMuzzleFlash = PRECACHE_MODEL( "sprites/muzzleflash3.spr" );

	//check for compatibility for old code
	pev->weapons = pev->frags;

/*	switch ( pev->weapons )
	{
		case HUMAN_WEAPON_MP5:			ALERT( at_console, "HUMAN_WEAPON_MP5\n" ); break;
		case HUMAN_WEAPON_SHOTGUN:		ALERT( at_console, "HUMAN_WEAPON_SHOTGUN\n" ); break;
		case HUMAN_WEAPON_LAW:			ALERT( at_console, "HUMAN_WEAPON_LAW\n" ); break;
		case HUMAN_WEAPON_AWP:			ALERT( at_console, "HUMAN_WEAPON_AWP\n" ); break;
		case HUMAN_WEAPON_ASSAULT:		ALERT( at_console, "HUMAN_WEAPON_ASSAULT\n" ); break;
		case HUMAN_WEAPON_PISTOL:		ALERT( at_console, "HUMAN_WEAPON_PISTOL\n" ); break;
		case HUMAN_WEAPON_DESERT:		ALERT( at_console, "HUMAN_WEAPON_DESERT\n" ); break;
		case HUMAN_WEAPON_M249:			ALERT( at_console, "HUMAN_WEAPON_M249\n" ); break;
				
		case HUMAN_WEAPON_MP7:			ALERT( at_console, "HUMAN_WEAPON_MP7\n" ); break;
		case HUMAN_WEAPON_IRGUN:		ALERT( at_console, "HUMAN_WEAPON_IRGUN\n" ); break;
		case HUMAN_WEAPON_357:			ALERT( at_console, "HUMAN_WEAPON_357\n" ); break;
		case HUMAN_WEAPON_USP:			ALERT( at_console, "HUMAN_WEAPON_USP\n" ); break;
		case HUMAN_WEAPON_UZI:			ALERT( at_console, "HUMAN_WEAPON_UZI\n" ); break;
		case HUMAN_WEAPON_USAS:			ALERT( at_console, "HUMAN_WEAPON_USAS\n" ); break;
	}*/
	
	PRECACHE_SOUND( "weapons/SP0000.wav" );
	PRECACHE_SOUND( "weapons/SP0001.wav" );
//enviroment sounds	
/*
	PRECACHE_SOUND( "enviroment/Behindwall.wav" );
	PRECACHE_SOUND( "enviroment/Behindboxes.wav" );
	PRECACHE_SOUND( "enviroment/Behindboxes-alt.wav" );
	PRECACHE_SOUND( "enviroment/Nearladder.wav" );
	PRECACHE_SOUND( "enviroment/Nearladder-alt.wav" );
	PRECACHE_SOUND( "enviroment/Behindboxes.wav" );
	PRECACHE_SOUND( "enviroment/Behindboxes-alt.wav" );
	PRECACHE_SOUND( "enviroment/Behindcrates.wav" );
	PRECACHE_SOUND( "enviroment/closetocrates.wav" );
	PRECACHE_SOUND( "enviroment/checkoutboxes.wav" );
	PRECACHE_SOUND( "enviroment/checkoutcrates.wav" );
	PRECACHE_SOUND( "enviroment/Nearboxes.wav" );
	PRECACHE_SOUND( "enviroment/Nearboxes-alt.wav" );
	PRECACHE_SOUND( "enviroment/Neardesk.wav" );*/
//enviroment sounds	



	//sound scapes
	PRECACHE_SOUND( "weapons/mp5/fire_sil_dist-1.wav" );
	PRECACHE_SOUND( "weapons/mp5/fire_sil-1.wav" );
	PRECACHE_SOUND( "weapons/mp5/fire_dist-1.wav" );
	PRECACHE_SOUND( "weapons/mp5/fire-1.wav" );
	PRECACHE_SOUND( "weapons/spas12/fire_dist.wav" );
	PRECACHE_SOUND( "weapons/spas12/fire.wav" );
	PRECACHE_SOUND( "weapons/m14/fire_dist.wav" );
	PRECACHE_SOUND( "weapons/m14/fire.wav" );
	PRECACHE_SOUND( "weapons/ak47/fire_dist.wav" );
	PRECACHE_SOUND( "weapons/ak47/fire.wav" );
	PRECACHE_SOUND( "weapons/m4a1/fire_dist.wav" );
	PRECACHE_SOUND( "weapons/m4a1/fire.wav" );
	PRECACHE_SOUND( "weapons/glock18/fire_dist.wav" );
	PRECACHE_SOUND( "weapons/glock18/fire.wav" );
	PRECACHE_SOUND( "weapons/1911/fire_dist.wav" );
	PRECACHE_SOUND( "weapons/1911/fire.wav" );
	PRECACHE_SOUND( "weapons/deagle/fire_dist.wav" );
	PRECACHE_SOUND( "weapons/deagle/fire.wav" );
	PRECACHE_SOUND( "weapons/m249/m249_fire-1_dist.wav" );
	PRECACHE_SOUND( "weapons/m249/m249_fire-1.wav" );
	PRECACHE_SOUND( "weapons/p90/fire_dist.wav" );
	PRECACHE_SOUND( "weapons/p90/fire.wav" );
	PRECACHE_SOUND( "weapons/usas/fire_dist.wav" );
	PRECACHE_SOUND( "weapons/usas/fire.wav" );
	PRECACHE_SOUND( "weapons/rpg/fire.wav" );





	UTIL_PrecacheOther( "hvr_rocket" );

//	PRECACHE_MODEL( "sprites/voiceicon.spr" );

	PRECACHE_SOUND("zombie/claw_miss2.wav");	// kick

	PRECACHE_MODEL("models/p_test.mdl");
/*
	if (pev->frags == HUMAN_WEAPON_LAW)
	{
		m_flDistTooFar		= 1024.0 *4;
		m_flDistLook		= 2048.0 *4;
	}
	else if (pev->frags == HUMAN_WEAPON_AWP)
	{
		m_flDistTooFar		= 1024.0 *6;
		m_flDistLook		= 2048.0 *6;
	}
	else if (pev->frags == HUMAN_WEAPON_ASSAULT)
	{
		m_flDistTooFar		= 1024.0 *2;
		m_flDistLook		= 2048.0 *2;
	}
	else
	{
		m_flDistTooFar		= 1024.0;
		m_flDistLook		= 2048.0;
	}*/

	//TODO, I know, there's no use to shoot a rifle if player isn't going to get hit
	//decreasing the values will make the NPC get closer
		m_flDistTooFar		= 1024.0 *4;
		m_flDistLook		= 2048.0 *4;
}

char* GetRandomNPCName(void);

void CBaseHuman :: Spawn()
{
//spawm monster
	UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	pev->renderfx = 88;//to render p_ model at client side

	_STRAFE_CODE_TEST	= FALSE;
//   pev->flags = FL_CLIENT | FL_FAKECLIENT;
//	CREATE_FAKE_CLIENT( "asd" );
	b_FlashedMonster = FALSE;
	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_RED;
    pev->effects        = 0;
	pev->view_ofs		= Vector ( 0, 0, 68 );// position of the eyes relative to monster's origin.
    pev->yaw_speed      = 15;
	m_MonsterState		= MONSTERSTATE_NONE;

	m_afCapability		= bits_CAP_SQUAD | bits_CAP_HEAR | bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP /*| bits_CAP_STRAFE*/;

//	m_fHandGrenades		= pev->spawnflags & SF_HUMAN_HANDGRENADES;

	m_voicePitch		= 100 + RANDOM_LONG(-10,10);

	m_fCrouching		= FALSE;
	m_fStopCrouching	= FALSE;
	m_nLastSquadCommand  = SQUADCMD_NONE;

//	if ( FStringNull( m_fHandGrenades ))
//	m_fHandGrenades		= 1;//yes, they have for now

//	if ( FStringNull( m_iNumGrenades ))
//	m_iNumGrenades		= RANDOM_LONG(1,3);//WTF sys, WTF !

	if ( FStringNull( iDistToShoot ))
	iDistToShoot		= HUMAN_RIFLE_RANGE;//normal shoot


	m_flFieldOfView		= VIEW_FIELD_NARROW;
//	m_flFieldOfView		= VIEW_FIELD_WIDE;
//	m_flFieldOfView		= VIEW_FIELD_FULL;

	iSlowmocounter		= -1;

	//the following determines if the terrorist will use one of the two differetns animations
	//(standing or on knees)
	if (RANDOM_LONG(0,1))
	m_bSurrenderStanding	= TRUE;
	else
	m_bSurrenderStanding	= FALSE;

	yaw_adj = 0.0;
	
//	if (RANDOM_LONG( 0, 99 ) < 40)
//	bSlow = true;

	
/*		edict_t *pFind; 
		pFind = FIND_ENTITY_BY_CLASSNAME( NULL, "game_no_damage" );
		if (!FNullEnt( pFind ))
		{
				bSlow = true;
		}*/

	m_fNextMarkerTime = gpGlobals->time + 5;
	m_fNextCheckHotspotTime = gpGlobals->time + 10;

	pev->noise3 = MAKE_STRING(GetRandomNPCName());

	ALERT(at_console, "INFO: noise3 is %s\n", STRING(pev->noise3));
	//	ALERT(at_console, "random name: %s\n", GetRandomNPCName());


//if pev->frags is not present (can't find any value, not even zero)
	if ( FStringNull( pev->frags ))
	{
		switch (RANDOM_LONG(0,9))
		{
			case 0: pev->frags = HUMAN_WEAPON_MP5; break;
			case 1: pev->frags = HUMAN_WEAPON_SHOTGUN; break;
			case 2: pev->frags = HUMAN_WEAPON_LAW; break;
			case 3: pev->frags = HUMAN_WEAPON_AWP; break;
			case 4: pev->frags = HUMAN_WEAPON_ASSAULT; break;
			case 5: pev->frags = HUMAN_WEAPON_PISTOL; break;
			case 6: pev->frags = HUMAN_WEAPON_M249; break;
			case 7: pev->frags = HUMAN_WEAPON_DESERT; break;//just I put it here
			case 8: pev->frags = HUMAN_WEAPON_UZI; break;
			case 9: pev->frags = HUMAN_WEAPON_USAS; break;

			
		}
		if (pev->frags == HUMAN_WEAPON_LAW)
			pev->frags = HUMAN_WEAPON_USAS;
	}

//random option from FGD file

	if ( pev->frags == -1)
	{
		switch (RANDOM_LONG(0,9))
		{
			case 0: pev->frags = HUMAN_WEAPON_MP5; break;
			case 1: pev->frags = HUMAN_WEAPON_SHOTGUN; break;
			case 2: pev->frags = HUMAN_WEAPON_LAW; break;
			case 3: pev->frags = HUMAN_WEAPON_AWP; break;
			case 4: pev->frags = HUMAN_WEAPON_ASSAULT; break;
			case 5: pev->frags = HUMAN_WEAPON_PISTOL; break;
			case 6: pev->frags = HUMAN_WEAPON_M249; break;
			case 7: pev->frags = HUMAN_WEAPON_DESERT; break;//just I put it here
			case 8: pev->frags = HUMAN_WEAPON_UZI; break;
			case 9: pev->frags = HUMAN_WEAPON_USAS; break;
		}
		if (pev->frags == HUMAN_WEAPON_LAW)
			pev->frags = HUMAN_WEAPON_USAS;
	}
		
	if (pev->spawnflags & SF_MONSTER_DONT_REACT)
	{
		m_iBreakPoint = 999999; //dont get frustrated
		m_bDontReact = TRUE;
	}
	
	m_bDontReact = FALSE;

	SET_P_MODEL;

	//test
//	pev->frags = HUMAN_WEAPON_NONE;

	bFistsReady = FALSE;

//set bodygroups and ammo
	if (pev->frags == HUMAN_WEAPON_MP5)
	{
	//	SetBodygroup( HUMAN_BODYGROUP_WEAPON, HUMAN_BODY_WEAPON_SMG );
		SET_WEAPON(P_WEAPON_FAMAE);

		m_cClipSize		= 30;
		m_iNumClips		= 3;//120 bullets

		m_iBreakPoint = MED_RESISTENCE;

		if (RANDOM_LONG( 0, 99 ) < 80)
		m_fStealthed = FALSE;
		else
		m_fStealthed = TRUE;
	}
	else if (pev->frags == HUMAN_WEAPON_SHOTGUN)
	{
	//	SetBodygroup( HUMAN_BODYGROUP_WEAPON, HUMAN_BODY_WEAPON_SPAS12 );
		SET_WEAPON(P_WEAPON_SPAS);

		m_cClipSize		= 8;
		m_iNumClips		= 4;//32 bullets
			
		m_iBreakPoint = MED_RESISTENCE;
	}
	else if (pev->frags == HUMAN_WEAPON_LAW)
	{
	//	SetBodygroup( HUMAN_BODYGROUP_WEAPON, HUMAN_BODY_WEAPON_LAW );
		SET_WEAPON(P_WEAPON_LAW);

		m_cClipSize		= 1;
		m_iNumClips		= 1;//single shot
		
		m_iBreakPoint = LOW_RESISTENCE;
	}
	else if (pev->frags == HUMAN_WEAPON_AWP)
	{
	//	SetBodygroup( HUMAN_BODYGROUP_WEAPON, HUMAN_BODY_WEAPON_SNIPER );
		SET_WEAPON(P_WEAPON_SNIPER);

		m_cClipSize		= 1;
		m_iNumClips		= 10;//10 shots
	
		m_iBreakPoint = LOW_RESISTENCE;
	}
	else if (pev->frags == HUMAN_WEAPON_ASSAULT)
	{
	//	SetBodygroup( HUMAN_BODYGROUP_WEAPON, HUMAN_BODY_WEAPON_ASSAULT );
		if (bTerrorist)
		SET_WEAPON(P_WEAPON_AK47);
		else
		SET_WEAPON(P_WEAPON_M4A1);

		m_cClipSize		= 30;
		m_iNumClips		= 3;//120 bullets

		m_iBreakPoint = HIGH_RESISTENCE;
	}
	else if (pev->frags == HUMAN_WEAPON_PISTOL)
	{
	//	SetBodygroup( HUMAN_BODYGROUP_WEAPON, HUMAN_BODY_WEAPON_NONE );

		//weapon will be drawn after
		/*
		if (bTerrorist)
		SET_WEAPON(P_WEAPON_GLOCK);
		else
		SET_WEAPON(P_WEAPON_1911);
*/
		REMOVE_P_MODEL;

		if (bTerrorist)
		m_cClipSize		= 20;
		else
		m_cClipSize		= 8;

		m_iNumClips		= 999;//infinite? bullets

		m_iBreakPoint = LOW_RESISTENCE;
	}	
	else if (pev->frags == HUMAN_WEAPON_DESERT)
	{
	//	SetBodygroup( HUMAN_BODYGROUP_WEAPON, HUMAN_BODY_WEAPON_NONE );
		SET_WEAPON(P_WEAPON_DEAGLE);

		m_cClipSize		= 7;
		m_iNumClips		= 999;//infinite? bullets
		m_fHasDeagle	= TRUE;//TODO: fgedize this value

		m_iBreakPoint = HIGH_RESISTENCE;
	}
	else if (pev->frags == HUMAN_WEAPON_M249)
	{
	//	SetBodygroup( HUMAN_BODYGROUP_WEAPON, HUMAN_BODY_WEAPON_M249 );
		SET_WEAPON(P_WEAPON_M249);

		m_cClipSize		= 70;
		m_iNumClips		= 2;//140 bullets

		m_iBreakPoint = SUPER_RESISTENCE;
	}
	else if (pev->frags == HUMAN_WEAPON_UZI)
	{
	//	SetBodygroup( HUMAN_BODYGROUP_WEAPON, HUMAN_BODY_WEAPON_UZI );
		SET_WEAPON(P_WEAPON_P90);

		m_cClipSize		= 30;
		m_iNumClips		= 3;

		m_iBreakPoint = LOW_RESISTENCE;
	}
	else if (pev->frags == HUMAN_WEAPON_USAS)
	{
	//	SetBodygroup( HUMAN_BODYGROUP_WEAPON, HUMAN_BODY_WEAPON_USAS );
		SET_WEAPON(P_WEAPON_USAS);

		m_cClipSize		= 20;
		m_iNumClips		= 3;

		m_iBreakPoint = HIGH_RESISTENCE;
	}
	else if (pev->frags == HUMAN_WEAPON_NONE)
	{
	//	REMOVE_P_MODEL;

	//	m_bDropped = TRUE;

		m_cClipSize		= 99;
		m_iNumClips		= 99;

		m_bKamikaze = TRUE;//go to kamikaze
	//	SetBodygroup( HUMAN_BODYGROUP_WEAPON, GUN_NONE );
		REMOVE_P_MODEL;
	
	//	ClearConditions(bits_COND_NO_AMMO_LOADED);

		m_iBreakPoint = HIGH_RESISTENCE;
	}

	if(bFistsReady)
	REMOVE_P_MODEL;

	//check for compatibility for old code
	pev->weapons = pev->frags;

	m_cAmmoLoaded		= m_cClipSize;

	//FGD ME!!
	if ( FStringNull( m_bCanDropPrimWeapon ))
	m_bCanDropPrimWeapon = TRUE;//of course! why not!
				
/*	if ( pev->spawnflags & SF_SLEEPING )
	m_bWakeUp	= FALSE;
	else
	m_bWakeUp	= TRUE;
*/

//	if (RANDOM_LONG(0,1))
//	m_bUseArmor			= TRUE;
//	else
//	m_bUseArmor			= FALSE;
	if ( FStringNull( m_bUseArmor ))
	{
		if ( FClassnameIs( pev, "monster_agency_member" ) )
		m_bUseArmor			= TRUE;

		if ( IsLeader() )
		m_bUseArmor	= TRUE;
	}

	CBaseHuman::g_talkWaitTime = 0;

	//this call reset some values, let's give the new ones
	MonsterInit();
/*
	if (pev->frags == HUMAN_WEAPON_LAW)
	{
		m_flDistTooFar		= 1024.0 *4;
		m_flDistLook		= 2048.0 *4;
	}
	else if (pev->frags == HUMAN_WEAPON_AWP)
	{
		m_flDistTooFar		= 1024.0 *6;
		m_flDistLook		= 2048.0 *6;
	}
	else if (pev->frags == HUMAN_WEAPON_ASSAULT)
	{
		m_flDistTooFar		= 1024.0 *4;//2
		m_flDistLook		= 2048.0 *4;
	}
	else
	{
		//this will be overridden when the monster will deploy a pistol
		m_flDistTooFar		= 1024.0;
		m_flDistLook		= 2048.0;
	}*/

	//TODO, I know, there's no use to shoot a rifle if player isn't going to get hit
	//decreasing the values will make the NPC get closer
		m_flDistTooFar		= 1024.0 *4;
		m_flDistLook		= 2048.0 *4;

	//override frustration
	//set random frustration, we will check later if it's a terrorist or not
	if ( FStringNull( m_iBreakPoint ))
	{
		switch (RANDOM_LONG(1,4))
		{
			case 1: m_iBreakPoint = LOW_RESISTENCE; break;
			case 2: m_iBreakPoint = MED_RESISTENCE; break;
			case 3: m_iBreakPoint = HIGH_RESISTENCE; break;
			case 4: m_iBreakPoint = SUPER_RESISTENCE; break;
		}
	}

	if (!bTerrorist)
	{
	SetBodygroup( HUMAN_BODYGROUP_HEAD, RANDOM_LONG( 0, 4 ) );
	}
	else
	{
	/*	SetBodygroup( 0, RANDOM_LONG( 0, 1 ) );
		SetBodygroup( 1, RANDOM_LONG( 0, 2 ) );
		SetBodygroup( 2, RANDOM_LONG( 0, 1 ) );
		SetBodygroup( 3, RANDOM_LONG( 0, 3 ) );*/
	//	pev->skin = 	RANDOM_LONG( 0, 23 );
	}
}


//=============================================
// Center - over-ridden because of crouching
//=============================================

Vector CBaseHuman::Center()
{
	if ( m_fCrouching)
	{
		return pev->origin + Vector(0, 0, 18);
	}
	else
	{
		return pev->origin + Vector(0, 0, 36);
	}
}

//=========================================================
// StartMonster - final bit of initization before a monster 
// is turned over to the AI.  Over-ridden because base class
// calls model file to try and set weapons capabilities
//=========================================================

void CBaseHuman :: StartMonster ( void )
{
	CSquadMonster::StartMonster();

	if (bFistsReady)
	{
		m_afCapability |= bits_CAP_MELEE_ATTACK1;
		m_afCapability |= bits_CAP_MELEE_ATTACK2;
	}

	if (pev->weapons != HUMAN_WEAPON_NONE )
	{
		m_afCapability |= bits_CAP_RANGE_ATTACK1;
	}
	
	if (m_fHandGrenades)
	{
		m_afCapability |= bits_CAP_RANGE_ATTACK2;
	}
	
	if ( LookupSequence ( "frontkick" ) != ACTIVITY_NOT_AVAILABLE ) 
	{
		m_afCapability |= bits_CAP_MELEE_ATTACK1;
	}
}

//=========================================================
// Weapon Entity Name
//=========================================================

char * CBaseHuman::WeaponEntityName()//not used
{
	switch( pev->weapons )
	{
		case HUMAN_WEAPON_M249: return "weapon_m249"; break;
		case HUMAN_WEAPON_SHOTGUN: return "weapon_spas12"; break;
		case HUMAN_WEAPON_LAW: return "weapon_rpg"; break;
		case HUMAN_WEAPON_NONE:
		default: return ""; break;
	}
}


//=========================================================
// IRelationship - do I love or hate pTarget?
//=========================================================

int CBaseHuman::IRelationship( CBaseEntity *pTarget )
{
//		return R_NO;


	if ( pTarget->IsPlayer() )
		if ( m_afMemory & bits_MEMORY_PROVOKED )
			return R_HT;

//	if ( pev->weapons == HUMAN_WEAPON_LAW && pTarget->Classify() == CLASS_HELICOPTER )
//		return R_HT;
	
	if ( pTarget->m_bSurrender )
	return R_NO;

	if ( bTerrorist && (pTarget->m_bSurrender) && IsLeader())//if i'm a terrorist and the target is surrendered and im a leader
	return R_HT;

	return CBaseMonster::IRelationship( pTarget );
}


//=========================================================
// ALertSound - monster says "Freeze!"
//=========================================================

void CBaseHuman :: AlertSound( void )
{
	if ( m_hEnemy == NULL ) return;
	if ( !FOkToShout() ) return;

	if ( (bTerrorist) && (RANDOM_FLOAT ( 0 , 1 ) < 0.9 ) && ( m_szSpeechLabel == "FM_" ) )
	DetectEnviroment();
	else
	PlayLabelledSentence( "ALERT" );	
			
	CBasePlayer *pPlayer = NULL;
	pPlayer = (CBasePlayer *)UTIL_PlayerByIndex( 1 );
	if ( pPlayer )
		pPlayer->PlayActionMusic();
}

//=========================================================
// PainSound - when hit but not dead
//=========================================================

void CBaseHuman :: PainSound ( void )
{
	// Make sure he doesn't just repeatedly go uh-uh-uh-uh-uh when hit by rapid fire
	if (gpGlobals->time < m_painTime) return;
	m_painTime = gpGlobals->time + RANDOM_FLOAT(0.5, 0.75);

	PlayLabelledSentence( "PAIN" );
	SquadIssueCommand( SQUADCMD_DISTRESS ); 
}

//=========================================================
// DeathSound - on death
//=========================================================

void CBaseHuman :: DeathSound ( void )
{
	PlayLabelledSentence( "DIE" );
	SquadIssueCommand( SQUADCMD_DISTRESS ); 
}


//=========================================================
// IdleSound - chat about stuff when idle
//=========================================================

void CBaseHuman :: IdleSound ( void )
{
	// try to start a conversation, or make statement
	BOOL Spoke = TRUE;

	if (!FOkToSpeak())
		return;

	// if there is a friend nearby to speak to, play sentence, set friend's response time, return
	CBaseEntity *pEntity = FindNearestFriend( !HasConditions(bits_COND_PROVOKED) );	// include players unless I have been provoked
	
	CBaseHuman * pFriend = NULL;
	if (pEntity) pFriend = pEntity->MyHumanPointer();

	if ( pFriend != NULL && FInViewCone(pFriend) && FVisible(pFriend) )
	{
		m_hTalkTarget = pFriend;

		if ( pFriend->pev->deadflag == DEAD_NO )
		{
			if ( !(pFriend->IsMoving()) )
			{
				if ( !FBitSet( m_bitsSaid, bit_saidHello ) && ( pFriend->IsPlayer() || pFriend->IsLeader() ) ) // Friend is Commander
				{
					PlayLabelledSentence( "HELLO" );
					SetBits( m_bitsSaid, bit_saidHello );
				}
				else if ( !FBitSet( m_bitsSaid, bit_saidDamageHeavy ) &&	// Friend is heavily wounded
					pFriend->pev->health <= pFriend->pev->max_health / 8 )
				{
					PlayLabelledSentence( "HURTC" );
					SetBits( m_bitsSaid, bit_saidDamageHeavy );
				}
				else if ( !FBitSet( m_bitsSaid, bit_saidDamageMedium ) &&	// Friend is medium wounded
					pFriend->pev->health <= pFriend->pev->max_health / 4 && 
					pFriend->pev->health > pFriend->pev->max_health / 8 )
				{
					PlayLabelledSentence( "HURTB" );
					SetBits( m_bitsSaid, bit_saidDamageMedium );
				}
				else if ( !FBitSet( m_bitsSaid, bit_saidDamageLight ) &&	// Friend is lightly wounded
					pFriend->pev->health <= pFriend->pev->max_health / 2 && 
					pFriend->pev->health > pFriend->pev->max_health / 4)
				{
					PlayLabelledSentence( "HURTA" );
					SetBits( m_bitsSaid, bit_saidDamageLight );
				}
				else if ( !FBitSet(m_bitsSaid, bit_saidWoundLight ) &&	// I am lightly wounded
					(pev->health <= (2 * pev->max_health / 3)) && 
					(pev->health > (pev->max_health / 3)))
				{
					PlayLabelledSentence( "WOUND" );
					SetBits( m_bitsSaid, bit_saidWoundLight );
				}
				else if ( !FBitSet(m_bitsSaid, bit_saidWoundHeavy ) && // I am heavily wounded
					(pev->health <= (pev->max_health / 3)))
				{
					PlayLabelledSentence( "MORTAL" );
					SetBits( m_bitsSaid, bit_saidWoundHeavy );
				}
				else if ( !FBitSet( m_bitsSaid, bit_saidQuestion ) && !RANDOM_LONG(0, 4) )	// Ask a question
				{
					PlayLabelledSentence( "QUESTION" ); 
					SetBits( m_bitsSaid, bit_saidQuestion );
					
					CBaseHuman *pHuman = pFriend->MyHumanPointer();
					if (pHuman) pHuman->SetAnswerQuestion( this, GetDuration( "QUESTION" ) );
				}
				else if ( !FBitSet( m_bitsSaid, bit_saidIdle ) && !RANDOM_LONG(0, 4) )	// Say something idly
				{
					PlayLabelledSentence( "IDLE" ); 
					SetBits( m_bitsSaid, bit_saidIdle );
				}
				else if ( pFriend->IsPlayer() && !FBitSet( m_bitsSaid, bit_saidStare ) && !RANDOM_LONG(0, 4) ) // Player is staring at me
				{
					if ( ( pFriend->pev->origin - pev->origin ).Length2D() < 128 )
					{
						UTIL_MakeVectors( pFriend->pev->angles );
						if ( UTIL_DotPoints( pFriend->pev->origin, pev->origin, gpGlobals->v_forward ) > m_flFieldOfView )
						{
							PlayLabelledSentence( "STARE" ); 
							SetBits( m_bitsSaid, bit_saidStare );
						}
					}
				}
				else Spoke = FALSE;
			}
		}
		else if ( !FBitSet( m_bitsSaid, bit_saidDead ) )	// Friend is dead
		{
			PlayLabelledSentence( "DEAD" );
			SetBits( m_bitsSaid, bit_saidDead );

			if ( SafeToChangeSchedule() )
			{
				if ( m_hEnemy != NULL )
				{
					// Change schedule immediately as this could be important
					ChangeSchedule( GetScheduleOfType( SCHED_HUMAN_RETREAT ) );	
				}
				else
				{
					// Have no enemy in Alert or idle state
					ChangeSchedule( GetScheduleOfType( SCHED_TAKE_COVER_FROM_ORIGIN ) );	
				}
			}
		}
		else Spoke = FALSE;
	}
	else Spoke = FALSE;

	if ( !Spoke )
	{
		// didn't speak
		m_hTalkTarget = NULL;
	}
}

//=========================================================
// FOkToShout - is it ok for me to shout?
//=========================================================

int CBaseHuman :: FOkToShout( void )
{
	// if in the grip of a barnacle, don't speak
	if ( m_MonsterState == MONSTERSTATE_PRONE || m_IdealMonsterState == MONSTERSTATE_PRONE )
	{
		return FALSE;
	}

	// if not alive, certainly don't speak
	if ( pev->deadflag != DEAD_NO )
	{
		return FALSE;
	}

	if ( pev->deadflag == DEAD_DYING )//if are not dead dead
	{
		return FALSE;
	}

	if ( pev->spawnflags & SF_MONSTER_GAG )
	{
		return FALSE;
	}

	if ( IsTalking() )	// Don't say anything if I'm already talking
	{
		return FALSE;
	}

	// if player is not in pvs, don't speak
	if ( FNullEnt(FIND_CLIENT_IN_PVS(edict())))
		return FALSE;

	return TRUE;
}


//=========================================================
// FOkToSpeak - is it ok for me to speak in a chatty way?
//=========================================================

int CBaseHuman :: FOkToSpeak( void )
{
	// If it's not okay to shout then it's definitely not okay to speak
	if ( !FOkToShout() )
	{
		return FALSE;
	}

	// if someone else is talking, don't speak
	if (gpGlobals->time <= CBaseHuman::g_talkWaitTime)
		return FALSE;

	// If I have said something not very long ago
	if ( m_flLastTalkTime > gpGlobals->time - 10 )
	{
		return FALSE;
	}

	// If I'm in combat then I don't want to chat but can still shout
	if ( m_MonsterState == MONSTERSTATE_COMBAT )
		return FALSE;

	return TRUE;
}


//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.  Humans mostly have same
// yaw speeds, although some could be over-ridden?
//=========================================================

void CBaseHuman :: SetYawSpeed ( void )
{
    int ys;

	ys = 0;

	switch ( m_Activity )
	{
	case ACT_IDLE:	
		ys = 150;//150		
		break;
	case ACT_RUN:	
		ys = 150;	
		break;
	case ACT_WALK:	
		ys = 180;		
		break;
	case ACT_RANGE_ATTACK1:	
		ys = 120;	
		break;
	case ACT_RANGE_ATTACK2:	
		ys = 120;	
		break;
	case ACT_MELEE_ATTACK1:	
		ys = 120;	
		break;
	case ACT_MELEE_ATTACK2:	
		ys = 120;	
		break;
	case ACT_TURN_LEFT:
	case ACT_TURN_RIGHT:	
		ys = 180;//180
		break;
	case ACT_GLIDE:
	case ACT_FLY:
		ys = 30;
		break;
	default:
		ys = 90;
		break;
	}

	pev->yaw_speed = ys;
}

#define	CONE_1DEGREES	0.00873
#define	CONE_2DEGREES	0.01745
#define	CONE_3DEGREES	0.02618
#define	CONE_4DEGREES	0.03490
#define	CONE_5DEGREES	0.04362
#define	CONE_6DEGREES	0.05234
#define	CONE_7DEGREES	0.06105
#define	CONE_8DEGREES	0.06976
#define	CONE_9DEGREES	0.07846
#define	CONE_10DEGREES	0.08716
#define	CONE_15DEGREES  0.13053
#define	CONE_20DEGREES	0.17365

//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//
// Returns number of events handled, 0 if none.
//=========================================================

void CBaseHuman :: HandleAnimEvent( MonsterEvent_t *pEvent )
{
	Vector	vecGunPos;
	Vector	vecGunAngles;

	switch( pEvent->event )
	{
	case HUMAN_AE_PICKITEM:
	{
		CBaseEntity *pEntidadItem = UTIL_FindEntityByClassname(NULL, "weapon_mp5");

		if (pEntidadItem)
		{
			ClientPrint(pev, HUD_PRINTCENTER, "#AI_Removing9mmAR");

			pev->weapons = HUMAN_WEAPON_MP5;//now we have a smg

		//	SetBodygroup( HUMAN_BODYGROUP_WEAPON, HUMAN_BODY_WEAPON_SMG );
			m_cClipSize		= 30;
			m_iNumClips		= 3;//120 bullets

			if (RANDOM_LONG( 0, 99 ) < 80)
			m_fStealthed = FALSE;
			else
			m_fStealthed = TRUE;

			UTIL_Remove( pEntidadItem );
			pEntidadItem = NULL;
		}
	}
	break;

/*	case HUMAN_AE_BURST1:
	case HUMAN_AE_BURST2:
	case HUMAN_AE_BURST3:
	{
		// m_vecEnemyLKP should be center of enemy body
			Vector vecArmPos, vecArmDir;
			Vector vecDirToEnemy;
			Vector angDir;

			if (HasConditions( bits_COND_SEE_ENEMY))
			{
				vecDirToEnemy = ( ( m_vecEnemyLKP ) - pev->origin );
				angDir = UTIL_VecToAngles( vecDirToEnemy );
				vecDirToEnemy = vecDirToEnemy.Normalize();
			}
			else
			{
				angDir = pev->angles;
				UTIL_MakeAimVectors( angDir );
				vecDirToEnemy = gpGlobals->v_forward;
			}

			pev->effects = EF_MUZZLEFLASH;

			GetAttachment( 3, vecArmPos, vecArmDir );

			vecArmPos = vecArmPos + vecDirToEnemy * 0;//32

			MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, vecArmPos );
				WRITE_BYTE( TE_SPRITE );
				WRITE_COORD( vecArmPos.x );	// pos
				WRITE_COORD( vecArmPos.y );
				WRITE_COORD( vecArmPos.z );
				WRITE_SHORT( iAgruntMuzzleFlash );		// model
				WRITE_BYTE( 2 );				// size * 10
				WRITE_BYTE( 255 );			// brightness
			MESSAGE_END();
	}
	break;*/

	case HUMAN_AE_BURST1:
	{
		switch ( pev->weapons )
		{
			case	HUMAN_WEAPON_MP5:
				if( m_fStealthed )
				Fire( BULLET_MONSTER_MP5, 1, VECTOR_CONE_10DEGREES, m_iBrassShell, TE_BOUNCE_SHELL, "weapons/mp5/fire_sil-1");
				else
				Fire( BULLET_MONSTER_MP5, 1, VECTOR_CONE_15DEGREES, m_iBrassShell, TE_BOUNCE_SHELL, "weapons/mp5/fire-1");
				break;
			case	HUMAN_WEAPON_SHOTGUN:	Fire( BULLET_PLAYER_SPAS12, 10, VECTOR_CONE_20DEGREES, m_iShotgunShell, TE_BOUNCE_SHOTSHELL, "weapons/spas12/fire");break;
			case	HUMAN_WEAPON_LAW:		RPGFire();break;
			case	HUMAN_WEAPON_AWP:
					if (m_hEnemy != NULL && (m_hEnemy->IsPlayer())&& m_hEnemy->IsMoving())
					{
						break;
					}
					else
					{
						Fire( BULLET_PLAYER_M14, 1, VECTOR_CONE_PERFECT, m_iBrassShell, TE_BOUNCE_SHELL, "weapons/m14/fire"); break;
					}
				break;

			case	HUMAN_WEAPON_ASSAULT:
				if (bTerrorist)
				{
					Fire( BULLET_PLAYER_FAMAS, 1, VECTOR_CONE_15DEGREES, m_iBrassShell, TE_BOUNCE_SHELL, "weapons/ak47/fire");break;
				}
				else
				Fire( BULLET_PLAYER_FAMAS, 1, VECTOR_CONE_1DEGREES, m_iBrassShell, TE_BOUNCE_SHELL, "weapons/m4a1/fire");break;

			case	HUMAN_WEAPON_PISTOL:
				if (bTerrorist)
				{
					Fire( BULLET_MONSTER_9MM, 1, VECTOR_CONE_3DEGREES, m_iBrassShell, TE_BOUNCE_SHELL, "weapons/glock18/fire");break;
				}
				else
				Fire( BULLET_MONSTER_9MM, 1, VECTOR_CONE_2DEGREES, m_iBrassShell, TE_BOUNCE_SHELL, "weapons/1911/fire");break;

			case	HUMAN_WEAPON_DESERT:	Fire( BULLET_PLAYER_DEAGLE, 1, VECTOR_CONE_2DEGREES, m_iBrassShell, TE_BOUNCE_SHELL, "weapons/deagle/fire");break;
			case	HUMAN_WEAPON_M249:		Fire( BULLET_PLAYER_M249, 1, VECTOR_CONE_20DEGREES, m_iBrassShell, TE_BOUNCE_SHELL, "weapons/m249/fire");break;
			case	HUMAN_WEAPON_UZI:		Fire( BULLET_MONSTER_MP5, 1, VECTOR_CONE_15DEGREES, m_iBrassShell, TE_BOUNCE_SHELL, "weapons/p90/fire");break;
			case	HUMAN_WEAPON_USAS:		Fire( BULLET_MONSTER_MP5, 8, VECTOR_CONE_20DEGREES, m_iShotgunShell, TE_BOUNCE_SHOTSHELL, "weapons/usas/fire");break;
		}
		break;
		CSoundEnt::InsertSound ( bits_SOUND_COMBAT, Classify(), pev->origin, 384, 0.3 );
	}
	break;

	case HUMAN_AE_BURST2:
	case HUMAN_AE_BURST3:
	{
		switch ( pev->weapons )
		{
			case	HUMAN_WEAPON_MP5:		Fire( BULLET_MONSTER_MP5, 1, VECTOR_CONE_20DEGREES, m_iBrassShell, TE_BOUNCE_SHELL, "weapons/mp5/fire-1");break;
			case	HUMAN_WEAPON_ASSAULT:
				if (bTerrorist)
				{
					Fire( BULLET_PLAYER_FAMAS, 1, VECTOR_CONE_15DEGREES, m_iBrassShell, TE_BOUNCE_SHELL, "weapons/ak47/fire");break;
				}
				else
				Fire( BULLET_PLAYER_FAMAS, 1, VECTOR_CONE_10DEGREES, m_iBrassShell, TE_BOUNCE_SHELL, "weapons/m4a1/fire");break;
			case	HUMAN_WEAPON_M249:		Fire( BULLET_PLAYER_M249, 1, VECTOR_CONE_20DEGREES, m_iBrassShell, TE_BOUNCE_SHELL, "weapons/m249/fire");break;
			case	HUMAN_WEAPON_UZI:		Fire( BULLET_MONSTER_MP5, 1, VECTOR_CONE_20DEGREES, m_iBrassShell, TE_BOUNCE_SHELL, "weapons/p90/fire");break;
			case	HUMAN_WEAPON_USAS:		Fire( BULLET_MONSTER_MP5, 8, VECTOR_CONE_20DEGREES, m_iShotgunShell, TE_BOUNCE_SHOTSHELL, "weapons/usas/fire");break;
		}
		break;
	}
	break;

	case HUMAN_AE_RELOAD:
	{
		// Make weapon-specific reload sound
	/*	switch ( pev->weapons )
		{
			case HUMAN_WEAPON_M249:
				EMIT_SOUND( ENT(pev), CHAN_WEAPON, "weapons/m60_reload_full.wav", 1, HUMAN_ATTN );
				break;
		}*/

		GetAttachment( 3, vecGunPos, vecGunAngles );
		DropItem( "item_clip_pistol", vecGunPos, vecGunAngles );//test

		// Reload
		m_cAmmoLoaded = m_cClipSize;
		ClearConditions(bits_COND_NO_AMMO_LOADED);
	}
	break;

	case HUMAN_AE_KAMIKAZE:
	{
		ExplosionCreate( Center(), pev->angles, edict(), 100, TRUE );
		GibMonster();
	}
	break;

	case HUMAN_AE_GREN_TOSS:
	{
		UTIL_MakeVectors( pev->angles );

		if (!bPlayerVisible)
		CGrenade::FlashShootTimed( pev, GetGunPosition(), m_vecTossVelocity, 30.5 );
		else
		{
			if (RANDOM_LONG( 0, 99 ) < 30)
			{
				CGrenade::ShootTimed( pev, GetGunPosition() + Vector( RANDOM_FLOAT(-5,5), RANDOM_FLOAT(-5,5), RANDOM_FLOAT(-5,5) ), m_vecTossVelocity, 4 );
			}
			else
			{
				switch ( RANDOM_LONG( 0, 2 ) )
				{
					case 0:	CGrenade::ShootFlashbang( pev, GetGunPosition() + Vector( RANDOM_FLOAT(-5,5), RANDOM_FLOAT(-5,5), RANDOM_FLOAT(-5,5) ), m_vecTossVelocity, 4 );	break;
					case 1:	CGrenade::ShootTimed( pev, GetGunPosition() + Vector( RANDOM_FLOAT(-5,5), RANDOM_FLOAT(-5,5), RANDOM_FLOAT(-5,5) ), m_vecTossVelocity, 4 );	break;
					case 2:	CGrenade::ShootTimedSmoke( pev, GetGunPosition() + Vector( RANDOM_FLOAT(-5,5), RANDOM_FLOAT(-5,5), RANDOM_FLOAT(-5,5) ), m_vecTossVelocity, 4.0 );break;//23.0
				}
			}
		}

		m_LastAttack2Check = FALSE;
		m_flNextAttack2Check = gpGlobals->time + 6;// wait six seconds before even looking again to see if a grenade can be thrown.
	}
	break;

	case HUMAN_AE_GREN_DROP:
	{
		UTIL_MakeVectors( pev->angles );

		if (!bPlayerVisible)
		CGrenade::FlashShootTimed( pev, pev->origin + gpGlobals->v_forward * 17 - gpGlobals->v_right * 27 + gpGlobals->v_up * 6, g_vecZero, 30 );
		else
		{
			if (RANDOM_LONG( 0, 99 ) < 70) 	//sometimes blow the corpses
			{
				CGrenade::ShootTimed( pev, pev->origin + gpGlobals->v_forward * 17 - gpGlobals->v_right * 27 + gpGlobals->v_up * 6, g_vecZero, 4 );
			}
			else
			{
				switch ( RANDOM_LONG( 0, 2 ) )
				{
					case 0:	CGrenade::ShootTimed( pev, pev->origin + gpGlobals->v_forward * 17 - gpGlobals->v_right * 27 + gpGlobals->v_up * 6, g_vecZero, 4 );	break;
					case 1:	CGrenade::ShootTimed( pev, pev->origin + gpGlobals->v_forward * 17 - gpGlobals->v_right * 27 + gpGlobals->v_up * 6, g_vecZero, 4 );break;
					case 2:	CGrenade::ShootTimedSmoke( pev, pev->origin + gpGlobals->v_forward * 17 - gpGlobals->v_right * 27 + gpGlobals->v_up * 6, g_vecZero, 4);break;//23
				}
			}
		}
	}
	break;

	case HUMAN_AE_DROP_GUN:
	{
		if(!m_bSurrender)
		{
			if ( pev->weapons != HUMAN_WEAPON_NONE )
			{
				if (pev->spawnflags & SF_MONSTER_NO_WPN_DROP) break;

				GetAttachment( 3, vecGunPos, vecGunAngles );

			//	SetBodygroup( HUMAN_BODYGROUP_WEAPON, GUN_NONE );//seems like REMOVE_P_MODEL isn't enough

				REMOVE_P_MODEL;

				CBaseEntity *pItem;

				switch ( pev->weapons )
				{
					case	HUMAN_WEAPON_MP5:		pItem = DropGun( "weapon_mp5", vecGunPos, vecGunAngles ); break;
					case	HUMAN_WEAPON_SHOTGUN:	pItem = DropGun("weapon_spas12", vecGunPos, vecGunAngles); break;
					case	HUMAN_WEAPON_LAW:		pItem = DropGun("weapon_rpg", vecGunPos, vecGunAngles); break;
					case	HUMAN_WEAPON_AWP:		pItem = DropGun("weapon_m14", vecGunPos, vecGunAngles); break;
					case	HUMAN_WEAPON_ASSAULT:
						{
							if (bTerrorist)
							{
								pItem = DropGun("weapon_ak47", vecGunPos, vecGunAngles); break;
							}
							else
								pItem = DropGun("weapon_m4a1", vecGunPos, vecGunAngles); break;
						}
					case	HUMAN_WEAPON_PISTOL:
						{
							if (bTerrorist)
							{
								pItem = DropGun("weapon_glock18", vecGunPos, vecGunAngles); break;
							}
							else
								pItem = DropGun("weapon_1911", vecGunPos, vecGunAngles); break;
						}
					case	HUMAN_WEAPON_DESERT:	pItem = DropGun("weapon_deagle", vecGunPos, vecGunAngles); break;
					case	HUMAN_WEAPON_M249:		pItem = DropGun("weapon_M249", vecGunPos, vecGunAngles); break;
					case	HUMAN_WEAPON_UZI:		pItem = DropGun("weapon_p90", vecGunPos, vecGunAngles); break;
					case	HUMAN_WEAPON_USAS:		pItem = DropGun("weapon_usas", vecGunPos, vecGunAngles); break;
				}

				if ( pItem )
				{
					pItem->pev->velocity = Vector (RANDOM_FLOAT(-50,50), RANDOM_FLOAT(-50,50), RANDOM_FLOAT(-50,50));
					pItem->pev->avelocity = Vector ( RANDOM_FLOAT( -50, 50 ), RANDOM_FLOAT( -50, 50 ), RANDOM_FLOAT( -50, 50 ) );

					pev->weapons = HUMAN_WEAPON_NONE;

					CBasePlayerWeapon *pWeapon = (CBasePlayerWeapon *)pItem;
					if ( pWeapon )
						pWeapon->m_iDefaultAmmo = m_cAmmoLoaded;
				}
			}

			if (m_fHandGrenades)
				DropItem( "item_grenade", BodyTarget( pev->origin ), vecGunAngles );

			if (!m_bDropped)//we have something to drop
			{
				if (RANDOM_LONG(0,1))
				{
					pev->weapons = HUMAN_WEAPON_DESERT;//now we have a desert eagle
					m_cClipSize = 7;//edit, change the clip size and give ammo

					m_fHasDeagle = TRUE;
				}
				else
				{
					pev->weapons = HUMAN_WEAPON_PISTOL;//now we have a pistol
					m_cClipSize = 20;//edit, change the clip size and give ammo
				}

				m_flDistTooFar		= 512.0;
				m_flDistLook		= 1024.0;

				m_iNumClips = 999;//at this point the monster doens't have any clips, give it a few of them
				m_cAmmoLoaded = m_cClipSize;
			}
		}
	}
	break;

	case HUMAN_AE_KICK:
	{
		CBaseEntity *pHurt = Kick();

		if ( pHurt )
		{
			EMIT_SOUND( ENT(pev), CHAN_WEAPON, "zombie/claw_miss2.wav", 1, HUMAN_ATTN );
			UTIL_MakeVectors( pev->angles );

			if(bFistsReady)
			{
				pHurt->pev->punchangle.x = -25;
				pHurt->pev->punchangle.y = 0;

			//	pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_forward * 100 + gpGlobals->v_up * 50;
				pHurt->TakeDamage( pev, pev, 20, DMG_CLUB );
			}
			else
			{
				pHurt->pev->punchangle.x = -25;
				pHurt->pev->punchangle.y = -25;
			//	pHurt->pev->punchangle.z = 15;

				pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_forward * 100 + gpGlobals->v_up * 50;
				pHurt->TakeDamage( pev, pev, gSkillData.hgruntDmgKick, DMG_CLUB );
			}

		/*	CBasePlayer *m_pPlayer = GetClassPtr((CBasePlayer *)pev);

			if ( ( m_pPlayer->pev->weapons & (1<<WEAPON_SUIT) ) && (m_pPlayer->m_pActiveItem->m_iId != NULL) )//FIX
			{
				switch ( m_pPlayer->m_pActiveItem->m_iId)
				{
					case  WEAPON_CROWBAR:
					//do nothing
					break;//1911

					default:
						SERVER_COMMAND("drop\n");
						SERVER_COMMAND("weapon_knife\n");
					break;
				}
			}*/
		}
	}
	break;

	case HUMAN_AE_DRAW:
	{
		switch ( pev->weapons )
		{
			case HUMAN_WEAPON_DESERT:
			case HUMAN_WEAPON_PISTOL:
			{
				SET_P_MODEL;

				if(m_fHasDeagle)
			//	SetBodygroup( HUMAN_BODYGROUP_WEAPON, HUMAN_BODY_WEAPON_DEAGLE );
				SET_WEAPON(P_WEAPON_DEAGLE);
				else
				{
			//	SetBodygroup( HUMAN_BODYGROUP_WEAPON, HUMAN_BODY_WEAPON_PISTOL );
					if (bTerrorist)
					SET_WEAPON(P_WEAPON_GLOCK);
					else
					SET_WEAPON(P_WEAPON_1911);
				}

				EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "weapons/draw_smg.wav", 1, ATTN_NORM, 0, 100 );
				m_fGunDrawn = TRUE;
			}
			break;
		}
	}
	break;

	default:
		CSquadMonster::HandleAnimEvent( pEvent );
	}
}

//=========================================================
// Get Voice Pitch
//=========================================================
int CBaseHuman :: GetVoicePitch( void )
{
	return m_voicePitch;
}

//=========================================================
// GetDeathActivity - determines the best type of death
// anim to play.
// SysOp edit: I've added S2P Physics here. Whit this you can
// show the best type of death (no more clipping on walls)
//=========================================================
Activity CBaseHuman :: GetDeathActivity ( void )
{
	Activity	deathActivity;
	BOOL		fTriedDirection;
	float		flDot;
	TraceResult	tr;
	Vector		vecSrc;

	if ( pev->deadflag != DEAD_NO )
	return m_IdealActivity;

	vecSrc = Center();

	fTriedDirection = FALSE;
	deathActivity = ACT_DIESIMPLE;// in case we can't find any special deaths to do.

	UTIL_MakeVectors ( pev->angles );
	flDot = DotProduct ( gpGlobals->v_forward, g_vecAttackDir * -1 );

	if ( FClassnameIs( pev, "monster_cine_dead_crouch" ) )
	{
		m_fCrouching = true;
	}

	switch ( m_LastHitGroup )
	{
	case HITGROUP_HEAD:
		{
			UTIL_TraceHull ( vecSrc, vecSrc + gpGlobals->v_forward * -32, dont_ignore_monsters, head_hull, edict(), &tr );

			if ( tr.flFraction != 1.0 )
			{
				deathActivity = ACT_DIEFORWARD;
				ALERT( at_console, ">NO ROOM TO FALL BACKWARD!\n");
			}
			else
			{
				iSlowmocounter = 20;

				if( bViolentHeadDamaged )
				{
					deathActivity =	ACT_DIE_HEADSHOT_VIOLENT;

//					if( bSlow )
//					{
	/*					Vector vecLookerOrigin = pev->origin + pev->view_ofs;//look through the caller's 'eyes'

						MESSAGE_BEGIN(MSG_ALL, gmsgParticles);
							WRITE_SHORT(0);
							WRITE_BYTE(0);
							WRITE_COORD( vecLookerOrigin.x );
							WRITE_COORD( vecLookerOrigin.y );
							WRITE_COORD( vecLookerOrigin.z );
							WRITE_COORD( 0 );
							WRITE_COORD( 0 );
							WRITE_COORD( 0 );
							WRITE_SHORT(iDefaultBloodSlowCore);
						MESSAGE_END();	
						
						MESSAGE_BEGIN(MSG_ALL, gmsgParticles);
							WRITE_SHORT(0);
							WRITE_BYTE(0);
							WRITE_COORD( vecLookerOrigin.x );
							WRITE_COORD( vecLookerOrigin.y );
							WRITE_COORD( vecLookerOrigin.z );
							WRITE_COORD( 0 );
							WRITE_COORD( 0 );
							WRITE_COORD( 0 );
							WRITE_SHORT(iDefaultBloodSlowDrop);
						MESSAGE_END();
//					}*/
				}
				else
				deathActivity = ACT_DIE_HEADSHOT;
			}
		}
		break;

	case HITGROUP_STOMACH:
		{
			UTIL_TraceHull ( vecSrc, vecSrc + gpGlobals->v_forward * -32, dont_ignore_monsters, head_hull, edict(), &tr );

			if ( tr.flFraction != 1.0 )
			{
				deathActivity = ACT_DIEFORWARD;
				ALERT( at_console, ">NO ROOM TO FALL BACKWARD!\n");
			}
			else
			{
				deathActivity = ACT_DIE_GUTSHOT;
			}
		}
		break;

	case HITGROUP_GENERIC:

		fTriedDirection = TRUE;

		if ( flDot > 0.3 )
		{
			deathActivity = ACT_DIEFORWARD;
		}
		else if ( flDot <= -0.3 )
		{
			deathActivity = ACT_DIEBACKWARD;
		}
		break;

	default:

		fTriedDirection = TRUE;

		if ( flDot > 0.3 )
		{
			deathActivity = ACT_DIEFORWARD;
		}
		else if ( flDot <= -0.3 )
		{
			deathActivity = ACT_DIEBACKWARD;
		}
		break;
	}
/*
	if ( LookupActivity ( deathActivity ) == ACTIVITY_NOT_AVAILABLE )
	{
		if ( fTriedDirection )
		deathActivity = ACT_DIESIMPLE;
		else
		{
			if ( flDot > 0.3 )
			deathActivity = ACT_DIEFORWARD;
			else if ( flDot <= -0.3 )
			deathActivity = ACT_DIEBACKWARD;
		}
	}

	if ( LookupActivity ( deathActivity ) == ACTIVITY_NOT_AVAILABLE )
	deathActivity = ACT_DIESIMPLE;
*/
	if ( deathActivity == ACT_DIEFORWARD )
	{
		UTIL_TraceHull ( vecSrc, vecSrc + gpGlobals->v_forward * 64, dont_ignore_monsters, head_hull, edict(), &tr );

		if ( tr.flFraction != 1.0 )
		{
			deathActivity = ACT_DIEBACKWARD;//backwards? -ACT_DIESIMPLE
			ALERT( at_console, ">NO ROOM TO FALL FORWARD!\n");
		}
	}

	if ( deathActivity == ACT_DIEBACKWARD )
	{
		UTIL_TraceHull ( vecSrc, vecSrc + gpGlobals->v_forward * -32, dont_ignore_monsters, head_hull, edict(), &tr );

		if ( tr.flFraction != 1.0 )
		{
			deathActivity = ACT_DIE_COLLIDE;
		}
		else
		{
			UTIL_TraceHull ( vecSrc, vecSrc + gpGlobals->v_forward * -64, dont_ignore_monsters, head_hull, edict(), &tr );

			if ( tr.flFraction != 1.0 )
			{
				deathActivity = ACT_DIEFORWARD;
				ALERT( at_console, ">NO ROOM TO FALL BACKWARD!\n");
			}
		}
	}
			
//	if ( !m_bWakeUp )
//	deathActivity = ACT_DIE_SLEEPING;

	ALERT( at_console, "deathActivity = %i\n",deathActivity);
    
	return deathActivity;
}


//=========================================================
// GetSmallFlinchActivity - determines the best type of flinch
// anim to play.
//=========================================================
Activity CBaseHuman :: GetSmallFlinchActivity ( void )
{
	Activity	flinchActivity;

	switch ( m_LastHitGroup )
	{
		// pick a region-specific flinch
		//new
	case HITGROUP_HEAD:
		flinchActivity = ACT_FLINCH_HEAD;
		break;

	case HITGROUP_CHEST:
		flinchActivity = ACT_FLINCH_CHEST;
		break;
	case HITGROUP_STOMACH:
		flinchActivity = ACT_FLINCH_GUT;
		break;

	case HITGROUP_LEFTARM:
		flinchActivity = ACT_FLINCH_LEFTARM;
		break;
	case HITGROUP_RIGHTARM:
		flinchActivity = ACT_FLINCH_RIGHTARM;
		break;
	case HITGROUP_LEFTLEG:
		flinchActivity = ACT_FLINCH_LEFTLEG;
		break;
	case HITGROUP_RIGHTLEG:
		flinchActivity = ACT_FLINCH_RIGHTLEG;
		break;
	default:
		// just get a generic flinch.
		flinchActivity = ACT_SMALL_FLINCH;
		break;
	}

	return flinchActivity;
}

//=========================================================
// SetTurnActivity - measures the difference between the way
// the monster is facing and determines whether or not to
// select one of the 180 turn animations.
//=========================================================

void CBaseHuman :: SetTurnActivity ( void )
{
	float flYD;
	flYD = FlYawDiff();

	if ( pev->movetype == MOVETYPE_FLY ) 
	{
		return;
	}
	//until crouch animations are done

	if ( flYD <= -45 )
	{// big right turn
		m_IdealActivity = ACT_TURN_RIGHT;
	}
	else if ( flYD > 45 )
	{// big left turn
		m_IdealActivity = ACT_TURN_LEFT;
	}
}

//=========================================================
// SetActivity - totally overrides basemonster SetActivities
// because of the necessity to pick a crouch animation etc
//
// If you want an activity to have several "modes", i.e. 
// crouched and standing, don't give it an ACT_ title
//=========================================================

// Sys NOTE: 
// This code will look for a ACT_WHATEVER first. If can't find any ACT_
// the code will try to perform animations by name. Some of them are needed, just like "crouch" "uncrouch"
// Any animation can be replaced by adding "crouch_" before animation's name. It'll be used when
// the monster are crouched, of course.

// I'd like to add those ACT_ in .qc file, but I can't compile if I do that. I need to modify "studiomdl"
// to make it work, so in the meanwhile let's use this code.

void CBaseHuman :: SetActivity ( Activity NewActivity )
{
	int	iSequence = ACTIVITY_NOT_AVAILABLE;
	void *pmodel = GET_MODEL_PTR( ENT(pev) );
	char seq[40];

// First try just looking up the activity normally
	iSequence = LookupActivity ( NewActivity );

	// If that doesn't work, look up a named activity
	if (iSequence == ACTIVITY_NOT_AVAILABLE) 
	switch ( NewActivity)
	{
		case ACT_RANGE_ATTACK1:
			
	//		if (RANDOM_LONG( 0, 99 ) > 80)//sometimes dont shoot
	//		{	
	//			CBaseEntity *pPlayer = UTIL_PlayerByIndex( 1 );
	//		
	//			if(pPlayer->pev->health >25)
	//			{
					switch ( pev->weapons )
					{
						//T/AG weapons
						case	HUMAN_WEAPON_MP5:		strcpy( seq, "shoot_mp5" );	break;
						case	HUMAN_WEAPON_SHOTGUN:	strcpy( seq, "shoot_shotgun" );break;
						case	HUMAN_WEAPON_LAW:		strcpy( seq, "shoot_law" );	break;
						case	HUMAN_WEAPON_AWP:		strcpy( seq, "shoot_sniper" );	break;
						case	HUMAN_WEAPON_ASSAULT:	strcpy( seq, "shoot_rifle" );	break;
						case	HUMAN_WEAPON_PISTOL:	strcpy( seq, "shoot_pistol" );	break;
						case	HUMAN_WEAPON_DESERT:	strcpy( seq, "shoot_desert" );	break;
						case	HUMAN_WEAPON_M249:		strcpy( seq, "shoot_m249" );	break;

						//HL2 npc weapons
						case	HUMAN_WEAPON_MP7:		strcpy( seq, "shoot_mp7" );	break;
						case	HUMAN_WEAPON_IRGUN:		strcpy( seq, "shoot_irgun" );	break;
						case	HUMAN_WEAPON_357:		strcpy( seq, "shoot_357" );	break;
						case	HUMAN_WEAPON_USP:		strcpy( seq, "shoot_pistol" );	break;
						case	HUMAN_WEAPON_UZI:		strcpy( seq, "shoot_mp5" );	break;
						case	HUMAN_WEAPON_USAS:		strcpy( seq, "shoot_usas" );break;
					}
	//			}
	//		}
		break;
	
		case ACT_RANGE_ATTACK2:
			strcpy( seq, "throwgrenade");
			break;

		case ACT_SPECIAL_ATTACK1:
			strcpy( seq, "drop_grenade" );
			break;
	
		case ACT_MELEE_ATTACK1:
			iSequence = LookupSequence ( "frontkick" );
					
			if (m_bKamikaze)
			iSequence = LookupSequence( "fire_he" );

			if ( bFistsReady )
			{
				switch (RANDOM_LONG(1,3))
				{
					case 1:	iSequence = LookupSequence ( "fists_1" ); break;
					case 2:	iSequence = LookupSequence ( "fists_2" ); break;
					case 3:	iSequence = LookupSequence ( "fists_3" ); break;
				}
				break;
			}
		break;

		case ACT_CROUCH:
			iSequence = LookupSequence ( "crouch" );
			break;

		case ACT_UNCROUCH:
			iSequence = LookupSequence ( "uncrouch" );
			break;

		case ACT_COWER:
			strcpy( seq, "cower" );
			break;

		case ACT_SIGNAL1:
			strcpy( seq, "signal_forward");
					
			if(m_fCrouching)
			strcpy( seq, "signal_advance");
			break;

			break;

		case ACT_SIGNAL2:
			strcpy( seq, "signal_advance");
					
			if(m_fCrouching)
			strcpy( seq, "signal_advance");
			break;

		case ACT_SIGNAL3:
			strcpy( seq, "signal_takecover");
					
			if(m_fCrouching)
			strcpy( seq, "signal_advance");
			break;
				
		case ACT_DROP_GUN:
			iSequence = LookupSequence ( "drop_weapon" );
			break;

		case ACT_IDLE:
			{		
//				ALERT( at_console, "ACT_IDLE!\n");

		//		if( basd == FALSE )
				if ( m_MonsterState == MONSTERSTATE_COMBAT )
				{
//					ALERT( at_console, "MONSTERSTATE_COMBAT!\n");

					strcpy( seq, "combatidle" );//add combatidle

					switch ( pev->weapons )//add combatidle plus "_mp5" for example. combatidle_mp5
					{
						//T/AG weapons
						case	HUMAN_WEAPON_MP5:		strcat( seq, "_mp5" );	break;
						case	HUMAN_WEAPON_SHOTGUN:	strcat( seq, "_shotgun" );	break;
						case	HUMAN_WEAPON_LAW:		strcat( seq, "_law" );	break;
						case	HUMAN_WEAPON_AWP:		strcat( seq, "_sniper" );	break;
						case	HUMAN_WEAPON_ASSAULT:	strcat( seq, "_rifle" );	break;
						case	HUMAN_WEAPON_PISTOL:	strcat( seq, "_pistol" );	break;
						case	HUMAN_WEAPON_DESERT:	strcat( seq, "_desert" );	break;
						case	HUMAN_WEAPON_M249:		strcat( seq, "_m249" );	break;

						//HL2 npc weapons
						case	HUMAN_WEAPON_MP7:		strcat( seq, "_mp7" );	break;
						case	HUMAN_WEAPON_IRGUN:		strcat( seq, "_irgun" );	break;
						case	HUMAN_WEAPON_357:		strcat( seq, "_357" );	break;
						case	HUMAN_WEAPON_USP:		strcat( seq, "_pistol" );	break;
										
						case	HUMAN_WEAPON_UZI:		strcat( seq, "_mp5" );	break;
						case	HUMAN_WEAPON_USAS:		strcat( seq, "_usas" );	break;
					}
										
					//GROSS HACK: FOR SOME REASON THE MONSTER_STATE ISN'T CLEARED AFTER MONSTER SURRENDERS
					//TRY TO SET ANIMATIONS HERE IF SO!
					if(m_bSurrender)
					{
						if (m_bSurrenderStanding)
						iSequence = LookupSequence( "surrender_idle" );
						else
						iSequence = LookupSequence( "surrender_idle2" );
					}

					break;
				}
				else
				{
	//				if ( !m_bWakeUp )
	//				iSequence = LookupSequence( "sleep2" );
	//				else
	//				{

					if (!FStringNull (m_szIdleAnimation) )
					{
						if (RANDOM_LONG(0,1))
						strcpy( seq, STRING(m_szIdleAnimation) );
						else
						strcpy( seq, STRING(m_szIdleAnimation) );

						switch ( pev->weapons )
						{
							case	HUMAN_WEAPON_DESERT:
							case	HUMAN_WEAPON_PISTOL:
							case	HUMAN_WEAPON_357:
							case	HUMAN_WEAPON_USP:	
							iSequence = LookupSequence (STRING( m_szIdleAnimation) );
							break;
						}
					}
					else
					{
									if (RANDOM_LONG(0,1))
						strcpy( seq, "idle1" );
						else
						strcpy( seq, "idle2" );

						switch ( pev->weapons )
						{
							case	HUMAN_WEAPON_DESERT:
							case	HUMAN_WEAPON_PISTOL:
							case	HUMAN_WEAPON_357:
							case	HUMAN_WEAPON_USP:	
							iSequence = LookupSequence ( "idle1_pistol" );
							break;
						}
					}

					if(m_bSurrender)
					{
						if (m_bSurrenderStanding)
						iSequence = LookupSequence( "surrender_idle" );
						else
						iSequence = LookupSequence( "surrender_idle2" );
					}
								
					if (bOnFire)
					{
						iSequence = LookupSequence ( "idle_fire" );
					}

				}
							
				if ( bFistsReady )
				iSequence = LookupSequence( "fists_idle" );
			}
			break;

		case ACT_COMBAT_IDLE:
		case ACT_IDLE_ANGRY:
		{
			ALERT( at_console, "ACT_COMBAT_IDLE! ACT_IDLE_ANGRY\n");

			strcpy( seq, "combatidle" );//add combatidle

			switch ( pev->weapons )//add combatidle plus "_mp5" for example. combatidle_mp5
			{
				//T/AG weapons
				case	HUMAN_WEAPON_MP5:		strcat( seq, "_mp5" );	break;
				case	HUMAN_WEAPON_SHOTGUN:	strcat( seq, "_shotgun" );	break;
				case	HUMAN_WEAPON_LAW:		strcat( seq, "_law" );	break;
				case	HUMAN_WEAPON_AWP:		strcat( seq, "_sniper" );	break;
				case	HUMAN_WEAPON_ASSAULT:	strcat( seq, "_rifle" );	break;
				case	HUMAN_WEAPON_PISTOL:	strcat( seq, "_pistol" );	break;
				case	HUMAN_WEAPON_DESERT:	strcat( seq, "_desert" );	break;
				case	HUMAN_WEAPON_M249:		strcat( seq, "_m249" );	break;

				//HL2 npc weapons
				case	HUMAN_WEAPON_MP7:		strcat( seq, "_mp7" );	break;
				case	HUMAN_WEAPON_IRGUN:		strcat( seq, "_irgun" );	break;
				case	HUMAN_WEAPON_357:		strcat( seq, "_357" );	break;
				case	HUMAN_WEAPON_USP:		strcat( seq, "_pistol" );	break;
								
				case	HUMAN_WEAPON_UZI:		strcat( seq, "_mp5" );	break;
				case	HUMAN_WEAPON_USAS:		strcat( seq, "_usas" );	break;
			}
			break;
				
			if ( bFistsReady )
			iSequence = LookupSequence( "fists_idle" );
		}
		break;
			
		case ACT_WALK:
			
			if ( pev->health >= pev->max_health / 3 )
			{
				strcpy( seq, "walk" );
			}
			else
			{
				iSequence = LookupActivity ( ACT_WALK_HURT );// 		iSequence = LookupActivity ( ACT_WALK_HURT ); ??????

			
				if(m_fCrouching)
				iSequence = LookupSequence ( "crouch_walk" );
			}

			switch ( pev->weapons )
			{
				case	HUMAN_WEAPON_DESERT:
				case	HUMAN_WEAPON_PISTOL:
				case	HUMAN_WEAPON_357:
				case	HUMAN_WEAPON_USP:	
				iSequence = LookupSequence ( "walk_pistol" );
				break;
			}
			
			if (m_bKamikaze)
			iSequence = LookupSequence( "run_he" );
			
			if ( bFistsReady )
			iSequence = LookupSequence( "walk_pistol" );

		break;


		case ACT_RUN:

			CSound *pSound;
			pSound = PBestSound();

			ASSERT( pSound != NULL );

			switch ( pev->weapons )
			{
				//T/AG weapons
				case	HUMAN_WEAPON_MP5:
				case	HUMAN_WEAPON_ASSAULT:
				case	HUMAN_WEAPON_M249:
				case	HUMAN_WEAPON_UZI:

//				if ( (m_MonsterState == MONSTERSTATE_COMBAT) && (!HasConditions( bits_COND_NO_AMMO_LOADED )) /*&& (!HasConditions(bits_COND_ENEMY_OCCLUDED)) && (NoFriendlyFire())*/ && (!m_fCrouching) )
			/*	if ( (m_MonsterState == MONSTERSTATE_COMBAT) && (!HasConditions( bits_COND_NO_AMMO_LOADED )) && (m_hEnemy != NULL) && (FVisible( m_hEnemy )) && (!m_fCrouching) )
				iSequence = LookupSequence( "run_shooting_rifle" );
				else
				{*/
					if( _STRAFE_CODE_TEST )
					{
						iSequence = LookupSequence( "run_rifle_N" );
						break;
					}

					if(	m_fCrouching )
					iSequence = LookupSequence( "crouch_run" );
					else if ( pSound && (pSound->m_iType & bits_SOUND_DANGER) )
					iSequence = LookupActivity ( ACT_WALK_HURT ); //doesn't work
					else
					iSequence = LookupSequence( "run" );
			//	}

				break;

				case	HUMAN_WEAPON_SHOTGUN:
				case	HUMAN_WEAPON_LAW:
				case	HUMAN_WEAPON_AWP:
				case	HUMAN_WEAPON_USAS:

					if( _STRAFE_CODE_TEST )
					{
						iSequence = LookupSequence( "run_rifle_N" );
						break;
					}

					if(	m_fCrouching )
					iSequence = LookupSequence( "crouch_run" );
					else if ( pSound && pSound->FIsSound() )//walk slow
					iSequence = LookupActivity ( ACT_WALK_HURT ); 
					else
					iSequence = LookupSequence( "run" );

				break;

				case	HUMAN_WEAPON_PISTOL:
				case	HUMAN_WEAPON_DESERT:

					if( _STRAFE_CODE_TEST )
					{
						iSequence = LookupSequence( "run_rifle_N" );
						break;
					}
			/*	if ( (m_MonsterState == MONSTERSTATE_COMBAT) && (!HasConditions( bits_COND_NO_AMMO_LOADED )) && (m_hEnemy != NULL) && (FVisible( m_hEnemy )) && (!m_fCrouching) && (m_fGunDrawn) )
				iSequence = LookupSequence( "run_shooting_pistol" );
				else
				{*/					
					if(	m_fCrouching )//stupid check because this ent can't crouch if has a pistol
					iSequence = LookupSequence( "run_pistol" );
					else
					iSequence = LookupSequence( "run_pistol" );
			//	}
				break;
			}

			if (m_bKamikaze)
			iSequence = LookupSequence( "run_he" );
			
			if ( bFistsReady )
			iSequence = LookupSequence( "run_pistol" );

			if (m_bSurrender)
				iSequence = LookupSequence("run_pistol");

			break;

			//this is not used
		case ACT_WALK_HURT:
			strcpy( seq, "walk_hurt" );
			break;

		case ACT_RUN_HURT:
			strcpy( seq, "run_hurt" );
			break;
			//this is not used

		case ACT_RELOAD:
		{
			if (pev->weapons == HUMAN_WEAPON_PISTOL || HUMAN_WEAPON_DESERT )//pistols
			EMIT_SOUND( ENT(pev), CHAN_WEAPON, "weapons/SP0001.wav", 1, HUMAN_ATTN );
			else
			EMIT_SOUND( ENT(pev), CHAN_WEAPON, "weapons/SP0000.wav", 1, HUMAN_ATTN );

			strcpy( seq, "reload" );
			m_iNumClips--;//less clip!

			switch ( pev->weapons )//add reload plus "_mp5" for example. combatidle_mp5
			{
				//T/AG weapons
				case	HUMAN_WEAPON_MP5:		strcat( seq, "_mp5" );	break;
				case	HUMAN_WEAPON_SHOTGUN:	strcat( seq, "_shotgun" );	break;
				case	HUMAN_WEAPON_LAW:		strcat( seq, "_mp5" );	break;//law????
				case	HUMAN_WEAPON_AWP:		strcat( seq, "_sniper" );	break;
				case	HUMAN_WEAPON_ASSAULT:	strcat( seq, "_rifle" );	break;
				case	HUMAN_WEAPON_PISTOL:	strcat( seq, "_pistol" );	break;
				case	HUMAN_WEAPON_DESERT:	strcat( seq, "_desert" );	break;
				case	HUMAN_WEAPON_M249:		strcat( seq, "_m249" );	break;

				//HL2 npc weapons
				case	HUMAN_WEAPON_MP7:		strcat( seq, "_mp7" );	break;
				case	HUMAN_WEAPON_IRGUN:		strcat( seq, "_irgun" );	break;
				case	HUMAN_WEAPON_357:		strcat( seq, "_357" );	break;
				case	HUMAN_WEAPON_USP:		strcat( seq, "_pistol" );	break;

				case	HUMAN_WEAPON_UZI:		strcat( seq, "_mp5" );	break;
				case	HUMAN_WEAPON_USAS:		strcat( seq, "_usas" );	break;
			}
		}
		break;

		case ACT_SURRENDER:
					
		//	PlaySentence( "T_SURR", 4, VOL_NORM, ATTN_NORM, 100 );
			PlayLabelledSentence( "SURR" );

			if (m_bSurrenderStanding)
			{
				if (pev->weapons == HUMAN_WEAPON_PISTOL || HUMAN_WEAPON_DESERT )//pistols
				{
					if (RANDOM_LONG(0,1))
					iSequence = LookupSequence( "surrender_pistol" );
					else
					iSequence = LookupSequence( "surrender_pistol2" );
				}
				else
				{
					iSequence = LookupSequence( "surrender_smg" );
				}
			}
			else
			{
				iSequence = LookupSequence ( "surrender_all" );
			}
		
			break;
			
		case ACT_SIDESTEP_RIGHT:

			if( m_bUseSideJump )
			iSequence = LookupSequence( "sidejump_left" );
			else
			iSequence = LookupSequence( "sidestep_right" );

			break;
	
		case ACT_SIDESTEP_LEFT:	

			if( m_bUseSideJump )
			iSequence = LookupSequence( "sidejump_right" );
			else
			iSequence = LookupSequence( "sidestep_left" );

			break;
		
		case ACT_WAKEUP:
			iSequence = LookupSequence ( "sleep_wakeup" );
			break;
	
		case ACT_PICK_ITEM:
			{
				iSequence = LookupSequence ( "pickup_smg" );
						
				TaskComplete();//chain events has been completed ATM
			}
			break;
			
//turn animations	
			
		//determined by QC
		/*
		case ACT_TURN_LEFT:
			iSequence = LookupSequence ( "turn_left" );
			break;

		case ACT_TURN_RIGHT:
			iSequence = LookupSequence ( "turn_right" );
			break;
*/
//flinch animations
			
		case ACT_FLINCH_HEAD:
			strcpy( seq, "flinchhead" );
					
			if(m_fCrouching)
			strcpy( seq, "flinch" );
			
			if ( bFistsReady )
			switch (RANDOM_LONG(1,2))
			{
				case 1:iSequence = LookupSequence ( "fists_flinch" ); break;
				case 2:	iSequence = LookupSequence ( "fists_flinch2" ); break;
			}
			break;

		case ACT_FLINCH_CHEST:
			if ( RANDOM_LONG(0,1) )
			iSequence = LookupSequence ( "flinchfalsedead" );
			else
			strcpy( seq, "flinchchest" );
			
			if(m_fCrouching)
			strcpy( seq, "flinch" );
				if ( bFistsReady )
			switch (RANDOM_LONG(1,2))
			{
				case 1:iSequence = LookupSequence ( "fists_flinch" ); break;
				case 2:	iSequence = LookupSequence ( "fists_flinch2" ); break;
			}
			break;

		case ACT_FLINCH_GUT:
			if ( RANDOM_LONG(0,1) )
			iSequence = LookupSequence ( "flinchfalsedead" );
			else
			strcpy( seq, "flinchgut" );
					
			if(m_fCrouching)
			strcpy( seq, "flinch" );
				if ( bFistsReady )
			switch (RANDOM_LONG(1,2))
			{
				case 1:iSequence = LookupSequence ( "fists_flinch" ); break;
				case 2:	iSequence = LookupSequence ( "fists_flinch2" ); break;
			}
			break;
				
		case ACT_FLINCH_LEFTLEG:
		case ACT_FLINCH_LEFTARM:
			if ( RANDOM_LONG(0,1) )
			iSequence = LookupSequence ( "flinchfalsedead" );
			else
			strcpy( seq, "flinchleft" );
					
			if(m_fCrouching)
			strcpy( seq, "flinch" );
				if ( bFistsReady )
			switch (RANDOM_LONG(1,2))
			{
				case 1:iSequence = LookupSequence ( "fists_flinch" ); break;
				case 2:	iSequence = LookupSequence ( "fists_flinch2" ); break;
			}
			break;

		case ACT_FLINCH_RIGHTLEG:
		case ACT_FLINCH_RIGHTARM:
			if ( RANDOM_LONG(0,1) )
			iSequence = LookupSequence ( "flinchfalsedead" );
			else
			strcpy( seq, "flinchright" );
					
			if(m_fCrouching)
			strcpy( seq, "flinch" );
				if ( bFistsReady )
			switch (RANDOM_LONG(1,2))
			{
				case 1:iSequence = LookupSequence ( "fists_flinch" ); break;
				case 2:	iSequence = LookupSequence ( "fists_flinch2" ); break;
			}
			break;
 
		case ACT_SMALL_FLINCH:
			if ( RANDOM_LONG(0,1) )
			strcpy( seq, "flinchsmall" );
			else
			strcpy( seq, "flinchsmall" );
					
			if(m_fCrouching)
			strcpy( seq, "flinch" );
		
			if ( bFistsReady )
			switch (RANDOM_LONG(1,2))
			{
				case 1:iSequence = LookupSequence ( "fists_flinch" ); break;
				case 2:	iSequence = LookupSequence ( "fists_flinch2" ); break;
			}

		break;
			/*
		case ACT_FLINCH_HEAD:
		case ACT_FLINCH_CHEST:
		case ACT_FLINCH_GUT:
		case ACT_FLINCH_LEFTLEG:
		case ACT_FLINCH_LEFTARM:
		case ACT_FLINCH_RIGHTLEG:
		case ACT_FLINCH_RIGHTARM:
		case ACT_SMALL_FLINCH:
			iSequence = LookupSequence ( "flinchfalsedead" );
			break;*/

//victory animations
		case ACT_VICTORY_DANCE:
			iSequence = LookupActivity ( NewActivity );	
			break;
//death animations		
		case ACT_DIE_HEADSHOT_VIOLENT:
			switch (RANDOM_LONG(1,5))
			{
				case 1:	strcpy( seq, "headshot_violent1" ); break;
				case 2:	strcpy( seq, "headshot_violent2" ); break;
				case 3:	strcpy( seq, "headshot_violent3" ); break;
				case 4:	strcpy( seq, "headshot_violent4" ); break;
				case 5:	strcpy( seq, "headshot_violent5" ); break;
			}
						
			if(m_fCrouching)
			strcpy( seq, "die_headshot_funny" ); break;	
					
			if (m_bSurrender)
			switch (RANDOM_LONG(1,2))
			{
				case 1:	strcpy( seq, "surrender_headshot_violent1" ); break;
				case 2:	strcpy( seq, "surrender_headshot_violent2" ); break;
			}
		break;

		case ACT_DIE_SLEEPING:
			iSequence = LookupSequence ( "sleep_die" );
		break;

		case ACT_DIE_COLLIDE:
			switch (RANDOM_LONG(1,5))
			{
				case 1:	strcpy( seq, "collide1" ); break;
				case 2:	strcpy( seq, "collide2" ); break;
				case 3:	strcpy( seq, "collide3" ); break;
				case 4:	strcpy( seq, "collide4" ); break;
				case 5:	strcpy( seq, "collide5" ); break;
			}

			if(m_fCrouching)
			strcpy( seq, "die_simple" );
		break;
		
		case ACT_DIE_HEADSHOT:
			switch (RANDOM_LONG(1,5))
			{
				case 1:	strcpy( seq, "headshot1" ); break;
				case 2:	strcpy( seq, "headshot2" ); break;
				case 3:	strcpy( seq, "headshot3" ); break;
				case 4:	strcpy( seq, "headshot4" ); break;
				case 5:	strcpy( seq, "headshot5" ); break;
			}

			if(m_fCrouching)
			switch (RANDOM_LONG(1,4))
			{
				case 1:	strcpy( seq, "die_headshot1" ); break;
				case 2:	strcpy( seq, "die_headshot2" ); break;
				case 3:	strcpy( seq, "die_headshot3" ); break;
				case 4:	strcpy( seq, "die_headshot4" ); break;
			}
				
			if (m_bSurrender && !m_bSurrenderStanding)
			switch (RANDOM_LONG(1,3))
			{
				case 1:	strcpy( seq, "surrender_headshot1" ); break;
				case 2:	strcpy( seq, "surrender_headshot2" ); break;
				case 3:	strcpy( seq, "surrender_headshot3" ); break;
			}
		break;

		case ACT_DIEFORWARD:
			switch (RANDOM_LONG(1,5))
			{
				case 1:	strcpy( seq, "forward1" ); break;
				case 2:	strcpy( seq, "forward2" ); break;
				case 3:	strcpy( seq, "forward3" ); break;
				case 4:	strcpy( seq, "forward4" ); break;
				case 5:	strcpy( seq, "forward5" ); break;
			}
			
			if(m_fCrouching)
			switch (RANDOM_LONG(1,4))
			{
				case 1:	strcpy( seq, "die_forward1" ); break;
				case 2:	strcpy( seq, "die_forward2" ); break;
				case 3:	strcpy( seq, "die_forward3" ); break;
				case 4:	strcpy( seq, "die_forward4" ); break;
			}	
					
			if (m_bSurrender && !m_bSurrenderStanding)
			switch (RANDOM_LONG(1,3))
			{
				case 1:	strcpy( seq, "surrender_forward1" ); break;
				case 2:	strcpy( seq, "surrender_forward2" ); break;
				case 3:	strcpy( seq, "surrender_forward3" ); break;
			}
			break;
		
		case ACT_DIEBACKWARD:
			switch (RANDOM_LONG(1,5))
			{
				case 1:	strcpy( seq, "backward1" ); break;
				case 2:	strcpy( seq, "backward2" ); break;
				case 3:	strcpy( seq, "backward3" ); break;
				case 4:	strcpy( seq, "backward4" ); break;
				case 5:	strcpy( seq, "backward5" ); break;
			}
					
			if(m_fCrouching)
			switch (RANDOM_LONG(1,4))
			{
				case 1:	strcpy( seq, "die_backward1" ); break;
				case 2:	strcpy( seq, "die_backward2" ); break;
				case 3:	strcpy( seq, "die_backward3" ); break;
				case 4:	strcpy( seq, "die_backward4" ); break;
			}	
					
			if (m_bSurrender && !m_bSurrenderStanding)
			switch (RANDOM_LONG(1,3))
			{
				case 1:	strcpy( seq, "surrender_backward1" ); break;
				case 2:	strcpy( seq, "surrender_backward2" ); break;
				case 3:	strcpy( seq, "surrender_backward3" ); break;
			}
			break;
			
		case ACT_DIESIMPLE:
			switch (RANDOM_LONG(1,3))
			{
				case 1:	strcpy( seq, "simple1" ); break;
				case 2:	strcpy( seq, "simple2" ); break;
				case 3:	strcpy( seq, "simple3" ); break;
			}		
			if(m_fCrouching)
			strcpy( seq, "die_simple" );
		break;
			
		case ACT_DIE_GUTSHOT:
			switch (RANDOM_LONG(1,2))
			{
				case 1:	strcpy( seq, "gutshot1" ); break;
				case 2:	strcpy( seq, "gutshot2" ); break;
			}
			if(m_fCrouching)
			strcpy( seq, "die_simple" );
		break;

		//doesn't get called
/*
		case ACT_DIE_CHESTSHOT:
			strcpy( seq, "die_chestshot" );
			break;
*/
		case ACT_EXPLOSION_HIT:
		case ACT_EXPLOSION_LAND:
		case ACT_EXPLOSION_FLY:
			if ( NewActivity == ACT_EXPLOSION_HIT ) strcpy( seq, "explosion_hit" );
			if ( NewActivity == ACT_EXPLOSION_LAND ) strcpy( seq, "explosion_land" );
			if ( NewActivity == ACT_EXPLOSION_FLY ) strcpy( seq, "explosion_fly" );

			switch ( m_expldir )
			{
				case 1: strcat( seq, "_forward" ); break;
				case 2: strcat( seq, "_backward" ); break;
				case 3: strcat( seq, "_left" ); break;
				case 4: strcat( seq, "_right" ); break;
			}

			iSequence = LookupSequence ( seq );
			break;

		case ACT_HOVER:
			iSequence = LookupSequence ( "repel_repel" );//repel_jump
			break;

		case ACT_GLIDE:
			iSequence = LookupSequence ( "repel_repel" );//repel_repel
			break;

		case ACT_LAND:
			iSequence = LookupSequence ( "repel_repel" );//repel_land
			break;

		case ACT_FLY:
			iSequence = LookupSequence ( "repel_repel" );//repel_shoot
			break;
	}

	// If we still don't have a sequence, try and see if crouching is needed
	if (iSequence == ACTIVITY_NOT_AVAILABLE)
	{
		char seq2[40];

		if ( m_fCrouching )
		{
			strcpy( seq2, "crouch_" );
			strcat( seq2, seq );
			strcpy( seq, seq2 );
		}
		//called animations directly
		/*
		if (m_bSurrender)
		{
			strcpy( seq2, "surrender_" );
			strcat( seq2, seq );
			strcpy( seq, seq2 );
		}
*/
		iSequence = LookupSequence( seq );
	}

//	ALERT (at_console, "Picked final sequence: %s\n", seq);
	
	m_Activity = NewActivity; // Go ahead and set this so it doesn't keep trying when the anim is not present

	// Set to the desired anim, or default anim if the desired is not present
	if ( iSequence > ACTIVITY_NOT_AVAILABLE )
	{
		if ( pev->sequence != iSequence || !m_fSequenceLoops )
		{
			pev->frame = 0;
		}

		pev->sequence		= iSequence;	// Set to the reset anim (if it's there)
		ResetSequenceInfo( );
		SetYawSpeed();
			
		//	pev->gaitsequence = LookupSequence ( "repel_repel" );//repel_shoot

	}
	else
	{
		// Not available try to get default anim
		ALERT ( at_console, "%s has no sequence for act:%d\n", STRING(pev->classname), NewActivity );
		pev->sequence		= 0;	// Set to the reset anim (if it's there)
	}

	if (FClassnameIs( pev, "monster_camera" ))
	iSequence = LookupSequence ( "idle" );//repel_shoot
}


//=========================================================
// Fire procedures - Shoots one round from designated weapon 
// at the enemy human is facing.
//=========================================================

void CBaseHuman :: Fire ( int nBulletType, int Num, Vector vecAccuracy, int nShellModel, int nShellBounce, const char * szSnd )
{
	char szsndfile[128];
	bool bPlayerisFar = false;

	CBaseEntity *pEntidadLODcheck1 = NULL;

	while ((pEntidadLODcheck1 = UTIL_FindEntityByClassname(pEntidadLODcheck1, "player")) != NULL) 
	{	
		float flDist = (pEntidadLODcheck1->Center() - pev->origin).Length();

		if ( flDist > 1111)
		{
			sprintf( szsndfile, "%s_dist.wav", szSnd );
			bPlayerisFar = true;
		}
		else
		sprintf( szsndfile, "%s.wav", szSnd );
	}

	CBasePlayer *pPlayer = NULL;
	pPlayer = (CBasePlayer *)UTIL_PlayerByIndex( 1 );
	if ( pPlayer )
		pPlayer->PlayActionMusic();

	// Get a vector to the target
	UTIL_MakeVectors(pev->angles);
	Vector vecShootDir, vecShootOrigin = GetGunPosition();
	

	if ( m_MonsterState == MONSTERSTATE_SCRIPT )
	{
		vecShootDir = gpGlobals->v_forward;
	}
	else
	{
		vecShootDir = ShootAtEnemy( vecShootOrigin );	
	}


	if (m_fCrouching)
	{
		vecAccuracy.x -= 0.04362;// VECTOR_CONE_5DEGREES;
		vecAccuracy.y -= 0.04362;// VECTOR_CONE_5DEGREES;
		vecAccuracy.z -= 0.04362;// VECTOR_CONE_5DEGREES;
	}

	if (RANDOM_LONG( 0, 99 ) < 5)
	{
		vecAccuracy = VECTOR_CONE_1DEGREES;
	}

	// Fire
//	if( pev->weapons != HUMAN_WEAPON_AWP )
	FireBullets( Num, GetGunEndPosition(), vecShootDir, vecAccuracy, HUMAN_RIFLE_RANGE, nBulletType, 4 ); // shoot +-5 degrees

	// Eject a shell if necessary
	if ( nShellModel )
	{
		if( pev->weapons == HUMAN_WEAPON_AWP )
		{
			Vector	vecShellVelocity = gpGlobals->v_right * RANDOM_FLOAT(40,90)/4 + gpGlobals->v_up * RANDOM_FLOAT(75,200)/4 + gpGlobals->v_forward * RANDOM_FLOAT(-40, 40)/4;
			EjectBrass ( vecShootOrigin + vecShootDir * 24, vecShellVelocity, pev->angles.y, nShellModel, nShellBounce ); 
		}
		else
		{
			Vector	vecShellVelocity = gpGlobals->v_right * RANDOM_FLOAT(40,90) + gpGlobals->v_up * RANDOM_FLOAT(75,200) + gpGlobals->v_forward * RANDOM_FLOAT(-40, 40);
			EjectBrass ( vecShootOrigin + vecShootDir * 24, vecShellVelocity, pev->angles.y, nShellModel, nShellBounce ); 
		}
	}

	if(bPlayerisFar)
		EMIT_SOUND_DYN( ENT(0), CHAN_VOICE, szsndfile, RANDOM_FLOAT(0.3,0.6), ATTN_NONE, 0, PITCH_NORM + RANDOM_LONG(-10,10) );
	else
		EMIT_SOUND( ENT(pev), CHAN_WEAPON, szsndfile, 1, HUMAN_ATTN );

	m_cAmmoLoaded--;
	
//	ALERT( at_console, "INFO: m_cAmmoLoaded %i\n", m_cAmmoLoaded);
}

//=========================================================
// Fire M79 grenade
//=========================================================

void CBaseHuman::M79Fire()
{/*
	if ( m_cAmmoLoaded <= 0 ) return;

	if (RANDOM_LONG(0, 1))
		EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/m79_fire1.wav", 0.8, HUMAN_ATTN);
	else
		EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/m79_fire2.wav", 0.8, HUMAN_ATTN);

	UTIL_MakeVectors(pev->angles);
	if ( m_MonsterState == MONSTERSTATE_SCRIPT ) m_vecTossVelocity = gpGlobals->v_forward * 800;

	CGrenade::ShootContact( pev, GetGunEndPosition(), m_vecTossVelocity );
	m_LastAttack1Check = FALSE;
	m_cAmmoLoaded--;
	m_flNextAttack1Check = gpGlobals->time + 1;
	
	Vector angDir = UTIL_VecToAngles( m_vecTossVelocity );
	SetBlending( 0, angDir.x );*/
}


//=========================================================
// Fire RPG Rocket
//=========================================================

void CBaseHuman :: RPGFire ( void )
{
	if ( m_cAmmoLoaded <= 0 ) return;

	UTIL_MakeAimVectors( pev->angles );
	Vector vecSrc = GetGunPosition();
	Vector vecShootDir = ShootAtEnemy( vecSrc );

	Vector angDir = UTIL_VecToAngles( vecShootDir ) + Vector( RANDOM_FLOAT(-50,50), RANDOM_FLOAT(-50,50), RANDOM_FLOAT(-50,50) );//let's add some spread
	SetBlending( 0, angDir.x );
	angDir.x = -angDir.x;
		
	CBaseEntity *pRocket = CBaseEntity::Create( "hvr_rocket", GetGunPosition(), UTIL_VecToAngles( vecShootDir ), edict() );

	if (pRocket)
		pRocket->pev->velocity = pev->velocity + gpGlobals->v_forward * 100;

	UTIL_MakeVectors ( pRocket->pev->angles );	

	EMIT_SOUND( ENT(pev), CHAN_WEAPON, "weapons/rpg/fire.wav", 1, HUMAN_ATTN );
	CSoundEnt::InsertSound ( bits_SOUND_COMBAT, Classify(), pev->origin, 384, 0.3 );

	m_LastAttack1Check = FALSE;
	m_cAmmoLoaded--;// take away a bullet!
	m_flNextAttack1Check = gpGlobals->time + 1;
}


//=========================================================
// GetGunEndPosition - position of the END of the gun
//=========================================================

Vector CBaseHuman :: GetGunEndPosition( )
{
	Vector v, a;
	GetAttachment(0, v, a);

	return v;
}


//=========================================================
// GetGunPosition - height of the gun above the origin of the monster
//=========================================================

Vector CBaseHuman :: GetGunPosition( )
{
	if (m_fCrouching )
	{
		return pev->origin + Vector( 0, 0, 36 );
	}
	else
	{
		return pev->origin + Vector( 0, 0, 62 );
	}
}


//=========================================================
// Kick and return if you hit anything
//=========================================================

CBaseEntity *CBaseHuman :: Kick( void )
{
	m_flLastKick = gpGlobals->time;

	TraceResult tr;

	UTIL_MakeVectors( pev->angles );
	Vector vecStart = pev->origin;
	vecStart.z += pev->size.z * 0.5;
	Vector vecEnd = vecStart + (gpGlobals->v_forward * 70);

	UTIL_TraceHull( vecStart, vecEnd, dont_ignore_monsters, head_hull, ENT(pev), &tr );
	
	if ( tr.pHit )
	{
		CBaseEntity *pEntity = CBaseEntity::Instance( tr.pHit );
		return pEntity;
	}

	return NULL;
}

/*
//=========================================================
// MonsterThink - if head is cut off we need to spawn some blood here
//=========================================================

void CBaseHuman :: MonsterThink ( void )
{
	// This is to ensure that if the human is repelling and for some reason his velocity is aimed upwards
	// he will not go shooting off into space

	if ( pev->movetype == MOVETYPE_FLY && pev->velocity.z >= 0 ) 
	{
		pev->velocity.z -= 10;
	}
	
	CBaseMonster::MonsterThink();
}
*/

//=========================================================
// TraceAttack - not to be confused with TakeDamage!
//=========================================================
extern int gmsgSpecMsg;

void CBaseHuman::TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType)
{
	float		flDot;
	Vector		vecSrc;

	UTIL_MakeVectors ( pev->angles );
	flDot = DotProduct ( gpGlobals->v_forward, g_vecAttackDir * -1 );

	if ( flDot > 0.3 )
	yaw_adj = RANDOM_FLOAT ( -50.0, 0.0 );
	else if ( flDot <= -0.3 )
	yaw_adj = RANDOM_FLOAT ( 0.0, 50.0 );

	if (bitsDamageType & DMG_PELLETS)
		m_bloodColor = DONT_BLEED;
	else
		m_bloodColor = BLOOD_COLOR_RED;
	
	CBaseEntity *pPlayer = GetClassPtr((CBaseEntity *)pevAttacker);

	if( pPlayer && pPlayer->IsPlayer() )//only players, huh :I
	{
		float flDist = ( pev->origin - pPlayer->pev->origin ).Length2D();
		
		if ( flDist <= 64 )
		{
		MESSAGE_BEGIN(MSG_ONE,gmsgSpecMsg,NULL,pPlayer->pev);
			WRITE_BYTE(CL_HURTIMAGE);
			WRITE_BYTE(1);
		MESSAGE_END();
		}

	}

	switch( ptr->iHitgroup)
	{
	case HITGROUP_CHEST:
		if (bitsDamageType & (DMG_BULLET | DMG_SLASH | DMG_BLAST))
		{	
			if(m_bUseArmor)
			{
				flDamage = flDamage / 4;

				switch (RANDOM_LONG(0,1))
				{
					case 0:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/damage/hit_kevlar-1.wav", 0.9, ATTN_NORM); break;
					case 1:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/damage/hit_kevlar-2.wav", 0.9, ATTN_NORM); break;
				}
					
				m_bloodColor = DONT_BLEED;
			}
			
			if ( flDot > 0.3 )
			yaw_adj = RANDOM_FLOAT ( -50.0, 0.0 );
			else if ( flDot <= -0.3 )
			yaw_adj = RANDOM_FLOAT ( 0.0, 50.0 );
		}
		break;

	case HITGROUP_STOMACH:
		if (bitsDamageType & (DMG_BULLET | DMG_SLASH | DMG_BLAST))
		{
			if(m_bUseArmor)
			{
				flDamage = flDamage / 4;

				switch (RANDOM_LONG(0,1)) 
				{
					case 0:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/damage/hit_kevlar-1.wav", 0.9, ATTN_NORM); break;
					case 1:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/damage/hit_kevlar-2.wav", 0.9, ATTN_NORM); break;
				}
					
				m_bloodColor = DONT_BLEED;
			}
						
			if ( flDot > 0.3 )
			yaw_adj = RANDOM_FLOAT ( 0.0, 50.0 );
			else if ( flDot <= -0.3 )
			yaw_adj = RANDOM_FLOAT ( -50.0, 0.0 );
		}
		break;

	case HITGROUP_HEAD:
		if (bitsDamageType & (DMG_BULLET | DMG_SLASH | DMG_CLUB))
		{
			Vector vecLookerOrigin = pev->origin + pev->view_ofs;//look through the caller's 'eyes'
			
			if (m_fCrouching)
				vecLookerOrigin[2] -= 30;

			MESSAGE_BEGIN(MSG_ALL, gmsgParticles);
				WRITE_SHORT(0);
				WRITE_BYTE(1);
				WRITE_COORD(vecLookerOrigin.x);
				WRITE_COORD(vecLookerOrigin.y);
				WRITE_COORD(vecLookerOrigin.z);
				WRITE_COORD(0);
				WRITE_COORD(0);
				WRITE_COORD(0);
				WRITE_SHORT(0);
				WRITE_STRING("e_headshot_cluster.txt");
			MESSAGE_END();

			/*BEGIN_READ(pbuf, iSize);
	int iId = READ_SHORT();
	int iType = READ_BYTE();

	vec3_t pos;
	pos.x = READ_COORD();
	pos.y = READ_COORD();
	pos.z = READ_COORD();

	vec3_t ang;
	ang.x = READ_COORD();
	ang.y = READ_COORD();
	ang.z = READ_COORD();
	int iPreset = READ_SHORT();
	char *szPath = READ_STRING();*/
			if(m_bUseArmor)
			{
				flDamage *= 0.5;
				
				switch (RANDOM_LONG(0,1)) 
				{
					case 0:	EMIT_SOUND(ENT(pev), CHAN_STATIC, "player/damage/helmet1.wav", 0.9, ATTN_NORM); break;
					case 1:	EMIT_SOUND(ENT(pev), CHAN_STATIC, "player/damage/helmet2.wav", 0.9, ATTN_NORM); break;
				}

				//hawk add hawk debug hawk fix
	/*			if (RANDOM_LONG( 0, 99 ) < 40)
				UTIL_WhiteSparks( ptr->vecEndPos, ptr->vecPlaneNormal, 0, 5, 50, 1000 );//chispas

				UTIL_WhiteSparks( ptr->vecEndPos, ptr->vecPlaneNormal, 9, 5, 5, 100 );//puntos
				UTIL_WhiteSparks( ptr->vecEndPos, ptr->vecPlaneNormal, 0, 5, 100, 20 );//chispas
		*/
				m_bloodColor = DONT_BLEED;
			}

		//	EMIT_SOUND(ENT(pev), CHAN_STATIC, "player/damage/headshot2.wav", 0.9, ATTN_NORM);

				CBaseEntity *pPlayer = GetClassPtr((CBaseEntity *)pevAttacker);

		if( pPlayer && pPlayer->IsPlayer() )//only players, huh :I
				EMIT_SOUND_FMOD_2D_ONE( ENT( pevAttacker ), "player/damage/headshot2.wav" );

			SentenceStop();
					
			if ( flDot > 0.3 )
			yaw_adj = RANDOM_FLOAT ( -50.0, 0.0 );
			else if ( flDot <= -0.3 )
			yaw_adj = RANDOM_FLOAT ( 0.0, 50.0 );
		}
		// always a head shot
		ptr->iHitgroup = HITGROUP_HEAD;
		break;
	}

	if (pPlayer && pPlayer->IsPlayer())//only players, huh :I
	{
		switch ( RANDOM_LONG( 0, 2 ) )
		{
			case 0: EMIT_SOUND_FMOD_2D_ONE( ENT( pevAttacker ), "player/damage/hit_flesh-1.wav" ); break;
			case 1: EMIT_SOUND_FMOD_2D_ONE( ENT( pevAttacker ), "player/damage/hit_flesh-2.wav" ); break;
			case 2: EMIT_SOUND_FMOD_2D_ONE( ENT( pevAttacker ), "player/damage/hit_flesh-3.wav" ); break;
		}
		switch ( RANDOM_LONG( 0, 1 ) )
		{
			case 0: EMIT_SOUND_FMOD_2D_ONE( ENT( pevAttacker ), "weapons/bullet_hit1.wav" ); break;
			case 1: EMIT_SOUND_FMOD_2D_ONE( ENT( pevAttacker ), "weapons/bullet_hit2.wav" ); break;
		}
	}

	if ( (flDamage >  20) )//deagle or something
		bViolentHeadDamaged = TRUE;
	else
		bViolentHeadDamaged = FALSE;

	CSquadMonster::TraceAttack( pevAttacker, flDamage, vecDir, ptr, bitsDamageType );
}


//=========================================================
// TakeDamage
//=========================================================

int CBaseHuman :: TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	if (pev->deadflag == DEAD_DYING)
		return	CSquadMonster :: TakeDamage ( pevInflictor, pevAttacker, 0, bitsDamageType );


	Forget( bits_MEMORY_INCOVER );

  	int ret = CSquadMonster :: TakeDamage ( pevInflictor, pevAttacker, flDamage, bitsDamageType );

	if ( m_MonsterState == MONSTERSTATE_SCRIPT ||
		 m_MonsterState == MONSTERSTATE_PRONE ||
		 m_MonsterState == MONSTERSTATE_NONE ) return ret;

/*	if ( pevInflictor && IsAlive() && ( m_IdealMonsterState == MONSTERSTATE_DEAD ) &&  ( FBitSet( bitsDamageType, DMG_BLAST ) ) )
	{
		Vector dir = pev->origin - pevInflictor->origin;

		//MakeIdealYaw( dir );
		float Yaw = VecToYaw( dir ) + ( 180 - pev->angles.y );
		if ( Yaw < 0 ) Yaw += 360;
		if ( Yaw >= 360 ) Yaw -= 360;

		if ( Yaw >= 180 - 45 && Yaw < 180 + 45 ) m_expldir = 1; // his back is to the explosion
		else if ( Yaw >= 180 + 45 && Yaw < 180 + 90 + 45 ) m_expldir = 3; // his right side is to the explosion
		else if ( Yaw >= 180 - 90 - 45 && Yaw < 180 - 45 ) m_expldir = 4; // his left side is to the explosion
		else m_expldir = 2; // his front is to the explosion

		if ( FBitSet( bitsDamageType, DMG_BLAST ) )
		{
			pev->velocity = dir.Normalize() * flDamage * 3;
			pev->velocity.z = flDamage * 5;
		}
//		else
//		{
//			pev->velocity = dir.Normalize() * ( 100 + flDamage );
//			pev->velocity.z = 200;
//		}
						
	//	if ( bTerrorist )
	//	SET_MODEL(ENT(pev), "models/terrorist_zo.mdl");

		if ( SafeToChangeSchedule() )
		{
		ChangeSchedule( GetScheduleOfType( SCHED_HUMAN_EXPLOSION_DIE ) );
		SetState( MONSTERSTATE_DEAD );
		}
	}*/

	//only player can make the monster surrender
	if ( m_MonsterState != MONSTERSTATE_PRONE && (pevAttacker->flags & FL_CLIENT) )
	{
		if ( bitsDamageType & DMG_BULLET )
		m_iFrustration += flDamage; //ie, 762 mm shoot takes 23 points, total 46. Whit four shots the monster can surrender.
//		m_iFrustration += flDamage * 2; //ie, 762 mm shoot takes 23 points, total 46. Whit four shots the monster can surrender.

		if ( bitsDamageType & DMG_BLAST )
		m_iFrustration += 50;

		if ( bitsDamageType & DMG_SLASH )//knife
		m_iFrustration += 30;

		if ( bitsDamageType & DMG_CLUB )//crowbar
		m_iFrustration += 80;
	
		if (bitsDamageType & DMG_PELLETS)
		{
			m_iFrustration += 80;

			if ( SafeToChangeSchedule() )
			{
				ChangeSchedule( GetScheduleOfType( SCHED_COWER ) );
			
				PlaySentence( "T_COUGH", 4, VOL_NORM, ATTN_NORM, 100 );
			}
		}
	}

	if ( ( bitsDamageType & DMG_BULLET) || ( bitsDamageType & DMG_SHOTGUN) )
	{
		if (pev->deadflag == DEAD_DYING)
		{
		/*
			if(pev->frame < 30)
			{
				pev->frame = RANDOM_FLOAT ( 0, 15 );
				pev->framerate = 0.5;
				ResetSequenceInfo( );
			}
			else
			pev->framerate = 1;*/
		}

		// TEST || TEST || TEST || TEST || TEST || TEST || TEST || TEST || TEST || TEST || TEST || TEST || TEST
		// TEST || TEST || TEST || TEST || TEST || TEST || TEST || TEST || TEST || TEST || TEST || TEST || TEST
		/*
		if (bitsDamageType & DMG_PELLETS)
		{

		}
		else
		{
			if ( (m_iBreakPoint == HIGH_RESISTENCE) || (m_iBreakPoint == SUPER_RESISTENCE) || ( (m_iBreakPoint == MED_RESISTENCE) && ( (int)(RANDOM_FLOAT(0,1) == 1 )) ) )
			{
				if ( ( m_hEnemy != NULL && m_hEnemy->IsAlive() ) )	// Try and get new enemy
				{		
					if ( SafeToChangeSchedule() )
					{
						if(m_bSurrender)
						return ret;

						ChangeSchedule( GetScheduleOfType( SCHED_RANGE_ATTACK1 ) );
						
						PlayLabelledSentence( "EXPL" );//scream
					}
				}
			}
		}*/
		
		// TEST || TEST || TEST || TEST || TEST || TEST || TEST || TEST || TEST || TEST || TEST || TEST || TEST
		// TEST || TEST || TEST || TEST || TEST || TEST || TEST || TEST || TEST || TEST || TEST || TEST || TEST
	}

	ALERT( at_console, "flDamage %f\n", flDamage );
	return ret;
}


//=========================================================
// Detect when I have hit the ground after an explosion
//=========================================================

void CBaseHuman::ExplFlyTouch( CBaseEntity *pOther )
{
//	if ( FBitSet( pev->flags, FL_ONGROUND )  )
//	{
	m_IdealActivity = ACT_EXPLOSION_LAND;
	SetActivity( m_IdealActivity );
	TaskComplete();
	SetTouch( NULL );
//	}
}


//=========================================================
// Criteria for gibbing instead of just dying normally
//=========================================================

BOOL CBaseHuman::ShouldGibMonster( int iGib )
{
	if (m_bKamikaze)
	return TRUE;

	if ( ( iGib == GIB_NORMAL && pev->health < HUMAN_GIB_HEALTH_VALUE ) || ( iGib == GIB_ALWAYS ) )
		return TRUE;
	
	return FALSE;
}


//=========================================================
// Killed - called when he's killed
//=========================================================
CBaseEntity *pCamera = NULL;
CBaseEntity *pNpc = NULL;
CBaseEntity *pViewPos = NULL;
CBaseEntity *pViewAngles = NULL;
void CBaseHuman::Killed(entvars_t *pevAttacker, int iGib)
{
	//	Vector	vecSrc  = Center();
	Vector vecLookerOrigin = pev->origin + pev->view_ofs;//look through the caller's 'eyes'

	CBaseEntity *pEntidadLODcheck1 = NULL;

//	int chances = 25;
	int chances = 5;

	while ((pEntidadLODcheck1 = UTIL_FindEntityByClassname(pEntidadLODcheck1, "player")) != NULL)
	{
		float flDist = (pEntidadLODcheck1->Center() - pev->origin).Length();
		if (flDist > 800)
			chances -= 10;
	}

	if (bViolentHeadDamaged)
		chances += 50;

	if (RANDOM_FLOAT(0, 100) < chances)
	{
	/*	UTIL_MakeVectors(Vector(0, pev->v_angle.y, 0));

		Vector vecEyes, vecAngles, nan;

		GET_ATTACHMENT(this->edict(), 0, vecEyes, nan);
		GET_ATTACHMENT(this->edict(), 1, vecAngles, nan);

		vecEyes[0] += RANDOM_FLOAT(-50, 50);
		vecEyes[1] += RANDOM_FLOAT(-50, 50);
		vecEyes[2] += RANDOM_FLOAT(50, 100);

		pViewPos = CBaseEntity::Create("info_target", vecEyes, g_vecZero, edict());
		pViewPos->SetThink(&CBaseEntity::SUB_Remove);
		pViewPos->pev->nextthink = gpGlobals->time + 0.5;




		pViewAngles = CBaseEntity::Create("info_target", vecLookerOrigin, g_vecZero, edict());
		pViewAngles->pev->targetname = MAKE_STRING("ViewAngles");
		pViewAngles->SetThink(&CBaseEntity::SUB_Remove);
		pViewAngles->pev->nextthink = gpGlobals->time + 0.5;

		pCamera = CBaseEntity::Create("npc_camera", pViewPos->pev->origin, pViewAngles->pev->angles);


	
		CBaseEntity *pPlayer = NULL;
		pPlayer = CBaseEntity::Instance(g_engfuncs.pfnPEntityOfEntIndex(1));

		Vector vecArmPos, vecArmDir;
		Vector vecDirToEnemy;
		Vector angDir;

		vecDirToEnemy = ((pViewPos->pev->origin) - pCamera->pev->origin);
		angDir = UTIL_VecToAngles(vecDirToEnemy);
		vecDirToEnemy = vecDirToEnemy.Normalize();

		pCamera->pev->angles = vecDirToEnemy;

		pCamera->pev->angles.x = -pev->angles.x;
		pCamera->pev->angles.y = pev->angles.y;
		pCamera->pev->angles.z = 0;


		pCamera->pev->target = pViewAngles->pev->targetname;
		pCamera->Use(this, this, USE_TOGGLE, 0);


			pCamera->SetThink(&CBaseEntity::SUB_Remove);
			pCamera->pev->nextthink = gpGlobals->time + 3;
			*/
	//	SERVER_COMMAND("host_framerate 0.002\n");
	//	bSlow = true;
	//	bViolentHeadDamaged = true;

	 //	SERVER_COMMAND("hud_draw 0\n");

	//	CBaseEntity *pPlayer = GetClassPtr((CBaseEntity *)pevAttacker);

	//	if (pPlayer && pPlayer->IsPlayer())//only players, huh :I
 	//	EMIT_SOUND_FMOD_2D_ONE(ENT(pevAttacker), "player/bullettime.mp3");
		EMIT_SOUND_FMOD_2D_ALL("player/bullettime.mp3");
	}
//	if (!ShouldFadeOnDeath())
	{
		MESSAGE_BEGIN(MSG_ALL, gmsgParticles);
			WRITE_SHORT(0);
			WRITE_BYTE(1);
			WRITE_COORD(vecLookerOrigin.x);
			WRITE_COORD(vecLookerOrigin.y);
			WRITE_COORD(vecLookerOrigin.z);
			WRITE_COORD(0);
			WRITE_COORD(0);
			WRITE_COORD(0);
			WRITE_SHORT(0);
			WRITE_STRING("engine_blood_pool.txt");
		MESSAGE_END();

 		EMIT_SOUND_FMOD_3D(ENT(pev), "common/blood_spill.wav", 128);
	}

	if (  !ShouldGibMonster(iGib))
	{
		 
		SetUse(&CBaseHuman::DeadUse);
	}
	CSquadMonster::Killed(pevAttacker, iGib);

	if (m_bKamikaze)
	{
		Vector	vecGunPos;
		Vector	vecGunAngles;

		GetAttachment( 3, vecGunPos, vecGunAngles );
		
		CGrenade::ShootTimed( pev, vecGunPos + gpGlobals->v_forward * 17 - gpGlobals->v_right * 27 + gpGlobals->v_up * 6, g_vecZero, 4 );
	}

	CBaseEntity *killer = CBaseEntity::Instance(pevAttacker);
	SentenceStop();

//	pev->frame = 0;
	CSquadMonster::Killed( pevAttacker, iGib );
}

extern int gmsgTbutton;

bool m_bState = false;
bool m_bInUse = false;
float m_flOnTime = 3.0f;
float m_flOffTime = 3.0f;
float m_flStartTime = 0.0;
float m_flLastActTime = 0.0;

 

// Wargon: Âîçìîæíîñòü ïîäáèðàòü ïàòðîíû þçîì èç ìåðòâûõ âðàæèí. (1.1)
void CBaseHuman::DeadUse(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if (!pActivator->IsPlayer()) // Players only
		return;

	if (!pActivator->IsAlive()) // Alive only
		return;

	if (((m_flLastActTime + 2.0) > gpGlobals->time)  ) // Not so fast
		return;

	if (m_bInUse && pActivator->edict() != pev->owner) // One player at a time
		return;

	if (!m_bInUse) // First time usage
	{
		if (!m_bState)
		{
			if (m_flOnTime > 0.0) // Send message only if we have time
			{
				UTIL_ShowMessage(STRING(pev->noise), pActivator);

				MESSAGE_BEGIN(MSG_ONE, gmsgTbutton, NULL, pActivator->edict());
				WRITE_SHORT(m_flOnTime);
				MESSAGE_END();
			}
		}
		else if (m_bState)
		{
			if (m_flOffTime > 0)
			{
				MESSAGE_BEGIN(MSG_ONE, gmsgTbutton, NULL, pActivator->edict());
				WRITE_SHORT(m_flOffTime);
				MESSAGE_END();
			}
		}
		// Set some stuff
		m_flStartTime = gpGlobals->time;
		pev->owner = pActivator->edict();
		m_bInUse = true;
	}
	// Trying to activate it
	/*if (!m_bState)
	{
		if ((m_flStartTime + m_flOnTime) <= gpGlobals->time) // Check if it has to be enabled
		{
			m_bState = true;
			if (m_flOnTime > 0)
			{
				MESSAGE_BEGIN(MSG_ONE, gmsgTbutton, NULL, pev->owner);
				WRITE_SHORT(0);
				MESSAGE_END();
			}
 
			UTIL_ShowMessage(STRING(pev->message), pActivator);
			m_flLastActTime = gpGlobals->time;
 			// Button is no longer used
			m_bInUse = false;
			m_flStartTime = 0.0;
			pev->owner = NULL;
			return;
		}
	}
	else // Trying to disable it
	{
	if ((m_flStartTime + m_flOffTime) <= gpGlobals->time) // Check if it has to be disabled
		{
			m_bState = false;
			if (m_flOffTime > 0)
			{
				MESSAGE_BEGIN(MSG_ONE, gmsgTbutton, NULL, pev->owner);
				WRITE_SHORT(0);
				MESSAGE_END();
			}
			 
			m_flLastActTime = gpGlobals->time;
 			// Button is no longer used
			m_bInUse = false;
			m_flStartTime = 0.0;
			pev->owner = NULL;
			return;
		
	}
	*/
/*	if (pActivator->GiveAmmo(m_cAmmoLoaded, "ammo_9mmAR", _9MM_MAX_CARRY) != -1)
	{
		EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		 
		SetUse(NULL);
	}
	else if (pActivator->GiveAmmo(m_cAmmoLoaded, "ammo_buckshot", BUCKSHOT_MAX_CARRY) != -1)
	{
		EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		 
		SetUse(NULL);
	}*/
}

//=========================================================
// AlertFriends - when killed by player, tell friends player is evil
//=========================================================

void CBaseHuman::AlertFriends( void )
{
	CBaseEntity *pFriend = NULL;
	int i;

	// for each friend in this bsp...
	for ( i = 0; i < m_nNumFriendTypes; i++ )
	{
		while (pFriend = EnumFriends( pFriend, i, TRUE ))
		{
			CBaseHuman *pHuman = pFriend->MyHumanPointer();
			if ( pHuman != NULL && pHuman->IsAlive() && pHuman->pev->deadflag != DEAD_DYING	&& pHuman->FVisible( this ) ) 
				
				// don't provoke a friend that's playing a death animation. They're a goner
				// also don't provoke friends that can't see you
			{
				pHuman->Remember( bits_MEMORY_PROVOKED );
				if ( pHuman->IsFollowingPlayer() ) pHuman->StopFollowing( TRUE );
			}
		}
	}
}


//=========================================================
// ShutUpFriends - when I'm talking as part of a script, 
// tell friends to shut it
//=========================================================

void CBaseHuman::ShutUpFriends( void )
{
	CBaseEntity *pFriend = NULL;
	int i;

	// for each friend in this bsp...
	for ( i = 0; i < m_nNumFriendTypes; i++ )
	{
		while (pFriend = EnumFriends( pFriend, i, TRUE ))
		{
			CBaseMonster *pMonster = pFriend->MyMonsterPointer();
			if ( pMonster )
			{
				pMonster->SentenceStop();
			}
		}
	}
}


//=========================================================
// CanPlaySentence - used by scripted sentences
//=========================================================

int CBaseHuman::CanPlaySentence( BOOL fDisregardState ) 
{ 
	if ( fDisregardState )
		return CBaseMonster::CanPlaySentence( fDisregardState );
	return FOkToSpeak(); 
}


//=========================================================
// PlayScriptedSentence - what it says
//=========================================================

void CBaseHuman::PlayScriptedSentence( const char *pszSentence, float duration, float volume, float attenuation, BOOL bConcurrent, CBaseEntity *pListener )
{
	if ( !bConcurrent )
		ShutUpFriends();

	ClearConditions( bits_COND_PUSHED );	// Forget about moving!  I've got something to say!
	PlaySentence( pszSentence, duration, volume, attenuation, 100 );

	m_hTalkTarget = pListener;
}


//=========================================================
// PlayLabelledSentence - Adds a monster-specific label to
// the front of pszSentence and then calls PlaySentence
//=========================================================

void CBaseHuman::PlayLabelledSentence( const char *pszSentence )
{
	char szSentence[32];
	strcpy( szSentence, m_szSpeechLabel );
	strcat( szSentence, pszSentence );

	PlaySentence( szSentence, GetDuration( pszSentence ), VOL_NORM, HUMAN_ATTN, GetVoicePitch() );
}


//=========================================================
// PlaySentence - Plays a sentence from sentences.txt
//=========================================================

void CBaseHuman::PlaySentence( const char *pszSentence, float duration, float volume, float attenuation, float pitch )
{
	if ( !pszSentence )
		return;

	CBaseHuman::g_talkWaitTime = gpGlobals->time + duration;

	if ( pszSentence[0] == '!' )
		EMIT_SOUND_DYN( edict(), CHAN_VOICE, pszSentence, volume, attenuation, 0, pitch );
	else
	{
		if (FClassnameIs(pev, "monster_agency_member"))
		SENTENCEG_PlayRndSz( edict(), pszSentence, volume, attenuation, 0, pitch, TRUE );
		else
		SENTENCEG_PlayRndSz( edict(), pszSentence, volume, attenuation, 0, pitch );
	}

	Talk ( duration );

//	ALERT( at_console, "PlaySentence: %s\n", pszSentence );
}


//=========================================================
// Talk - set a timer that tells us when the monster is done
// talking.
//=========================================================

void CBaseHuman :: Talk( float flDuration )
{
	if ( flDuration <= 0 )
	{
		// no duration :( 
		m_flStopTalkTime = gpGlobals->time + 3;
	}
	else
	{
		m_flStopTalkTime = gpGlobals->time + flDuration;
	}

	m_flLastTalkTime = gpGlobals->time;
}


//=========================================================
// turn head towards supplied origin
//=========================================================

void CBaseHuman :: IdleHeadTurn( Vector &vecFriend )
{
	 // turn head in desired direction only if ent has a turnable head
	if (m_afCapability & bits_CAP_TURN_HEAD)
	{
		float yaw = VecToYaw(vecFriend - pev->origin) - pev->angles.y;

		if (yaw > 180) yaw -= 360;
		if (yaw < -180) yaw += 360;

		// turn towards vector
		SetBoneController( 0, yaw );
	}
}


//=========================================================
// Prepare this monster to answer question
//=========================================================

void CBaseHuman :: SetAnswerQuestion( CBaseHuman *pSpeaker, float duration )
{
	if ( SafeToChangeSchedule() )	ChangeSchedule( GetScheduleOfType( SCHED_HUMAN_IDLE_RESPONSE ) );
	m_hTalkTarget = (CBaseMonster *)pSpeaker;
	m_flStopTalkTime = duration;
}


//=========================================================
// CheckAmmo - overridden for Human because he actually
// uses ammo! (base class doesn't)
//=========================================================

void CBaseHuman :: CheckAmmo ( void )
{
	if ( m_cAmmoLoaded <= 0 )
	{
		SetConditions(bits_COND_NO_AMMO_LOADED);
	}
}


//=========================================================
// Will a bullet fired from my gun hit the enemy?
//=========================================================

BOOL CBaseHuman::CheckBulletAttack( float flDot, float flDist )
{
	if(m_bSurrender)
	return FALSE;

	if(m_bKamikaze)
	return FALSE;

	if(bOnFire)
	return FALSE;

	if (RANDOM_LONG( 0, 99 ) < 20)
	return FALSE;

//	if(!m_bWakeUp)
//	return FALSE;

	if ( gpGlobals->time < m_flNextAttack1Check ) return m_LastAttack1Check;

	m_LastAttack1Check = FALSE;

//	if ( !HasConditions( bits_COND_ENEMY_OCCLUDED ) && flDist <= HUMAN_RIFLE_RANGE && flDot >= 0.5 && NoFriendlyFire() )
	
//	if ( !HasConditions( bits_COND_ENEMY_OCCLUDED ) && flDist <= iDistToShoot && flDot >= 0.5 && NoFriendlyFire() )
	

	if ( !HasConditions( bits_COND_ENEMY_OCCLUDED ) /*&& flDist <= iDistToShoot*/ && NoFriendlyFire() )
	{
		// verify that a bullet fired from the gun will hit an enemy before the world.

		TraceResult	tr;
		Vector vecSrc = GetGunPosition();
		UTIL_TraceLine( vecSrc, m_hEnemy->BodyTarget(vecSrc), dont_ignore_monsters, ignore_glass, ENT(pev), &tr);
		
		if ( tr.flFraction == 1.0 || (tr.pHit != NULL && IRelationship( CBaseEntity::Instance(tr.pHit) ) > R_NO ) )
		{
			m_LastAttack1Check = TRUE;
			m_fStopCrouching = FALSE;
		}
		else if (m_fCrouching)
		{
			//EDIT vecSrc = pev->origin + Vector( 0, 0, 62 ); ??
			//why check if I can see the enemy if I am not crouched?
			//fire a traceline from my eyes, if blocked, then uncrouch
		//	vecSrc = pev->view_ofs;

	//		vecSrc = pev->origin + Vector( 0, 0, 52 );//estable
			vecSrc = pev->origin + Vector( 0, 0, 62 );

//			UTIL_TraceLine( vecSrc, m_hEnemy->BodyTarget(vecSrc), dont_ignore_monsters, ignore_glass, ENT(pev), &tr);
			UTIL_TraceLine( vecSrc, m_hEnemy->BodyTarget(vecSrc), dont_ignore_monsters, ignore_glass, ENT(pev), &tr);
		
			if ( tr.flFraction == 1.0 || (tr.pHit != NULL && IRelationship( CBaseEntity::Instance(tr.pHit) ) > R_NO ) )
			{
				m_LastAttack1Check = TRUE;
				m_fStopCrouching = TRUE;
			//	ShowLine( vecSrc, m_hEnemy->BodyTarget(vecSrc), 2 );
			}
		//	else
		//	ShowLine( vecSrc, m_hEnemy->BodyTarget(vecSrc), 1 );
		}

		m_flNextAttack1Check = gpGlobals->time + 1.0;//1.5
	}

	return m_LastAttack1Check;
}


//=========================================================
// Will a rocket hit the enemy?
//=========================================================

BOOL CBaseHuman::CheckRocketAttack( float flDot, float flDist )
{
	if(m_bSurrender)
	return FALSE;

	if(bOnFire)
	return FALSE;

//	if(!m_bWakeUp)
//	return FALSE;

	if ( gpGlobals->time < m_flNextAttack1Check ) return m_LastAttack1Check;
	m_LastAttack1Check = FALSE;

	// Don't shoot if enemy is too far away or too close
	if ( flDist > HUMAN_EXPLOSIVE_MAX_RANGE || flDist <= HUMAN_EXPLOSIVE_MIN_RANGE ) return m_LastAttack1Check;

	// if he isn't moving, it's ok to check.
	if ( m_flGroundSpeed != 0 )	return m_LastAttack1Check;

	// Will a rocket hit the enemy?
	TraceResult	tr;
	Vector vecSrc = GetGunPosition();
	Vector vecTarget = m_hEnemy->BodyTarget(vecSrc);
	UTIL_TraceLine( vecSrc, vecTarget, dont_ignore_monsters, dont_ignore_glass, ENT(pev), &tr);

	if ( tr.flFraction == 1.0 || (tr.pHit != NULL && IRelationship( CBaseEntity::Instance(tr.pHit) ) > R_NO ) )
	{
		// Am I or any of my squad members near the impact area?
		Vector vecImpact = vecSrc + tr.flFraction * ( vecTarget - vecSrc );

		if (	( ( vecImpact - pev->origin ).Length2D() <= HUMAN_EXPLOSIVE_MIN_RANGE )
			||	( InSquad() && SquadMemberInRange( vecImpact, HUMAN_EXPLOSIVE_MIN_RANGE ) ) )
		{
			m_flNextAttack1Check = gpGlobals->time + 1;
			m_LastAttack1Check = FALSE;
			return m_LastAttack1Check;
		}

		m_LastAttack1Check = TRUE;
	}

	return m_LastAttack1Check;
}


//=========================================================
// Will a contact grenade hit the enemy?
//=========================================================

BOOL CBaseHuman::CheckContactGrenadeAttack( float flDot, float flDist )
{
	if ( m_hEnemy == NULL ) return FALSE;

	// if he isn't moving, it's ok to check.
	if ( m_flGroundSpeed != 0 )
	{
		m_LastAttack1Check = FALSE;
		return m_LastAttack1Check;
	}

	// assume things haven't changed too much since last time
	if (gpGlobals->time < m_flNextAttack1Check )
	{
		return m_LastAttack1Check;
	}

	if ( !FBitSet ( m_hEnemy->pev->flags, FL_ONGROUND ) && m_hEnemy->pev->waterlevel == 0 && m_vecEnemyLKP.z > pev->absmax.z  )
	{
		//!!!BUGBUG - we should make this check movetype and make sure it isn't FLY? Players who jump a lot are unlikely to 
		// be grenaded.
		// don't throw grenades at anything that isn't on the ground!
		m_LastAttack1Check = FALSE;
		return m_LastAttack1Check;
	}

	Vector vecTarget;

	// find target
	vecTarget = m_vecEnemyLKP + (m_hEnemy->BodyTarget( pev->origin ) - m_hEnemy->pev->origin);
	
	// estimate position
	if (HasConditions( bits_COND_SEE_ENEMY))
		vecTarget = vecTarget + ((vecTarget - pev->origin).Length() / gSkillData.hgruntGrenadeSpeed) * m_hEnemy->pev->velocity;

	// Am I or any of my squad members near the intended grenade impact area?

	if (	( ( vecTarget - pev->origin ).Length2D() <= HUMAN_EXPLOSIVE_MIN_RANGE )
		||	( InSquad() && SquadMemberInRange( vecTarget, HUMAN_EXPLOSIVE_MIN_RANGE ) ) )
	{
		m_flNextAttack1Check = gpGlobals->time + 1;
		m_LastAttack1Check = FALSE;
		return m_LastAttack1Check;
	}

	Vector vecToss = VecCheckThrow( pev, GetGunPosition(), vecTarget, gSkillData.hgruntGrenadeSpeed, 0.5 );

	if ( vecToss != g_vecZero )
	{
		m_vecTossVelocity = vecToss;
		m_LastAttack1Check = TRUE;
		m_flNextAttack1Check = gpGlobals->time + 0.3; // 1/3 second.
	}
	else
	{
		m_LastAttack1Check = FALSE;
		m_flNextAttack1Check = gpGlobals->time + 1; // one full second.
	}

	return m_LastAttack1Check;
}


//=========================================================
// Will a grenade from my gun hit the enemy?
//=========================================================

BOOL CBaseHuman::CheckTimedGrenadeAttack( float flDot, float flDist )
{
	if(m_bSurrender)
	return FALSE;

	if(bOnFire)
	return FALSE;

//	if(!m_bWakeUp)
//	return FALSE;

	if ( m_hEnemy == NULL ) return FALSE;

	// if he isn't moving, it's ok to check.
	if ( m_flGroundSpeed != 0 )
	{
		m_LastAttack2Check = FALSE;
		return m_LastAttack2Check;
	}

	// assume things haven't changed too much since last time
	if (gpGlobals->time < m_flNextAttack2Check )
	{
		return m_LastAttack2Check;
	}

	if ( !FBitSet ( m_hEnemy->pev->flags, FL_ONGROUND ) && m_hEnemy->pev->waterlevel == 0 && m_vecEnemyLKP.z > pev->absmax.z  )
	{
		//!!!BUGBUG - we should make this check movetype and make sure it isn't FLY? Players who jump a lot are unlikely to 
		// be grenaded.
		// don't throw grenades at anything that isn't on the ground!
		m_LastAttack2Check = FALSE;
		return m_LastAttack2Check;
	}

	Vector vecTarget;

	vecTarget = m_vecEnemyLKP + (m_hEnemy->BodyTarget( pev->origin ) - m_hEnemy->pev->origin);
	vecTarget = vecTarget + m_hEnemy->pev->velocity * 2;

	// Am I or any of my squad members near the intended grenade impact area?

	if (	( ( vecTarget - pev->origin ).Length2D() <= HUMAN_EXPLOSIVE_MIN_RANGE )
		||	( InSquad() && SquadMemberInRange( vecTarget, HUMAN_EXPLOSIVE_MIN_RANGE ) ) )
	{
		m_flNextAttack2Check = gpGlobals->time + 1;
		m_LastAttack2Check = FALSE;
		return m_LastAttack2Check;
	}


	Vector vecToss = VecCheckToss( pev, GetGunPosition(), vecTarget, 0.5 );

	if ( vecToss != g_vecZero )
	{
		m_vecTossVelocity = vecToss;

		m_LastAttack2Check = TRUE;						// throw a hand grenade
		m_flNextAttack2Check = gpGlobals->time + 6; // don't check again for a while.
			
		// take it down
		m_iNumGrenades -= 1;
	}
	else
	{
		m_LastAttack2Check = FALSE;					// don't throw
		m_flNextAttack2Check = gpGlobals->time + 1; // don't check again for a while.
	}

	return m_LastAttack2Check;
}


//=========================================================
// CheckRangeAttack1
//=========================================================

BOOL CBaseHuman :: CheckRangeAttack1 ( float flDot, float flDist )
{
//	if (RANDOM_LONG( 0, 99 ) < 80)//this will make the enemies go run for melee
//	return FALSE;

	if(m_bSurrender)
	return FALSE;

	if(bOnFire)
	return FALSE;

//	if (!b_FacingEnemyForShoot)
//	return FALSE;

//	if(!m_bWakeUp)
//	return FALSE;

	switch ( pev->weapons )
	{
	case HUMAN_WEAPON_LAW:
		return CheckRocketAttack( flDot, flDist );
		break;

	default:
		return CheckBulletAttack( flDot, flDist );
		break;
	}
}


//=========================================================
// CheckRangeAttack2 - handgrenades
//=========================================================

BOOL CBaseHuman :: CheckRangeAttack2 ( float flDot, float flDist )
{
	if(m_bSurrender)
	return FALSE;

	if(bOnFire)
	return FALSE;

//	if (!b_FacingEnemyForShoot)
//	return FALSE;

	if (m_iNumGrenades <= 0)
	return FALSE;

//	if(!m_bWakeUp)
//	return FALSE;

	if ( m_fHandGrenades )
	{
		return CheckTimedGrenadeAttack( flDot, flDist );
	}
	else
	{
		return FALSE;
	}
}


//=========================================================
// CheckMeleeAttack1
//=========================================================

BOOL CBaseHuman :: CheckMeleeAttack1 ( float flDot, float flDist )
{
	if(m_bSurrender)
	return FALSE;

	if(bOnFire)
	return FALSE;

//	if(!m_bWakeUp)
//	return FALSE;

	// Don't repeat kicks too often, unless we have an explosive weapon (i.e. might not have a choice)
	if ( HasConditions( bits_COND_CAN_RANGE_ATTACK1 ) && m_flLastKick + KICK_INTERVAL > gpGlobals->time ) return FALSE;

	// Check if enemy is in range
	CBaseMonster *pEnemy;

	if ( m_hEnemy != NULL )
	{
		pEnemy = m_hEnemy->MyMonsterPointer();

		if ( !pEnemy )
		{
			return FALSE;
		}
	}

	if ( flDist <= HUMAN_KICK_RANGE && flDot >= 0.7	&& 
		 pEnemy->Classify() != CLASS_ALIEN_BIOWEAPON &&
		 pEnemy->Classify() != CLASS_PLAYER_BIOWEAPON )
	{
		return TRUE;
	}
	return FALSE;
}


//=========================================================
// FCanCheckAttacks - this is overridden for humans
// because they can throw/shoot grenades when they can't see their
// target and the base class doesn't check attacks if the monster
// cannot see its enemy.
//
// !!!BUGBUG - this gets called before a 3-round burst is fired
// which means that a friendly can still be hit with up to 2 rounds. 
// ALSO, grenades will not be tossed if there is a friendly in front,
// this is a bad bug. Friendly machine gun fire avoidance
// will unecessarily prevent the throwing of a grenade as well.
//=========================================================

BOOL CBaseHuman :: FCanCheckAttacks ( void )
{
	if(m_bSurrender)
	return FALSE;

	if(bOnFire)
	return FALSE;

	//aparentely this doesn't work
/*
	if ( pev->weapons == HUMAN_WEAPON_AWP )
	{
		if (m_hEnemy->IsMoving())
			return FALSE;
		else
			return TRUE;
	}
*/
//	if(!m_bWakeUp)
//	return FALSE;

	if ( !HasConditions( bits_COND_ENEMY_TOOFAR ) )
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


//=========================================================
// ISoundMask - returns a bit mask indicating which types
// of sounds this monster regards. 
//=========================================================

int CBaseHuman :: ISoundMask ( void) 
{
	// If friendly to player, ignore sounds player makes when I'm talking so I don't keep turning to look at him
	// Also ignore player if I am in combat - I have more important things to worry about

	CBaseEntity *pPlayer = NULL;
	pPlayer = CBaseEntity::Instance( g_engfuncs.pfnPEntityOfEntIndex( 1 ) );

	if ( pPlayer && IRelationship( pPlayer ) < R_NO && 
		( IsTalking() || m_hTalkTarget != NULL || m_MonsterState == MONSTERSTATE_COMBAT ) )
	{
		return	bits_SOUND_WORLD	|
				bits_SOUND_COMBAT	|
				bits_SOUND_CARCASS	|
				bits_SOUND_MEAT		|
				bits_SOUND_GARBAGE	|
				bits_SOUND_DANGER;
	}
	else
	{
		return	bits_SOUND_WORLD	|
				bits_SOUND_COMBAT	|
				bits_SOUND_CARCASS	|
				bits_SOUND_MEAT		|
				bits_SOUND_GARBAGE	|
				bits_SOUND_DANGER	|
				bits_SOUND_PLAYER;
	}
}

//=========================================================
// GibMonster - make gun fly through the air.
//=========================================================

void CBaseHuman :: GibMonster ( void )
{
	Vector	vecGunPos;
	Vector	vecGunAngles;
/*
	if ( pev->weapons != HUMAN_WEAPON_NONE )
	{// throw a gun if he has one
		GetAttachment( 0, vecGunPos, vecGunAngles );
		
		CBaseEntity *pGun;
		pGun = DropItem( WeaponEntityName(), vecGunPos, vecGunAngles );
		if ( pGun )
		{
			pGun->pev->velocity = Vector (RANDOM_FLOAT(-100,100), RANDOM_FLOAT(-100,100), RANDOM_FLOAT(200,300));
			pGun->pev->avelocity = Vector ( 0, RANDOM_FLOAT( 200, 400 ), 0 );
		}
		pev->weapons = HUMAN_WEAPON_NONE;
	}*/

	CBaseMonster :: GibMonster();
}


//=========================================================
// IsTalking - am I saying a sentence right now?
//=========================================================

BOOL CBaseHuman :: IsTalking( void )
{
	if ( m_flStopTalkTime > gpGlobals->time )
	{
		return TRUE;
	}

	return FALSE;
}


//=========================================================
// FindNearestFriend
// Scan for nearest visible friend - including those not in squad
//=========================================================

CBaseEntity *CBaseHuman :: FindNearestFriend(BOOL fPlayer)
{
	CBaseEntity *pFriend = NULL;
	CBaseEntity *pNearest = NULL;
	float range = 10000000.0;
	TraceResult tr;
	Vector vecStart = pev->origin;
	Vector vecCheck;
	int i;
	char *pszFriend;

	vecStart.z = pev->absmax.z;
	
	// for each type of friend...

	for (i = m_nNumFriendTypes - 1; i > -1; i--)
	{
		pszFriend = m_szFriends[i];

		if (!pszFriend)
			continue;

		if (!fPlayer && pszFriend == "player")
			continue;

		// for each friend in this bsp...
		while (pFriend = UTIL_FindEntityByClassname( pFriend, pszFriend ))
		{
			if (pFriend == this )
				// don't talk to self - sound advice for anyone I would have thought
				continue;

			CBaseMonster *pMonster = pFriend->MyMonsterPointer();

			// If not a monster for some reason, or in a script, or prone
			if ( !pMonster || pMonster->m_MonsterState == MONSTERSTATE_SCRIPT || pMonster->m_MonsterState == MONSTERSTATE_PRONE )
				continue;

			vecCheck = pFriend->pev->origin;
			vecCheck.z = pFriend->pev->absmax.z;

			// if closer than previous friend, and in range, see if he's visible

			if (range > (vecStart - vecCheck).Length())
			{
				UTIL_TraceLine(vecStart, vecCheck, ignore_monsters, ENT(pev), &tr);

				if (tr.flFraction == 1.0)
				{
					// visible and in range, this is the new nearest friend
					if ((vecStart - vecCheck).Length() < TALKRANGE_MIN)
					{
						pNearest = pFriend;
						range = (vecStart - vecCheck).Length();
					}
				}
			}
		}
	}
	return pNearest;
}


//=========================================================
// EnumFriends - Loop through friends, includes those not in squad
//=========================================================

CBaseEntity	*CBaseHuman::EnumFriends( CBaseEntity *pPrevious, int listNumber, BOOL bTrace )
{
	CBaseEntity *pFriend = pPrevious;
	char *pszFriend;
	TraceResult tr;
	Vector vecCheck;

	pszFriend = m_szFriends[ listNumber ];
	while (pFriend = UTIL_FindEntityByClassname( pFriend, pszFriend ))
	{
		if (pFriend == this || !pFriend->IsAlive())
			// don't talk to self or dead people
			continue;
		if ( bTrace )
		{
			vecCheck = pFriend->pev->origin;
			vecCheck.z = pFriend->pev->absmax.z;

			UTIL_TraceLine( pev->origin, vecCheck, ignore_monsters, ENT(pev), &tr);
		}
		else
			tr.flFraction = 1.0;

		if (tr.flFraction == 1.0)
		{
			return pFriend;
		}
	}

	return NULL;
}


//=========================================================
// StopFollowing
//=========================================================

void CBaseHuman::StopFollowing( BOOL clearSchedule )
{
	if ( IsFollowing() )
	{
		if ( IsFollowingPlayer() )
		{
			m_nLastSquadCommand = SQUADCMD_NONE;
			
			if ( !HasMemory( bits_MEMORY_PROVOKED ) && m_hTargetEnt->IsAlive() )
			{
				PlayLabelledSentence( "UNUSE" );
				m_hTalkTarget = m_hTargetEnt;
			}
		}	

		if ( m_movementGoal == MOVEGOAL_TARGETENT )
			RouteClear(); // Stop him from walking toward the leader
		m_hTargetEnt = NULL;
		if ( clearSchedule && SafeToChangeSchedule() )
			ClearSchedule();
		if ( m_hEnemy != NULL )
			m_IdealMonsterState = MONSTERSTATE_COMBAT;
	}
}


//=========================================================
// StartFollowing
//=========================================================

void CBaseHuman::StartFollowing( CBaseEntity *pLeader )
{
	if (m_hTargetEnt == pLeader) return; // Don't start following someone I'm already following

	if ( m_pCine )
		m_pCine->CancelScript();

	if ( m_hEnemy != NULL )
		m_IdealMonsterState = MONSTERSTATE_ALERT;

	m_hTargetEnt = pLeader;
	m_hTalkTarget = m_hTargetEnt;
	ClearConditions( bits_COND_PUSHED );

	if ( SafeToChangeSchedule() )
	{
		ClearSchedule();
	}
}


//=========================================================
// LimitFollowers - otherwise it could get really silly
//=========================================================

void CBaseHuman::LimitFollowers( CBaseEntity *pCaller, int maxFollowers )
{
	CBaseEntity *pFriend = NULL;
	int i, count;

	count = 0;
	// for each friend in this bsp...
	for ( i = 0; i < m_nNumFriendTypes; i++ )
	{
		while (pFriend = EnumFriends( pFriend, i, FALSE ))
		{
			CBaseMonster *pMonster = pFriend->MyMonsterPointer();
			if ( pMonster )
			{
				if ( pMonster->m_hTargetEnt == pCaller )
				{
					count++;
					if ( count > maxFollowers )
						pMonster->StopFollowing( TRUE );
				}
			}
		}
	}
}


//=========================================================
// CanFollow
//=========================================================

BOOL CBaseHuman::CanFollow( void )
{
	if ( m_MonsterState == MONSTERSTATE_SCRIPT )
	{
		if ( !m_pCine->CanInterrupt() )
			return FALSE;
	}

	if ( pev->movetype == MOVETYPE_FLY ) return FALSE;
	
	if ( !IsAlive() || pev->deadflag == DEAD_DYING )
		return FALSE;

	return !IsFollowingPlayer();
}


//=========================================================
// SquadCmdLegalForNonLeader - returns true if Cmd can be
// given by members of the squad who are not the 
// designated leader
//=========================================================

BOOL CBaseHuman :: SquadCmdLegalForNonLeader( SquadCommand Cmd )
{
	if ( Cmd == SQUADCMD_FOUND_ENEMY ||
		 Cmd == SQUADCMD_DEFENSE || 
		 Cmd == SQUADCMD_DISTRESS ) return TRUE;
	
	return FALSE;
}


//=========================================================
// SquadIssueCommand - Issues a command to everyone in the
// squad
//=========================================================

void CBaseHuman :: SquadIssueCommand ( SquadCommand Cmd )
{
	if (!InSquad())
		return;

	if (IsFollowingPlayer() && !SquadCmdLegalForNonLeader( Cmd )) 
		return;

	CSquadMonster *pSquadLeader = MySquadLeader( );
	if ( !pSquadLeader ) return;

	CBaseHuman *pHumanLeader = pSquadLeader->MyHumanPointer();
	if ( !pHumanLeader ) return;

	// Squad Leader refuses your request if the command has a lower priority than the one he is currently on
	if ( Cmd < pHumanLeader->m_nLastSquadCommand ) return;

	switch ( Cmd )
	{
	case SQUADCMD_NONE:					ALERT( at_console, "%s Issuing Squad Command: None\n", STRING( pev->classname ) ); break;
	case SQUADCMD_CHECK_IN:				ALERT( at_console, "%s Issuing Squad Command: Check In\n", STRING( pev->classname ) ); break;
	case SQUADCMD_SEARCH_AND_DESTROY:	ALERT( at_console, "%s Issuing Squad Command: Search and Destroy\n", STRING( pev->classname ) ); break;
	case SQUADCMD_OUTTA_MY_WAY:			ALERT( at_console, "%s Issuing Squad Command: Outta my way\n", STRING( pev->classname ) ); break;
	case SQUADCMD_FOUND_ENEMY:			ALERT( at_console, "%s Issuing Squad Command: Found Enemy\n", STRING( pev->classname ) ); break;
	case SQUADCMD_DISTRESS:				ALERT( at_console, "%s Issuing Squad Command: Distress\n", STRING( pev->classname ) ); break;
	case SQUADCMD_COME_HERE:			ALERT( at_console, "%s Issuing Squad Command: Come here\n", STRING( pev->classname ) ); break;
	case SQUADCMD_SURPRESSING_FIRE:		ALERT( at_console, "%s Issuing Squad Command: Surpressing Fire\n", STRING( pev->classname ) ); break;
	case SQUADCMD_ATTACK:				ALERT( at_console, "%s Issuing Squad Command: Attack\n", STRING( pev->classname ) ); break;
	case SQUADCMD_DEFENSE:				ALERT( at_console, "%s Issuing Squad Command: Defense\n", STRING( pev->classname ) ); break;
	case SQUADCMD_RETREAT:				ALERT( at_console, "%s Issuing Squad Command: Retreat\n", STRING( pev->classname ) ); break;
	case SQUADCMD_GET_DOWN:				ALERT( at_console, "%s Issuing Squad Command: Get Down\n", STRING( pev->classname ) ); break;
	case SQUADCMD_BEHIND_YOU:			ALERT( at_console, "%s Issuing Squad Command: Behind You\n", STRING( pev->classname ) ); break;
	}
	
	for (int i = 0; i < MAX_SQUAD_MEMBERS; i++)
	{
		CSquadMonster *pMember = pHumanLeader->MySquadMember(i);
		if (pMember && ( pMember->pev->origin - pev->origin).Length() <= SQUAD_COMMAND_RANGE )
		{
			CBaseHuman *pHuman = pMember->MyHumanPointer();
			if ( pHuman && ( !pHuman->IsFollowingPlayer() || SquadCmdLegalForNonLeader( Cmd ) ) )
			{
				pHuman->SquadReceiveCommand( Cmd );
			}
		}
	}
}


//=========================================================
// SquadReceiveCommand - Receives a command and takes an
// appropriate action
//=========================================================

void CBaseHuman :: SquadReceiveCommand( SquadCommand Cmd )
{
	if ( !IsAlive() || pev->deadflag == DEAD_DYING ) return;

	m_fSquadCmdAcknowledged = FALSE;

	switch ( Cmd )
	{
	case SQUADCMD_ATTACK:				// Attack a designated target, if you are not attacking something else
		
		m_nLastSquadCommand = Cmd;
		m_flLastSquadCmdTime = gpGlobals->time;
		if ( !SafeToChangeSchedule() ) break;
		
		if ( !HasConditions( bits_COND_SEE_ENEMY) )
		{
			if ( SquadGetCommanderEnemy() )	// Try and set my enemy to my commander's enemy
			{ 
				if ( FOkToShout() )
				{
					PlayLabelledSentence( "AFFIRMATIVE" );
					m_fSquadCmdAcknowledged = TRUE;
				}
				ClearSchedule();
			}
			else
			{
				SquadReceiveCommand( SQUADCMD_SEARCH_AND_DESTROY ); // If I can't, try to search for a new enemy
			}
		}
		break;

	case SQUADCMD_SEARCH_AND_DESTROY:	// Search for a target to attack
		
		m_nLastSquadCommand = Cmd;
		m_flLastSquadCmdTime = gpGlobals->time;
		if ( !SafeToChangeSchedule() ) break;

		if ( !HasConditions( bits_COND_SEE_ENEMY) )
		{
			ClearSchedule();	// If not fighting, dump out of current schedule
		}
		break;

	case SQUADCMD_DEFENSE:				// Defend a designated squad member (i.e. someone who is wounded)
		
		m_nLastSquadCommand = Cmd;
		m_flLastSquadCmdTime = gpGlobals->time;
		if ( !SafeToChangeSchedule() ) break;

		if ( !HasConditions( bits_COND_SEE_ENEMY) )
		{
			ClearSchedule();	// If not fighting, dump out of current schedule
		}
		break;
	
	case SQUADCMD_RETREAT:				// Run away from whatever you are attacking
		
		m_nLastSquadCommand = Cmd;
		m_flLastSquadCmdTime = gpGlobals->time;
		if ( !SafeToChangeSchedule() ) break;

		if ( m_hEnemy != NULL )
		{
			// Change schedule immediately as this could be important
			ChangeSchedule( GetScheduleOfType( SCHED_HUMAN_RETREAT ) );	
		}
		else
		{
			// Have no enemy in Alert or idle state
			ChangeSchedule( GetScheduleOfType( SCHED_TAKE_COVER_FROM_ORIGIN ) );	
		}
		break;
	
	case SQUADCMD_SURPRESSING_FIRE:		// Fire at a designated place
		
		if ( !HasConditions( bits_COND_SEE_ENEMY) )	// If I am already fighting someone ignore this command
		{
			if ( SquadGetCommanderEnemy() ||		// Try and get new enemy
				 SquadGetCommanderLineOfSight() )	// Fire at the position my commander is looking at
			{
				m_nLastSquadCommand = Cmd;
				m_flLastSquadCmdTime = gpGlobals->time;
				if ( !SafeToChangeSchedule() ) break;
				ClearSchedule();
			}
		}
		break;

	case SQUADCMD_COME_HERE:			// Come to Squad Leader
		
		m_nLastSquadCommand = Cmd;
		m_flLastSquadCmdTime = gpGlobals->time;
		if ( !SafeToChangeSchedule() ) break;

		if ( InSquad() && !IsFollowingPlayer() ) 
		{
			StartFollowing( MySquadLeader() );
		}
		if ( !HasConditions( bits_COND_SEE_ENEMY) )
		{
			if ( FOkToShout() )
			{
				PlayLabelledSentence( "AFFIRMATIVE" );
				m_fSquadCmdAcknowledged = TRUE;
			}
			ClearSchedule();	// If not fighting, dump out of current schedule
		}
		break;

	case SQUADCMD_GET_DOWN:				// Crouch	- likely to be only given by player
		if ( SafeToChangeSchedule() )
		{
			ChangeSchedule( GetScheduleOfType( SCHED_COWER ) );
		}
		break;

	case SQUADCMD_BEHIND_YOU:			// Turn around - likely to be only given by player
		if ( SafeToChangeSchedule() )
		{
			ChangeSchedule( GetScheduleOfType( SCHED_HUMAN_TURN_ROUND ) );
		}
		break;

	case SQUADCMD_CHECK_IN:				// Tell everyone to check in

		m_nLastSquadCommand = Cmd;
		m_flLastSquadCmdTime = gpGlobals->time;
		if ( !SafeToChangeSchedule() ) break;

		if ( !HasConditions( bits_COND_SEE_ENEMY )  )	// If I have no enemy, give the all clear
		{
			ChangeSchedule( GetScheduleOfType( SCHED_HUMAN_CHECK_IN ) );
		}
		else	// If I have an enemy, inform my squad
		{
			ChangeSchedule( GetScheduleOfType( SCHED_HUMAN_FOUND_ENEMY ) );
		}
		break;

	case SQUADCMD_FOUND_ENEMY:
		{
			if ( m_hEnemy == NULL || !m_hEnemy->IsAlive() && SquadGetMemberEnemy() && SafeToChangeSchedule() )
			{
				ClearSchedule();
			}
		}
		break;

	case SQUADCMD_DISTRESS:
		{
			if ( m_hEnemy == NULL || !m_hEnemy->IsAlive() && SquadGetMemberEnemy() && SafeToChangeSchedule() )
			{
				ClearSchedule();
			}
		}
		break;
	}
}


//=========================================================
// SquadGetCommanderLineOfSight - Get the position the
// commander is looking at
//=========================================================

BOOL CBaseHuman :: SquadGetCommanderLineOfSight()
{
	CBaseEntity *pCommander = NULL;

	if (IsFollowingPlayer())
	{
		pCommander = UTIL_FindEntityByClassname( NULL, "player" );
	}
	else if (InSquad())
	{
		pCommander = MySquadLeader();
	}

	if ( pCommander == NULL ) return FALSE;

	// Draw a line in front of the commander and point to that place

	UTIL_MakeAimVectors( pCommander->pev->angles );

	TraceResult tr;

//	UTIL_TraceLine( pCommander->EyePosition(), gpGlobals->v_forward * HUMAN_RIFLE_RANGE, dont_ignore_monsters, 
	UTIL_TraceLine( pCommander->EyePosition(), gpGlobals->v_forward * iDistToShoot, dont_ignore_monsters, 
		dont_ignore_glass, ENT(pCommander->pev), &tr);

//	m_vecEnemyLKP = pCommander->EyePosition() + tr.flFraction * gpGlobals->v_forward * HUMAN_RIFLE_RANGE;
	m_vecEnemyLKP = pCommander->EyePosition() + tr.flFraction * gpGlobals->v_forward * iDistToShoot;

	return TRUE;
}


//=========================================================
// SquadGetCommanderEnemy - Get's the commander's enemy
//=========================================================

BOOL CBaseHuman :: SquadGetCommanderEnemy()
{
	if (IsFollowingPlayer())
	{
		// Try and figure out what the player's enemy is
		CBaseEntity *pPlayer = NULL;
		pPlayer = UTIL_FindEntityByClassname( NULL, "player" );

		if (pPlayer)
		{
			CBaseMonster * pPlayerMonPtr = pPlayer->MyMonsterPointer();
			if ( pPlayerMonPtr )
			{
				pPlayerMonPtr->Look( 2048 );

				CBaseEntity * pEnemy = pPlayerMonPtr->BestVisibleEnemy();
				if ( pEnemy )
				{
					if ( m_hEnemy != NULL ) 
					{
						// remember the current enemy
						PushEnemy( m_hEnemy, m_vecEnemyLKP );
					}
					// get a new enemy
					m_hEnemy = pEnemy;
					m_vecEnemyLKP = pEnemy->pev->origin;
					CheckEnemy( pEnemy );
					SetConditions ( bits_COND_NEW_ENEMY );

					return TRUE;
				}
			}
		}
	}
	
	if (InSquad())
	{
		// Get Squad Leader's enemy

		CSquadMonster *pSquadLeader = MySquadLeader();
		if ( !pSquadLeader ) return FALSE;

		CBaseHuman *pCommander = pSquadLeader->MyHumanPointer();
		if ( !pCommander ) return FALSE;

		if ( pCommander->m_hEnemy == NULL ) return FALSE;

		if ( m_hEnemy != NULL ) 
		{
			// remember the current enemy
			PushEnemy( m_hEnemy, m_vecEnemyLKP );
		}
		// get a new enemy
		m_hEnemy = pCommander->m_hEnemy;
		m_vecEnemyLKP = m_hEnemy->pev->origin;
		CheckEnemy( m_hEnemy );
		SetConditions ( bits_COND_NEW_ENEMY );

		return TRUE;
	}
	
	return FALSE;
}


//=========================================================
// SquadGetWounded - Gets the guy with the lowest health,
// or returns false if they are all over 66%
//=========================================================

BOOL CBaseHuman :: SquadGetWounded()
{
	if ( InSquad() && !IsFollowingPlayer() )
	{
		CSquadMonster *pWounded = NULL;
		CSquadMonster *pSquadLeader = MySquadLeader();
		if ( !pSquadLeader ) return FALSE;

		for (int i = 0; i < MAX_SQUAD_MEMBERS; i++)
		{
			CSquadMonster *pMember = pSquadLeader->MySquadMember(i);
			if (pMember)
			{
				if ( !pWounded && !( pMember->pev->health > 2*pMember->pev->max_health / 3 ) )
				{
					pWounded = pMember;
				}
				else if ( pWounded && pWounded->pev->health > pMember->pev->health )
				{
					pWounded = pMember;
				}
			}
		}

		if ( pWounded && pWounded != this )  // No need to follow myself
		{
			StartFollowing( pWounded );
			return TRUE;	// if you found someone with low enough health, start following them and return true
		}
		else
		{
			return FALSE;
		}
	}
	else if (IsFollowingPlayer())
	{
		// NOTE - Try and find most damaged follower of player

		return FALSE;
	}
	else return FALSE;
}


//=========================================================
// SquadIsHealthy - determine whether the squad is more or less
// fit for action
//=========================================================

BOOL CBaseHuman :: SquadIsHealthy()
{
	if (!InSquad())
		return ( pev->health > ( 2 * pev->max_health ) / 3 );

	int cnt = 0;

	CSquadMonster *pSquadLeader = MySquadLeader( );
	if ( !pSquadLeader ) return ( pev->health > ( 2 * pev->max_health ) / 3 );

	for (int i = 0; i < MAX_SQUAD_MEMBERS; i++)
	{
		CSquadMonster *pMember = pSquadLeader->MySquadMember(i);
		if (pMember)
		{
			// If one member of the squad is very sick (lower than 33%) then squad is not healthy
			if ( pMember->pev->health < pMember->pev->max_health / 3 ) return FALSE;

			cnt += pMember->pev->health;
		}
	}

	// If average health of each member is higher than 66% then squad is healthy
	return cnt > ( 2 * pev->max_health * pSquadLeader->SquadCount() ) / 3;
}


//=========================================================
// SquadGetMemberEnemy - Find a squad member who has an 
// enemy and adopt his enemy
//=========================================================

BOOL CBaseHuman :: SquadGetMemberEnemy()
{
	if (!InSquad()) return FALSE;

	CSquadMonster *pSquadLeader = MySquadLeader( );
	if ( !pSquadLeader ) return FALSE;

	for (int i = 0; i < MAX_SQUAD_MEMBERS; i++)
	{
		CSquadMonster *pMember = pSquadLeader->MySquadMember(i);
		if (pMember && pMember->m_hEnemy != NULL && pMember->m_hEnemy->IsAlive() )
		{
			// get a new enemy
			m_hEnemy = pMember->m_hEnemy;
			m_vecEnemyLKP = pMember->m_vecEnemyLKP;
			SetConditions ( bits_COND_NEW_ENEMY );

			return TRUE;
		}
	}
	return FALSE;
}


//=========================================================
// SquadAnyIdle - Find out if there are any squad members
// who are doing bugger all and who can see me
//=========================================================

BOOL CBaseHuman :: SquadAnyIdle()
{
	if (!InSquad()) return FALSE;

	CSquadMonster *pSquadLeader = MySquadLeader( );
	if ( !pSquadLeader ) return FALSE;

	for (int i = 0; i < MAX_SQUAD_MEMBERS; i++)
	{
		CSquadMonster *pMember = pSquadLeader->MySquadMember(i);
		if (pMember && pMember->IsAlive() && 
			( pMember->m_MonsterState == MONSTERSTATE_IDLE || pMember->m_MonsterState == MONSTERSTATE_ALERT )
			&& pMember->FVisible( this ))
		{
			return TRUE;
		}
	}
	return FALSE;
}


//=========================================================
// SquadIsScattered - is my squad scattered, or more accurately
// are they a long way away from me?
//=========================================================

BOOL CBaseHuman :: SquadIsScattered()
{
	if (!InSquad()) return FALSE;

	float cnt = 0;

	CSquadMonster *pSquadLeader = MySquadLeader( );
	if ( !pSquadLeader ) return FALSE;

	for (int i = 0; i < MAX_SQUAD_MEMBERS; i++)
	{
		CSquadMonster *pMember = pSquadLeader->MySquadMember(i);
		if (pMember)
		{
			float flDist = (pMember->pev->origin - pev->origin).Length();

			// If one member of the squad is very far away (further than 768) then squad is scattered
			if ( flDist > 768 ) return FALSE;

			cnt += flDist;
		}
	}

	// If average distance of each member is further than 512 then squad is scattered
	return cnt > ( 512 * pSquadLeader->SquadCount() );
}


//=========================================================
// StartTask
//=========================================================

void CBaseHuman :: StartTask( Task_t *pTask )
{
	switch ( pTask->iTask )
	{
/*	case TASK_FIND_COVER_FROM_ENEMY:
		{
			entvars_t *pevCover;

			if ( m_hEnemy == NULL )
			{
				// Find cover from self if no enemy available
				pevCover = pev;
//				TaskFail();
//				return;
			}
			else
				pevCover = m_hEnemy->pev;

			if ( FindLateralCover( pevCover->origin, pevCover->view_ofs ) )
			{
				// try lateral first
				m_flMoveWaitFinished = gpGlobals->time + pTask->flData;
				TaskComplete();
			}

			else
			{
				// no coverwhatsoever.
				TaskFail();
			}
			break;
		}
	break;*/

	case TASK_RELOAD:
		m_IdealActivity = ACT_RELOAD;
		break;

	case TASK_HUMAN_CHECK_FIRE:
		if ( NoFriendlyFire() )
		{
			TaskComplete();
		}
		else
		{
			TaskFail();
		}
		break;

	case TASK_HUMAN_CROUCH:
	{
		if( _STRAFE_CODE_TEST )
		{
			m_fCrouching = FALSE;//just in case
			TaskComplete();
		}

		m_flCrouchTime = gpGlobals->time + CROUCH_TIME;
		
		if( bFistsReady )
		TaskComplete();

		if(m_bSurrender)//can't crouch if surrender
		TaskComplete();
		
		if (m_bKamikaze)
		TaskComplete();

		if(bOnFire)
		TaskComplete();

		if ( m_hEnemy != NULL && m_hEnemy->IsAlive() )//if the enemy is close, don't crouch!
		{
			float flDist = (m_hEnemy->pev->origin - pev->origin).Length();

			if ( flDist > 768 ) 
			TaskComplete();
		}

		//don't crouch, can't crouch if we have a pistol
		switch ( pev->weapons )
		{
			case HUMAN_WEAPON_DESERT:
			case HUMAN_WEAPON_PISTOL:
			{
				m_fCrouching = FALSE;//just in case
				TaskComplete();
				
				ALERT ( at_console, "\n\n\nPISTOL DETECTED CANT CROUCH!\n\n\n" );	
			}
			break;
		}

		if (m_fCrouching || FBitSet( pev->spawnflags, SF_MONSTER_PREDISASTER ) || pev->waterlevel == 3)
		{
			TaskComplete();
		}
		else
		{
			m_IdealActivity = ACT_CROUCH;
		}
		break;
	}
	break;

	case TASK_HUMAN_SURRENDER:
		if(pev->spawnflags & 4096)
			return;

		if (m_bSurrender)
		{
			TaskComplete();
		}
		else
		{
			m_IdealActivity = ACT_SURRENDER;
		}
		break;

	case TASK_HUMAN_SIDESTEP_RIGHT:
		if (m_bSideStepRightDone)
		{
			TaskComplete();
			m_bSideStepRightDone = false;//clear for next uses
		}
		else
		{
		//	PossessEntity();

			m_IdealActivity = ACT_SIDESTEP_RIGHT;

		/*		Vector2D	vec2DirToPoint; 
				Vector2D	vec2RightSide;

				// to start strafing, we have to first figure out if the target is on the left side or right side
				UTIL_MakeVectors ( pev->angles );

				vec2DirToPoint = ( m_Route[ 0 ].vecLocation - pev->origin ).Make2D().Normalize();
				vec2RightSide = gpGlobals->v_right.Make2D().Normalize();

				if ( DotProduct ( vec2DirToPoint, vec2RightSide ) > 0 )
				{
					// strafe right
					m_movementActivity = ACT_STRAFE_RIGHT;//ACT_WALK
				}
				else
				{
					// strafe left
					m_movementActivity = ACT_STRAFE_LEFT;//ACT_WALK
				}*/
		}
		break;

	case TASK_HUMAN_SIDESTEP_LEFT:
		if (m_bSideStepLeftDone)
		{
			TaskComplete();
			m_bSideStepLeftDone = false;//clear for next uses
		}
		else
		{
		//	PossessEntity();

			m_IdealActivity = ACT_SIDESTEP_LEFT;

		/*		Vector2D	vec2DirToPoint; 
				Vector2D	vec2RightSide;

				// to start strafing, we have to first figure out if the target is on the left side or right side
				UTIL_MakeVectors ( pev->angles );

				vec2DirToPoint = ( m_Route[ 0 ].vecLocation - pev->origin ).Make2D().Normalize();
				vec2RightSide = gpGlobals->v_right.Make2D().Normalize();

				if ( DotProduct ( vec2DirToPoint, vec2RightSide ) > 0 )
				{
					// strafe right
					m_movementActivity = ACT_STRAFE_RIGHT;//ACT_WALK
				}
				else
				{
					// strafe left
					m_movementActivity = ACT_STRAFE_LEFT;//ACT_WALK
				}*/
		}
		break;

	case TASK_HUMAN_ONFIRE:
		if (bOnFire)
		{
			TaskComplete();
		}
		else
		{		
			m_IdealActivity = ACT_ON_FIRE;
		}
		break;

	case TASK_HUMAN_WAKEUP:
/*		if (m_bWakeUp)
		{
			TaskComplete();
		}
		else
		{
			m_IdealActivity = ACT_WAKEUP;
		}
*/		break;

	case TASK_HUMAN_DROP_WEAPON:
		if (m_bDropped )
		{
			TaskComplete();
		}
		else
		{
			m_IdealActivity = ACT_DROP_GUN;
		}
		break;

	case TASK_HUMAN_UNCROUCH:
		if (!m_fCrouching)
		{
			TaskComplete();
		}
		else
		{
			m_IdealActivity = ACT_UNCROUCH;
		}
		break;

	case TASK_HUMAN_SOUND_EXPL:
		{
			if ( (pTask->flData == 0 || RANDOM_FLOAT(0,1) <= pTask->flData)/* && m_nHeadNum != GetNumHeads() */)
			{
				PlayLabelledSentence( "EXPL" );
				SquadIssueCommand( SQUADCMD_DISTRESS ); 
			}
			TaskComplete();
		}
		break;

	case TASK_HUMAN_SOUND_GRENADE:
		{
			if ( (pTask->flData == 0 || RANDOM_FLOAT(0,1) <= pTask->flData) &&
				FOkToShout())
			{
				PlayLabelledSentence( "GREN" );
			}
			TaskComplete();
		}
		break;

	case TASK_HUMAN_SOUND_HEAR:
		{
			if ( (pTask->flData == 0 || RANDOM_FLOAT(0,1) <= pTask->flData) &&
				 !FBitSet( m_bitsSaid, bit_saidHeard ) && FOkToSpeak() )
			{
				PlayLabelledSentence( "HEAR" );
				SetBits( m_bitsSaid, bit_saidHeard );
					
				CBasePlayer *pPlayer = NULL;
				pPlayer = (CBasePlayer *)UTIL_PlayerByIndex( 1 );
				if ( pPlayer )
					pPlayer->PlayActionMusic();
			}
			TaskComplete();
		}
		break;

	case TASK_HUMAN_SOUND_VICTORY:
		{
			if ( (pTask->flData == 0 || RANDOM_FLOAT(0,1) <= pTask->flData) &&
				 FOkToShout() )
			{
				PlayLabelledSentence( "KILL" );
			}
			TaskComplete();
		}
		break;

	case TASK_HUMAN_SOUND_MEDIC:
		{
			if ( (pTask->flData == 0 || RANDOM_FLOAT(0,1) <= pTask->flData) &&
				 FOkToShout() )
			{
				PlayLabelledSentence( "MEDIC" );
			}
			TaskComplete();
		}
		break;

	case TASK_HUMAN_SOUND_HELP:
		{
			if ( (pTask->flData == 0 || RANDOM_FLOAT(0,1) <= pTask->flData) &&
				 FOkToShout() )
			{
				PlayLabelledSentence( "HELP" );
				SquadIssueCommand( SQUADCMD_DEFENSE );
							
				CBasePlayer *pPlayer = NULL;
				pPlayer = (CBasePlayer *)UTIL_PlayerByIndex( 1 );
				if ( pPlayer )
					pPlayer->PlayActionMusic();
			}
			TaskComplete();
		}
		break;

	case TASK_HUMAN_SOUND_CHECK_IN:
		{
			if ( (pTask->flData == 0 || RANDOM_FLOAT(0,1) <= pTask->flData) &&
				 FOkToShout() )
			{
				m_flWaitFinished = gpGlobals->time + 1;
					
				if ( (bTerrorist) && (RANDOM_FLOAT ( 0 , 1 ) < 0.9 ) && ( m_szSpeechLabel == "FM_" ) )
				DetectEnviroment();
				else
				PlayLabelledSentence( "CHECK" );
							
				CBasePlayer *pPlayer = NULL;
				pPlayer = (CBasePlayer *)UTIL_PlayerByIndex( 1 );
				if ( pPlayer )
					pPlayer->PlayActionMusic();
			}
			else
			{
				TaskComplete();
			}
		}
		break;

	case TASK_HUMAN_SOUND_CLEAR:
		{
			CSquadMonster *pSquadLeader = MySquadLeader();

			if ( InSquad() && pSquadLeader != this )
			{
				int i = 0;
				for (i = 0; i < MAX_SQUAD_MEMBERS; i++ ) if ( pSquadLeader->m_hSquadMember[ i ] == this ) break;

				m_flWaitFinished = gpGlobals->time + i;
			}
			else
			{
				TaskFail();
			}
		}
		break;

	case TASK_HUMAN_SOUND_ATTACK:
		{
			if ( (pTask->flData == 0 || RANDOM_FLOAT(0,1) <= pTask->flData) &&
				 FOkToShout() )	
			{
				if ( (bTerrorist) && (RANDOM_FLOAT ( 0 , 1 ) < 0.9 ) && ( m_szSpeechLabel == "FM_" ) )
				DetectEnviroment();
				else
				PlayLabelledSentence( "ATTACK" );
				m_IdealActivity = ACT_SIGNAL1;	// Advance signal
							
				CBasePlayer *pPlayer = NULL;
				pPlayer = (CBasePlayer *)UTIL_PlayerByIndex( 1 );
				if ( pPlayer )
					pPlayer->PlayActionMusic();
			}
			else
			{
				TaskComplete();
			}
		}
		break;

	case TASK_HUMAN_SOUND_FOUND_ENEMY:
			if ( (pTask->flData == 0 || RANDOM_FLOAT(0,1) <= pTask->flData) &&
				 FOkToShout() )	
			{
				PlayLabelledSentence( "FOUND" );
				m_IdealActivity = ACT_SIGNAL1;	// Advance signal
			}
			else
			{
				TaskComplete();
			}
		break;

	case TASK_HUMAN_SOUND_SURPRESS:
		{
			if ( (pTask->flData == 0 || RANDOM_FLOAT(0,1) <= pTask->flData) &&
				 FOkToShout())
			{
				PlayLabelledSentence( "SURPRESS" );
				m_IdealActivity = ACT_SIGNAL1;	// Advance signal
			}
			else
			{
				TaskComplete();
			}
		}
		break;

	case TASK_HUMAN_SOUND_SURPRESSING:
		{
			if ( (pTask->flData == 0 || RANDOM_FLOAT(0,1) <= pTask->flData) &&
				!m_fSquadCmdAcknowledged && FOkToShout())
			{
				m_hTalkTarget = MySquadLeader();
				PlayLabelledSentence( "SURPRESSING" );
				m_fSquadCmdAcknowledged = TRUE;
			}
			TaskComplete();
		}
		break;

	case TASK_HUMAN_SOUND_SEARCHING:
		{
			if ( (pTask->flData == 0 || RANDOM_FLOAT(0,1) <= pTask->flData) &&
				!m_fSquadCmdAcknowledged && FOkToShout())
			{
				m_hTalkTarget = MySquadLeader();
				PlayLabelledSentence( "SEARCHING" );
				m_fSquadCmdAcknowledged = TRUE;
			}
			TaskComplete();
		}
		break;

	case TASK_HUMAN_SOUND_RETREATING:
		{
			if ( (pTask->flData == 0 || RANDOM_FLOAT(0,1) <= pTask->flData) &&
				!m_fSquadCmdAcknowledged && FOkToShout())
			{
				m_hTalkTarget = MySquadLeader();
				PlayLabelledSentence( "RETREATING" );
				m_fSquadCmdAcknowledged = TRUE;
			}
			TaskComplete();
		}
		break;

	case TASK_HUMAN_SOUND_AFFIRMATIVE:
		{
			if ( (pTask->flData == 0 || RANDOM_FLOAT(0,1) <= pTask->flData) &&
				!m_fSquadCmdAcknowledged && FOkToShout())
			{
				m_hTalkTarget = MySquadLeader();
				PlayLabelledSentence( "AFFIRMATIVE" );
				m_fSquadCmdAcknowledged = TRUE;
			}
			TaskComplete();
		}
		break;

	case TASK_HUMAN_SOUND_THROW:
		{
			if ( (pTask->flData == 0 || RANDOM_FLOAT(0,1) <= pTask->flData) &&
				FOkToShout())
			{
				PlayLabelledSentence( "THROW" );
			}
			TaskComplete();
		}
		break;

	case TASK_HUMAN_SOUND_COVER:
		{
			if ( (pTask->flData == 0 || RANDOM_FLOAT(0,1) <= pTask->flData) &&
				FOkToShout() )
			{
				m_hTalkTarget = MySquadLeader();
				PlayLabelledSentence( "COVER" );
			}
			TaskComplete();
		}
		break;

	case TASK_HUMAN_SOUND_RETREAT:
		{
			if ( (pTask->flData == 0 || RANDOM_FLOAT(0,1) <= pTask->flData) &&
				FOkToShout())
			{
				m_IdealActivity = ACT_SIGNAL3;	// Retreat signal
				PlayLabelledSentence( "RETREAT" );
			}
			else
			{
				TaskComplete();
			}
		}
		break;

	case TASK_HUMAN_SOUND_SEARCH_AND_DESTROY:
		{
			if ( (pTask->flData == 0 || RANDOM_FLOAT(0,1) <= pTask->flData) &&
				FOkToShout())
			{
				m_IdealActivity = ACT_SIGNAL2;	// Flank signal
				PlayLabelledSentence( "SEARCH" );
			}
			else
			{
				TaskComplete();
			}
		}
		break;

	case TASK_HUMAN_SOUND_COME_TO_ME:
		{
			if ( (pTask->flData == 0 || RANDOM_FLOAT(0,1) <= pTask->flData) &&
				FOkToShout())
			{
				m_IdealActivity = ACT_SIGNAL3;	// Retreat signal
				PlayLabelledSentence( "COME" );
			}
			else
			{
				TaskComplete();
			}
		}
		break;

	case TASK_HUMAN_SOUND_RESPOND:
		{
			if ( (pTask->flData == 0 || RANDOM_FLOAT(0,1) <= pTask->flData) &&
				FOkToSpeak())
			{
				PlayLabelledSentence( "ANSWER" );
			}
			TaskComplete();
		}
		break;

	case TASK_HUMAN_SOUND_CHARGE:
		{
			if ( (pTask->flData == 0 || RANDOM_FLOAT(0,1) <= pTask->flData) &&
				 FOkToShout() && m_hEnemy != NULL )	
			{
				m_hTalkTarget = m_hEnemy;
				switch ( m_hEnemy->Classify() )
				{
				case CLASS_ALIEN_MONSTER: 
					PlayLabelledSentence( "ZOMBIE" );
					break;
			
				default:
					PlayLabelledSentence( "CHARGE" );
					break;
				}
			}
			TaskComplete();
		}
		break;

	case TASK_HUMAN_SOUND_TAUNT:
		{
			if ( (pTask->flData == 0 || RANDOM_FLOAT(0,1) <= pTask->flData) &&
				FOkToShout())
			{
				m_hTalkTarget = m_hEnemy;
				PlayLabelledSentence( "TAUNT" );
			}
			TaskComplete();
		}
		break;

	case TASK_HUMAN_SOUND_HEALED:
		{
			if ( (pTask->flData == 0 || RANDOM_FLOAT(0,1) <= pTask->flData) &&
				FOkToShout())
			{
				m_hTalkTarget = m_hTargetEnt;
				PlayLabelledSentence( "HEALED" );
			}
			TaskComplete();
		}
		break;

	case TASK_HUMAN_FIND_MEDIC:
		{
			// First try looking for a medic in my squad

			if ( InSquad() )
			{
				CSquadMonster *pSquadLeader = MySquadLeader( );
				if ( pSquadLeader ) for (int i = 0; i < MAX_SQUAD_MEMBERS; i++)
				{
					CSquadMonster *pMember = pSquadLeader->MySquadMember(i);
					if ( pMember && pMember != this )
					{
						CBaseHuman *pHuman = pMember->MyHumanPointer();
						if ( pHuman && pHuman->IsMedic() && pHuman->pev->deadflag == DEAD_NO )
						{
							if ( !pHuman->IsFollowing() ) 
							{
								StartFollowing( pMember );
								pHuman->StartFollowing( this );
								TaskComplete();
							}
						}
					}
				}
			}

			// If that doesn't work, just look around and see if I can SEE a medic

			if ( !TaskIsComplete() ) 
			{
				CBaseEntity *pFriend = NULL;
				int i;

				// for each friend in this bsp...
				for ( i = 0; i < m_nNumFriendTypes; i++ )
				{
					while (pFriend = EnumFriends( pFriend, i, TRUE ))
					{
						CBaseHuman *pHuman = pFriend->MyHumanPointer();
						if ( pHuman && pHuman->IsMedic() && pHuman != this && pHuman->pev->deadflag == DEAD_NO )
						{
							if ( !pHuman->IsFollowing() ) 
							{
								StartFollowing( pHuman );
								pHuman->StartFollowing( this );
								TaskComplete();
							}
						}
					}
				}
			}

			// If still can't find one, suffer in silence
			// And don't try to look for one again for a while

			if ( !TaskIsComplete() ) 
			{
				m_flLastMedicSearch = gpGlobals->time;
				TaskFail();
			}
		}
		break;

	case TASK_HUMAN_FORGET_SQUAD_COMMAND:
		{
			m_nLastSquadCommand = SQUADCMD_NONE;
			TaskComplete();
		}
		break;
//sys test
		/*
	case TASK_GET_PATH_TO_ENEMY_LKP:
		{
			// Try and get to a place where you can see the enemy last known position by searching for progressively
			// closer positions to it NOTE - a bit time-consuming, would be better to create a dedicated procedure

			if (BuildNearestRoute( m_vecEnemyLKP, pev->view_ofs, 768, (m_vecEnemyLKP - pev->origin).Length() )
				&& FVisible( m_vecEnemyLKP ) )
			{
				TaskComplete();
			}
			else if (BuildNearestRoute( m_vecEnemyLKP, pev->view_ofs, 512, (m_vecEnemyLKP - pev->origin).Length() )
				&& FVisible( m_vecEnemyLKP ) )
			{
				TaskComplete();
			}
			else if (BuildNearestRoute( m_vecEnemyLKP, pev->view_ofs, 256, (m_vecEnemyLKP - pev->origin).Length() )
				&& FVisible( m_vecEnemyLKP ) )
			{
				TaskComplete();
			}
			else
			{
				// no way to get there =(
				ALERT ( at_aiconsole, "GetPathToEnemyLKP failed!!\n" );
				TaskFail();
			}
		}
		break;
*/
	case TASK_RANGE_ATTACK1:
		{
			if ( HasConditions( bits_COND_NO_AMMO_LOADED ) )
			{
				TaskFail();
			}
			else
			{
				pev->framerate = RANDOM_FLOAT(1.0, 0.1);				
			
				CSquadMonster::StartTask( pTask );	
			}
		}
		break;

	case TASK_HUMAN_EYECONTACT:
		break;

	case TASK_HUMAN_IDEALYAW:
		if (m_hTalkTarget != NULL)
		{
			pev->yaw_speed = 60;
			float yaw = VecToYaw(m_hTalkTarget->pev->origin - pev->origin) - pev->angles.y;

			if (yaw > 180) yaw -= 360;
			if (yaw < -180) yaw += 360;

			if (yaw < 0)
			{
				pev->ideal_yaw = min( yaw + 45, 0 ) + pev->angles.y;
			}
			else
			{
				pev->ideal_yaw = max( yaw - 45, 0 ) + pev->angles.y;
			}
		}
		TaskComplete();
		break;

	case TASK_HUMAN_EXPLOSION_FLY:
		{
			m_IdealActivity = ACT_EXPLOSION_FLY;
			pev->movetype = MOVETYPE_TOSS;
			pev->deadflag = DEAD_DYING;
			SetTouch( &CBaseHuman::ExplFlyTouch );
		}
		break;

	case TASK_RUN_PATH:	// over-ridden because base class checks model file for ACT_RUN, which we don't have
		{
			m_movementActivity = ACT_RUN;
			TaskComplete();
		}
		break;

	case TASK_WALK_PATH:	// over-ridden because base class checks model file for ACT_WALK, which we don't have
		{
			m_movementActivity = ACT_WALK;
			TaskComplete();
		}
		break;

	case TASK_HUMAN_FACE_TOSS_DIR:
		{
		}
		break;

	case TASK_HUMAN_GET_EXPLOSIVE_PATH_TO_ENEMY:
		{
			if ( m_hEnemy == NULL )
			{
				TaskFail();
				return;
			}

			if ( BuildNearestRoute( m_vecEnemyLKP, m_hEnemy->pev->view_ofs, 
				HUMAN_EXPLOSIVE_MIN_RANGE, HUMAN_EXPLOSIVE_MAX_RANGE ) )
			{
				TaskComplete();
			}
			else
			{
				TaskFail();
			}
		}
		break;

	case TASK_HUMAN_GET_MELEE_PATH_TO_ENEMY:
		{
			if ( m_hEnemy == NULL )
			{
				TaskFail();
				return;
			}

			CBaseEntity *pEnemy = m_hEnemy;

			if ( BuildRoute ( pEnemy->pev->origin, bits_MF_TO_ENEMY, pEnemy ) )
			{
				TaskComplete();
			}
			else if ( BuildNearestRoute( pEnemy->pev->origin, pEnemy->pev->view_ofs, 0, HUMAN_KICK_RANGE ) )
			{
				TaskComplete();
			}
			else
			{
				TaskFail();
			}
		}
		break;

	case TASK_HUMAN_RETREAT_FACE:
		{
			if ( m_hEnemy != NULL && HasConditions( bits_COND_CAN_RANGE_ATTACK1 ) )
			{
				MakeIdealYaw( m_hEnemy->pev->origin );
				SetTurnActivity(); 
			}
			else
			{
				float flCurrentYaw;
			
				flCurrentYaw = UTIL_AngleMod( pev->angles.y );
				pev->ideal_yaw = UTIL_AngleMod( flCurrentYaw + pTask->flData );
				SetTurnActivity();
			}
		}
		break;

	case TASK_HUMAN_WAIT_GOAL_VISIBLE:
		{
		}
		break;

	case TASK_RUN_TO_TARGET:
	case TASK_WALK_TO_TARGET:
		{
			Activity newActivity;

			if ( (m_hTargetEnt->pev->origin - pev->origin).Length() < 1 )
			{
				TaskComplete();
			}
			else
			{
				if ( pTask->iTask == TASK_WALK_TO_TARGET )
					newActivity = ACT_WALK;
				else
					newActivity = ACT_RUN;

				if ( m_hTargetEnt == NULL || !MoveToTarget( newActivity, 2 ) )
				{
					TaskFail();
					ALERT( at_aiconsole, "%s Failed to reach target!!!\n", STRING(pev->classname) );
					RouteClear();
				}
			}
			TaskComplete();
			break;
		}
/*
	case TASK_RUN_PATH:
		{
			if ( FindLateralCover( pevCover->origin, pevCover->view_ofs ) )
			{
					// try lateral first
					m_flMoveWaitFinished = gpGlobals->time + pTask->flData;
					TaskComplete();

				Vector2D	vec2DirToPoint; 
				Vector2D	vec2RightSide;

				// to start strafing, we have to first figure out if the target is on the left side or right side
				UTIL_MakeVectors ( pev->angles );

				vec2DirToPoint = ( m_Route[ 0 ].vecLocation - pev->origin ).Make2D().Normalize();
				vec2RightSide = gpGlobals->v_right.Make2D().Normalize();

				if ( DotProduct ( vec2DirToPoint, vec2RightSide ) > 0 )
				{
					// strafe right
					m_movementActivity = ACT_STRAFE_RIGHT;//ACT_WALK
				}
				else
				{
					// strafe left
					m_movementActivity = ACT_STRAFE_LEFT;//ACT_WALK
				}
			}
			else
			{
			m_movementActivity = ACT_RUN;
			TaskComplete();
			}

			break;
		}
	break;*/

	default:
		if (IsTalking() && m_hTalkTarget != NULL)
		{
			IdleHeadTurn( m_hTalkTarget->pev->origin );
		}
		else
		{
			SetBoneController( 0, 0 );
		}
		CSquadMonster::StartTask( pTask );	
		break;
	}
}


//=========================================================
// RunTask
//=========================================================

void CBaseHuman :: RunTask( Task_t *pTask )
{
	Vector	vecGunPos;
	Vector	vecGunAngles;

	switch ( pTask->iTask )
	{
/*	case TASK_DIE:
	{

	//	ALERT(at_console, ">CUSTOM TASK_DIE!\n");

		//SYS: do not wait 'til the monster has finished his animation, this will cause phantom collisions with dying monsters
		if (!BBoxFlat())
		{
			// a bit of a hack. If a corpses' bbox is positioned such that being left solid so that it can be attacked will
			// block the player on a slope or stairs, the corpse is made nonsolid. 
			//					pev->solid = SOLID_NOT;
			UTIL_SetSize(pev, Vector(-4, -4, 0), Vector(4, 4, 1));
		}
		else // !!!HACKHACK - put monster in a thin, wide bounding box until we fix the solid type/bounding volume problem
			UTIL_SetSize(pev, Vector(pev->mins.x, pev->mins.y, pev->mins.z), Vector(pev->maxs.x, pev->maxs.y, pev->mins.z + 1));

		if (m_fSequenceFinished && pev->frame >= 255)
		{
			pev->deadflag = DEAD_DEAD;

			SetThink(&CBaseHuman::MonsterThink);

			StopAnimation();

			if (ShouldFadeOnDeath())
			{
				// this monster was created by a monstermaker... fade the corpse out.
				SUB_StartFadeOut();
			}
			else
			{
				// body is gonna be around for a while, so have it stink for a bit.
				CSoundEnt::InsertSound(bits_SOUND_CARCASS, pev->origin, 384, 30);
			}
		}
		break;
		
	}*/
/*	case TASK_REMOVE_ITEM:
		if ( m_fSequenceFinished )
		{
			TaskComplete();
		}
		break;
*/
	case TASK_HUMAN_FACE_TOSS_DIR:
		{
			// project a point along the toss vector and turn to face that point.
			MakeIdealYaw( pev->origin + m_vecTossVelocity * 64 );
			ChangeYaw( pev->yaw_speed );

			if ( FacingIdeal() )
			{
				TaskComplete();
			}
		}
		break;

	case TASK_HUMAN_CROUCH:
		if ( m_fSequenceFinished )
		{
		//	pev->view_ofs = Vector( 0, 0, 36 );
			m_fCrouching = TRUE;
			UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_DUCK);

			TaskComplete();
		}
		break;

	case TASK_HUMAN_UNCROUCH:
		if ( m_fSequenceFinished )
		{
		//	pev->view_ofs = Vector( 0, 0, 62 );
			m_fCrouching = FALSE;
			UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);
			TaskComplete();
		}
		break;

	case TASK_HUMAN_SURRENDER:
		if(pev->spawnflags & 4096)
			return;
		
		if ( m_fSequenceFinished )
		{
		//	pev->solid		= SOLID_NOT;
			m_bSurrender	= TRUE;
		
			// !!!temporarily only considering other monsters and clients, don't see prisoners
			pev->spawnflags |= SF_MONSTER_PRISONER;

			if ( m_hEnemy != NULL )
			{
				m_MonsterState	= MONSTERSTATE_IDLE;
				m_hEnemy = NULL;//clear enemy
			}

			CBaseEntity *pEntity = NULL;

			Vector VecSrc = pev->origin;
								
			while ((pEntity = UTIL_FindEntityInSphere( pEntity, VecSrc, 512 )) != NULL)//512
			if (FClassnameIs(pEntity->pev, "monster_agency_member"))
				EMIT_SOUND_DYN( ENT(0), CHAN_STATIC, "npc_voices/friend/radio/SuspectCooperate.wav", 1, ATTN_NONE, 0, PITCH_NORM );

			CBaseEntity *pPlayer = UTIL_PlayerByIndex( 1 );
			CBasePlayer *pPl = (CBasePlayer*)pPlayer;

			FireTargets(STRING(pev->message), this, this, USE_TOGGLE, 0);

			pev->weapons = HUMAN_WEAPON_NONE;

			//remove this monster
			//copied and pasted from killed
			if ( InSquad() )
			MySquadLeader()->SquadRemove( this );
/*
			Remember( bits_MEMORY_KILLED );
			m_IdealMonsterState = MONSTERSTATE_DEAD;*/
			CBaseEntity *pOwner = CBaseEntity::Instance(pev->owner);
			if ( pOwner )
			{
				pOwner->DeathNotice( pev );
			}
		
		//	UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_DUCK);

			//check if monster is blocking a door or something. If so, remove it
			TraceResult tr;

			UTIL_MakeVectors( pev->angles );
			Vector vecStart = pev->origin;
			vecStart.z += pev->size.z;
			Vector vecEnd;
			bool bFailed = false;

			//check if there is room left (so that the player can go trough)
			vecEnd = vecStart + (gpGlobals->v_right * (pev->size.x * -1) * 2);

			UTIL_TraceHull( vecStart, vecEnd, dont_ignore_monsters, head_hull, ENT(pev), &tr );
			
			if ( tr.pHit ) //shit! we have found something, can't go left >>
			{
				ShowLine( vecStart, vecEnd, 1 );
				bFailed = true;
				break;
			}
			else//player can go trough
			{		
				//check if there is room right (so that the player can go trough)
				vecEnd = vecStart + (gpGlobals->v_right * pev->size.x *2);

				UTIL_TraceHull( vecStart, vecEnd, dont_ignore_monsters, head_hull, ENT(pev), &tr );
			
				if ( tr.pHit ) //shit! we have found something, can't go right <<
				{
					ShowLine( vecStart, vecEnd, 1 );
					bFailed = true;
					break;
				}
			}

			if( bFailed == true )
				SUB_StartFadeOut();

			TaskComplete();
		}
		break;

	case TASK_HUMAN_SIDESTEP_RIGHT:
		if ( m_fSequenceFinished )
		{
			m_bSideStepRightDone	= TRUE;

			CineCleanup();

			TaskComplete();
		}
		break;
	
	case TASK_HUMAN_SIDESTEP_LEFT:
		if ( m_fSequenceFinished )
		{
			m_bSideStepLeftDone	= TRUE;

			CineCleanup();

			TaskComplete();
		}
		break;

	case TASK_HUMAN_ONFIRE:
		if ( m_fSequenceFinished )
		{
			bOnFire	= TRUE;

			if ( m_hEnemy != NULL )
			{
				m_MonsterState	= MONSTERSTATE_IDLE;
				m_hEnemy = NULL;//clear enemy
			}
						
			pev->weapons = HUMAN_WEAPON_NONE;

			//remove this monster
			//copied and pasted from killed
			if ( InSquad() )
			MySquadLeader()->SquadRemove( this );

			TaskComplete();
		}
		break;

	case TASK_HUMAN_WAKEUP:
		if ( m_fSequenceFinished )
		{
			// reset position
			Vector new_origin, new_angle;
			GetBonePosition( 0, new_origin, new_angle );

			// Figure out how far they have moved
			// We can't really solve this problem because we can't query the movement of the origin relative
			// to the sequence.  We can get the root bone's position as we do here, but there are
			// cases where the root bone is in a different relative position to the entity's origin
			// before/after the sequence plays.  So we are stuck doing this:

			// !!!HACKHACK: Float the origin up and drop to floor because some sequences have
			// irregular motion that can't be properly accounted for.

			// UNDONE: THIS SHOULD ONLY HAPPEN IF WE ACTUALLY PLAYED THE SEQUENCE.
			Vector oldOrigin = pev->origin;

			// UNDONE: ugly hack.  Don't move monster if they don't "seem" to move
			// this really needs to be done with the AX,AY,etc. flags, but that aren't consistantly
			// being set, so animations that really do move won't be caught.
			if ((oldOrigin - new_origin).Length2D() < 8.0)
				new_origin = oldOrigin;

			pev->origin.x = new_origin.x;
			pev->origin.y = new_origin.y;
			pev->origin.z += 1;

			pev->flags |= FL_ONGROUND;
			int drop = DROP_TO_FLOOR( ENT(pev) );
			
			// Origin in solid?  Set to org at the end of the sequence
			if ( drop < 0 )
				pev->origin = oldOrigin;
			else if ( drop == 0 ) // Hanging in air?
			{
				pev->origin.z = new_origin.z;
				pev->flags &= ~FL_ONGROUND;
			}
			// else entity hit floor, leave there

			// pEntity->pev->origin.z = new_origin.z + 5.0; // damn, got to fix this

			//UTIL_SetOrigin( this, pev->origin );
			UTIL_SetOrigin( pev, pev->origin );//SP FIX
			pev->effects |= EF_NOINTERP;


			m_flFieldOfView		= 90;
			m_flDistLook		= 2000;

			m_MonsterState		= MONSTERSTATE_COMBAT;

			pev->solid			= SOLID_SLIDEBOX;
			pev->movetype		= MOVETYPE_STEP;
			pev->view_ofs		= Vector ( 0, 0, 68 );// position of the eyes relative to monster's origin.
			pev->yaw_speed      = 5;

//			m_bWakeUp	= TRUE;

			TaskComplete();
		}
		break;

	case TASK_HUMAN_DROP_WEAPON:
		if ( m_fSequenceFinished )
		{
			m_bDropped = TRUE;
			TaskComplete();
		}
		break;

	case TASK_HUMAN_SOUND_ATTACK:
		if ( m_fSequenceFinished )
		{
			SquadIssueCommand( SQUADCMD_ATTACK ); 
			TaskComplete();
		}
		break;

	case TASK_HUMAN_SOUND_FOUND_ENEMY:
		if ( m_fSequenceFinished )
		{
			SquadIssueCommand( SQUADCMD_FOUND_ENEMY ); 
			TaskComplete();
		}
		break;

	case TASK_HUMAN_SOUND_SURPRESS:
		if ( m_fSequenceFinished )
		{
			SquadIssueCommand( SQUADCMD_SURPRESSING_FIRE ); 
			TaskComplete();
		}
		break;

	case TASK_HUMAN_SOUND_SEARCH_AND_DESTROY:
		if ( m_fSequenceFinished )
		{
			SquadIssueCommand( SQUADCMD_SEARCH_AND_DESTROY ); 
			TaskComplete();
		}
		break;

	case TASK_HUMAN_SOUND_RETREAT:
		if ( m_fSequenceFinished )
		{
			SquadIssueCommand( SQUADCMD_RETREAT ); 
			TaskComplete();
		}
		break;

	case TASK_HUMAN_SOUND_COME_TO_ME:
		if ( m_fSequenceFinished )
		{
			SquadIssueCommand( SQUADCMD_COME_HERE ); 
			TaskComplete();
		}
		break;

	case TASK_HUMAN_SOUND_CHECK_IN:
		if ( gpGlobals->time >= m_flWaitFinished )
		{
			SquadIssueCommand( SQUADCMD_CHECK_IN ); 
			TaskComplete();
		}
		break;

	case TASK_HUMAN_SOUND_CLEAR:
		if ( gpGlobals->time >= m_flWaitFinished )
		{
			if ( FOkToShout() )
			{
				PlayLabelledSentence( "CLEAR" );
			}
			TaskComplete();
		}
		break;

	case TASK_HUMAN_EYECONTACT:
		if (!IsMoving() && IsTalking() && m_hTalkTarget != NULL)
		{
			IdleHeadTurn( m_hTalkTarget->pev->origin );
		}
		else
		{
			TaskComplete();
		}
		break;

	case TASK_HUMAN_EXPLOSION_FLY:
		{
			if ( FBitSet( pev->flags, FL_ONGROUND ) || pev->velocity == g_vecZero ) 
			{
				m_IdealActivity = ACT_EXPLOSION_LAND;
				SetActivity( m_IdealActivity );
				SetTouch( NULL );
				TaskComplete();
			}
		}
		break;

	case TASK_HUMAN_EXPLOSION_LAND:
		{
			if ( m_fSequenceFinished && pev->frame >= 255 )
			{
				pev->deadflag = DEAD_DEAD;
				
				SetThink ( NULL );
				StopAnimation();

				if ( !BBoxFlat() )
				{
					// a bit of a hack. If a corpses' bbox is positioned such that being left solid so that it can be attacked will
					// block the player on a slope or stairs, the corpse is made nonsolid. 
//					pev->solid = SOLID_NOT;
					UTIL_SetSize ( pev, Vector ( -4, -4, 0 ), Vector ( 4, 4, 1 ) );
				}
				else // !!!HACKHACK - put monster in a thin, wide bounding box until we fix the solid type/bounding volume problem
					UTIL_SetSize ( pev, Vector ( pev->mins.x, pev->mins.y, pev->mins.z ), Vector ( pev->maxs.x, pev->maxs.y, pev->mins.z + 1 ) );

				if ( ShouldFadeOnDeath() )
				{
					// this monster was created by a monstermaker... fade the corpse out.
					SUB_StartFadeOut();
				}
				else
				{
					// body is gonna be around for a while, so have it stink for a bit.
					CSoundEnt::InsertSound ( bits_SOUND_CARCASS, Classify(), pev->origin, 384, 30 );
				//	CSoundEnt::InsertSound ( bits_SOUND_CARCASS, pev->origin, 384, 30 );
				}
			}
		}
		break;

	case TASK_HUMAN_RETREAT_FACE:
		{
			if ( m_hEnemy != NULL && HasConditions( bits_COND_CAN_RANGE_ATTACK1 ) )
			{
				MakeIdealYaw( m_hEnemy->pev->origin );
				ChangeYaw( pev->yaw_speed );

				if ( FacingIdeal() )
				{
					TaskComplete();
				}
			}
			else
			{
				ChangeYaw( pev->yaw_speed );
	
				if ( FacingIdeal() )
				{
					TaskComplete();
				}
			}
		}
		break;

	case TASK_HUMAN_WAIT_GOAL_VISIBLE:
		{
			if ( FVisible( m_vecMoveGoal ) ) 
			{
				TaskComplete();
			}
			else if ( ( m_vecMoveGoal - pev->origin).Length() < 256 )
			{
				TaskFail();
			}
		}
		break;

	case TASK_MOVE_TO_TARGET_RANGE:
		{
			float distance;

			if ( m_hTargetEnt == NULL || !m_hTargetEnt->IsAlive() || m_hTargetEnt->pev->deadflag == DEAD_DYING )
			{
				TaskFail();
			}
			else
			{
				distance = ( m_vecMoveGoal - pev->origin ).Length2D();
				// Re-evaluate when you think your finished, or the target has moved too far
				if ( (distance < pTask->flData) || (m_vecMoveGoal - m_hTargetEnt->pev->origin).Length() > pTask->flData * 0.5 )
				{
					m_vecMoveGoal = m_hTargetEnt->pev->origin;
					distance = ( m_vecMoveGoal - pev->origin ).Length2D();
					FRefreshRoute();
				}

				// Set the appropriate activity based on an overlapping range
				// overlap the range to prevent oscillation
				if ( distance < pTask->flData )
				{
					TaskComplete();
					RouteClear();		// Stop moving
				}
				else 
				{
					// If a long way away or in a combat situation, run, otherwise walk

					CBaseMonster * pTargetMonster = m_hTargetEnt->MyMonsterPointer();

	//				if (  distance >= 270 ||
					if (  distance >= 120 ||
						  m_MonsterState == MONSTERSTATE_COMBAT || 
						  m_MonsterState == MONSTERSTATE_ALERT || 
							( pTargetMonster != NULL && 
								( pTargetMonster->m_MonsterState == MONSTERSTATE_COMBAT || 
								  pTargetMonster->m_MonsterState == MONSTERSTATE_ALERT	
								) 
							)
						)
					{
						//					FRefreshRoute();//sys test

						m_movementActivity = ACT_RUN;
					//	pev->framerate = 2;
					}
//					else if ( distance < 190 )
					else if ( distance < 100 )
					{
						m_movementActivity = ACT_WALK;
					}
				}
			}
			break;
		}
	default:
		CSquadMonster::RunTask( pTask );
		break;
	}
}

 

//=========================================================
// PrescheduleThink - this function runs after conditions
// are collected and before scheduling code is run.
//=========================================================
	
float fLaserPos = 0.0;

void CBaseHuman :: PrescheduleThink ( void )
{
 	//pev->gaitsequence = LookupSequence("Walk_Hurt");

	//EDIT: MONSTER SHOULD TAKE COVER AFTER THROWING UP A FB, OTHERWISE LAST LOCATION MUST BE OCCLUDED
	if ( b_FlashedMonster )
	{
	/*	m_iFrustration += 80;

		if ( SafeToChangeSchedule() )
		{
			ChangeSchedule( GetScheduleOfType( SCHED_COWER ) );
		
			PlaySentence( "T_COUGH", 4, VOL_NORM, ATTN_NORM, 100 );
		}

		b_FlashedMonster = FALSE;*/
	}

	edict_t *pClient = g_engfuncs.pfnPEntityOfEntIndex( 1 );
	CBaseEntity *pPlayer = CBaseEntity::Instance( FIND_ENTITY_BY_CLASSNAME( NULL, "player" ) );

	if( _STRAFE_CODE_TEST )
	{
				//SysOp:
				//SHOOT AND RUN CODE
				//SHOOT AND RUN CODE
				//SHOOT AND RUN CODE
				//
				// this code must work when the monsters run at some position
				// the monster's torso will face their enemy and their legs will run at his position
				// so that, you can see 'strafe' movements

				// now let's check if the torso it's seeing enemy, if so, make true b_FacingEnemyForShoot
				// if so, now we can send animations for run and shoot. The original animation will make
				// the monsters run and shoot but it doesn't look good if the monster are facing a wall and shoots
				// enemy.
				// Now we can handle this, so lets make some code :)

			//	bool	bFacingSomething = false;


				UTIL_MakeAimVectors( pev->angles );
				Vector posGun, angGun, m_posTarget;
			/*
				if (m_hEnemy != NULL)//enemy present
				{
					if (FVisible( m_hEnemy ))
					m_posTarget = m_hEnemy->Center( );	
				}
				else//no enemy
				{
					if(IsFollowingPlayer())//im followin' client
					m_posTarget = pPlayer->Center( );
					else if(IsTalking())//not following, so i'm doing nothing, let's check if I'm speaking
					{
						// if there is a friend nearby to speak to, play sentence, set friend's response time, return
						CBaseEntity *pEntity = FindNearestFriend( !HasConditions(bits_COND_PROVOKED) );	// include players unless I have been provoked

						CBaseHuman * pFriend = NULL;

						if (pEntity) pFriend = pEntity->MyHumanPointer();
						if ( pFriend != NULL && FInViewCone(pFriend) && FVisible(pFriend) )
						{
							m_hTalkTarget = pFriend;

							if ( pFriend->pev->deadflag == DEAD_NO )
							m_posTarget = pFriend->Center( );
						}
					}
					else//nothing to see
					m_posTarget = Vector (0,0,0);
				}
			*/
					
				m_posTarget = pPlayer->Center( );

				// look at my target
				float yaw = VecToYaw(m_posTarget - pev->origin) - pev->angles.y;

				//math for rotational stuff

				if (yaw > 180) 
					yaw -= 360;
				if (yaw < -180) 
					yaw += 360;

					

			//	pev->framerate = 0.3;

				//handle proper anims
				if ( m_flGroundSpeed != 0 )
				{
					switch ( pev->weapons )
					{
						//T/AG weapons
						case	HUMAN_WEAPON_MP5:
						case	HUMAN_WEAPON_ASSAULT:
						case	HUMAN_WEAPON_M249:
						case	HUMAN_WEAPON_UZI:
			/*
							if( yaw >= 60 )
							pev->sequence = LookupSequence(	"run_rifle_R" );
						 if( yaw <= -60 )
							pev->sequence = LookupSequence(	"run_rifle_L" );
			*/
					/*		else
							{
			pev->sequence = LookupSequence(	"run_rifle_N" );			
							}*/

						break;

						case	HUMAN_WEAPON_SHOTGUN:
						case	HUMAN_WEAPON_LAW:
						case	HUMAN_WEAPON_AWP:
						case	HUMAN_WEAPON_USAS:
			//do nothing!
						break;

						case	HUMAN_WEAPON_PISTOL:
						case	HUMAN_WEAPON_DESERT:
			/*
						if( yaw >= 90 )		
							pev->sequence = LookupSequence(	"run_pistol_R" );				
						if( yaw <= -90 )
							pev->sequence = LookupSequence(	"run_pistol_L" );		*/	
						/*	else
							{
							pev->sequence = LookupSequence(	"run_pistol_N" );				

							}*/

						break;
					}
						ALERT( at_console, "yaw %f, pev->angles.y %f\n", yaw, pev->angles.y);

	}


	// turn towards vector
	
	SetBoneController( 2, yaw );

	if( yaw >= 90 )
	SetBoneController( 2,  0);
	
	if( yaw <= -90 )
	SetBoneController( 2,  0);


	if (yaw >= 100)
		yaw = 100;
	
	if( yaw <= -100 )
		yaw = -100;
/*
	//if i am alive
	if ( (m_hEnemy != NULL) && (FVisible( m_hEnemy )) )
//	{

		if (RANDOM_LONG( 0, 99 ) > 70)//score a perfect headshot once in a while
		{
				// m_vecEnemyLKP should be center of enemy body
			Vector vecArmPos, vecArmDir;
			Vector vecDirToEnemy;
			Vector angDir;

			if (HasConditions( bits_COND_SEE_ENEMY))
			{
				vecDirToEnemy = ( ( m_vecEnemyLKP ) - pev->origin );
				angDir = UTIL_VecToAngles( vecDirToEnemy );
				vecDirToEnemy = vecDirToEnemy.Normalize();
			}
			else
			{
				angDir = pev->angles;
				UTIL_MakeAimVectors( angDir );
				vecDirToEnemy = gpGlobals->v_forward;
			}

			pev->effects = EF_MUZZLEFLASH;

			GetAttachment( 1, vecArmPos, vecArmDir );

		//	vecArmPos = vecArmPos + vecDirToEnemy * 0;//32

			MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, vecArmPos );
				WRITE_BYTE( TE_SPRITE );
				WRITE_COORD( vecArmPos.x );	// pos
				WRITE_COORD( vecArmPos.y );	
				WRITE_COORD( vecArmPos.z );	
				WRITE_SHORT( iMuzzleFlash );		// model
				WRITE_BYTE( 1 );				// size * 10
				WRITE_BYTE( 255 );			// brightness
			MESSAGE_END();

			Fire( BULLET_PLAYER_9MM, 1, VECTOR_CONE_1DEGREES, m_iBrassShell, TE_BOUNCE_SHELL, "weapons/1911/1911_fire-1.wav");
		}
//	}

*/
	}


	if ( CVAR_GET_FLOAT("game_mode_zombie" ) != 0 )
	{
		TraceResult	tr;

		CBaseEntity *pZombie = Create( "monster_zombie", pev->origin, pev->angles );
		pZombie->pev->spawnflags |= SF_MONSTER_FADECORPSE;		// if headcrab die, his corpse is fade

		UTIL_TraceHull ( pev->origin, pev->origin + gpGlobals->v_forward * -64, dont_ignore_monsters, head_hull, edict(), &tr );
/*
		if ( tr.flFraction != 1.0 )
		ALERT( at_console, ">NO ROOM TO FALL BACKWARD!\n");
		else
		{
			CBaseEntity *pZombie = Create( "monster_zombie", pev->origin + gpGlobals->v_forward * -64, pev->angles );
			pZombie->pev->spawnflags |= SF_MONSTER_FADECORPSE;		// if headcrab die, his corpse is fade
		}

		UTIL_TraceHull ( pev->origin, pev->origin + gpGlobals->v_forward * 64, dont_ignore_monsters, head_hull, edict(), &tr );

		if ( tr.flFraction != 1.0 )
		ALERT( at_console, ">NO ROOM TO FALL FORWARD!\n");
		else
		{
			CBaseEntity *pZombie = Create( "monster_zombie", pev->origin + gpGlobals->v_forward * 64, pev->angles );
			pZombie->pev->spawnflags |= SF_MONSTER_FADECORPSE;		// if headcrab die, his corpse is fade
		}
*/
		UTIL_Remove( this );
		return;
	}

	if( iSlowmocounter > 0 )
	iSlowmocounter--;

	if( iSlowmocounter == 0 )
	{
		iSlowmocounter = -1;
	}
	

/*
	UTIL_MakeVectors( pev->angles );

	Vector vecSrc = pev->origin + pev->view_ofs;
	Vector vecFOVEnd = vecSrc + (gpGlobals->v_forward * 128) + (gpGlobals->v_right * m_flFieldOfView /2.0 );
//	int iFOVStart = m_flFieldOfView /2;

//	if( iLaserPos >= 0 )
	fLaserPos += 0.5;

	if( fLaserPos == 45.0 )
	fLaserPos = -45.0;


//	Vector vecFOVStart = vecSrc + (gpGlobals->v_forward * 128) + (gpGlobals->v_right * -45 );
	Vector vecFOVStart = vecSrc + (gpGlobals->v_forward * 128) + (gpGlobals->v_right * fLaserPos );

	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_BEAMPOINTS );
		WRITE_COORD( vecSrc.x );
		WRITE_COORD( vecSrc.y );
		WRITE_COORD( vecSrc.z );

		WRITE_COORD( vecFOVStart.x );
		WRITE_COORD( vecFOVStart.y );
		WRITE_COORD( vecFOVStart.z );
		WRITE_SHORT( g_sModelIndexLaser );
		WRITE_BYTE( 0 ); // framerate
		WRITE_BYTE( 0 ); // framerate

		WRITE_BYTE( 1 ); // life
		WRITE_BYTE( 1 );  // width

		WRITE_BYTE( 0 );   // noise
		WRITE_BYTE( 255 );   // r, g, b
		WRITE_BYTE( 255 );   // r, g, b
		WRITE_BYTE( 255 );   // r, g, b
		WRITE_BYTE( 255 );	// brightness
		WRITE_BYTE( 10 );		// speed
	MESSAGE_END();
*/
	/*
	bool isOdd( int integer )
{

  if ( integer % 2== 0 )
     return true;
  else
     return false;
}
*/
/*	if ( pev->deadflag == DEAD_DYING )
	{
		if ( int(pev->frame) % 2== 0 )
		{
			Fire( BULLET_MONSTER_MP5, 1, VECTOR_CONE_20DEGREES, m_iBrassShell, TE_BOUNCE_SHELL, "weapons/mp5/fire-1.wav");
		}
	}
*/
//	if (pev->frags == HUMAN_WEAPON_MP5)
//	{
/*
	Vector	vecGunPos;
	Vector	vecGunAngles;

	GetAttachment( 3, vecGunPos, vecGunAngles );

	CSprite *pSprite = CSprite::SpriteCreate( "sprites/flash_beam.spr", vecGunPos + gpGlobals->v_forward * 0, FALSE );
	pSprite->SetTransparency( kRenderTransAdd, 255, 255, 255, 255, kRenderFxNone );
	pSprite->SetScale( 0.05 );		
//	pSprite->SetAttachment( edict(), 3 );
	UTIL_Remove( pSprite );

	CSprite *pSprite1 = CSprite::SpriteCreate( "sprites/flash_beam.spr", vecGunPos + gpGlobals->v_forward * 50, FALSE );
	pSprite1->SetTransparency( kRenderTransAdd, 255, 255, 255, 255, kRenderFxNone );
	pSprite1->SetScale( 0.1 );	
	UTIL_Remove( pSprite1 );
//	pSprite1->SetAttachment( edict(), 3 );

	CSprite *pSprite2 = CSprite::SpriteCreate( "sprites/flash_beam.spr", vecGunPos + gpGlobals->v_forward * 100, FALSE );
	pSprite2->SetTransparency( kRenderTransAdd, 255, 255, 255, 255, kRenderFxNone );
	pSprite2->SetScale( 0.2 );	
		UTIL_Remove( pSprite2 );

//	pSprite2->SetAttachment( edict(), 3 );

	CSprite *pSprite3 = CSprite::SpriteCreate( "sprites/flash_beam.spr", vecGunPos + gpGlobals->v_forward * 200, FALSE );
	pSprite3->SetTransparency( kRenderTransAdd, 255, 255, 255, 255, kRenderFxNone );
	pSprite3->SetScale( 0.4 );		
//	pSprite3->SetAttachment( edict(), 3 );
		UTIL_Remove( pSprite3 );

	CSprite *pSprite4 = CSprite::SpriteCreate( "sprites/flash_beam.spr", vecGunPos + gpGlobals->v_forward * 400, FALSE );
	pSprite4->SetTransparency( kRenderTransAdd, 255, 255, 255, 255, kRenderFxNone );
	pSprite4->SetScale( 0.8 );	
			UTIL_Remove( pSprite4 );

//	pSprite4->SetAttachment( edict(), 3 );
//	}
*/

	if( iTries == 3 )
	{
		iTries = 0;

		CBaseEntity *pPreviousMarker;

		pPreviousMarker = UTIL_FindEntityByClassname(NULL, "ai_marker");

		if (pPreviousMarker)
		{
			ClientPrint(pev, HUD_PRINTCENTER, "#AI_RemovingPreviousMarker");

			UTIL_Remove( pPreviousMarker );
			pPreviousMarker = NULL;
		}
	}
/*
	Vector vecShootOrigin;

	UTIL_MakeVectors(pev->angles);
	vecShootOrigin = pev->origin + Vector( 0, 0, 55 );
	Vector vecShootDir = ShootAtEnemy( vecShootOrigin );

	Vector angDir = UTIL_VecToAngles( vecShootDir );
	SetBlending( 0, angDir.x );
*/

/*	CBaseEntity *pEntidadMarker = NULL;

	while ((pEntidadMarker = UTIL_FindEntityByClassname(pEntidadMarker, "ai_marker")) != NULL) 
	{		
		float flDist = (pEntidadMarker->Center() - pev->origin).Length();

		if ( flDist <= 16)//already there
		{
			ALERT(at_console, "ai_marker-already there\n");
						
			if (pEntidadMarker)
			{
				ClientPrint(pev, HUD_PRINTCENTER, "#AI_RemovingPreviousMarker");

				UTIL_Remove( pEntidadMarker );
				pEntidadMarker = NULL;
			}
		}
	}
*/
	//report about enemy's type or position at once
	/*
	if ( m_flNextReportTime < gpGlobals->time )
	{	
		if (RANDOM_LONG(0,1))
		ReportEnemyPosition();
		else
		ReportEnemyFrustration();

		m_flNextReportTime = gpGlobals->time + 2;
	}
*/
	Vector posWeapon, angWeapon;
	GetAttachment( 2, posWeapon, angWeapon );

	//Vector vecEyePos = EyePosition();

	//SYS: only if playing a safe animation
//	if ( (pev->sequence == 16) || (pev->sequence == 25) )
	
	if( m_Activity == ACT_IDLE )
	if ( pev->weapons == HUMAN_WEAPON_AWP )
	{
		if ( !pev->deadflag == DEAD_DYING )
		if ( m_hEnemy != NULL ) 
		if (FVisible( m_hEnemy ))
		{	
			extern short g_sModelIndexLaser;

			MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
				WRITE_BYTE( TE_BEAMENTPOINT );
				WRITE_SHORT( m_hEnemy->entindex() );
				WRITE_COORD( posWeapon.x );
				WRITE_COORD( posWeapon.y );
				WRITE_COORD( posWeapon.z );
		/*		WRITE_COORD( vecEyePos.x );
				WRITE_COORD( vecEyePos.y );
				WRITE_COORD( vecEyePos.z );*/
				WRITE_SHORT( g_sModelIndexLaser );
				WRITE_BYTE( 0 ); // frame start
				WRITE_BYTE( 10 ); // framerate
				WRITE_BYTE( 1 ); // life
				WRITE_BYTE( 2 );  // width
				WRITE_BYTE( 0 );   // noise
				WRITE_BYTE( 255 );   // r, g, b
				WRITE_BYTE( 0 );   // r, g, b
				WRITE_BYTE( 0);   // r, g, b
				WRITE_BYTE( 100 );	// brightness
				WRITE_BYTE( 0 );		// speed
			MESSAGE_END();
		}
	}

	pev->framerate = 1.00;

	if( !m_fCrouching && m_Activity == ACT_RUN )
	pev->framerate = 1.50;
	
	if( m_fCrouching && m_Activity == ACT_RUN )
	pev->framerate = 0.80;
	
	if( m_Activity == ACT_RANGE_ATTACK1 )
	pev->framerate = RANDOM_FLOAT( 0.50, 1.00 );
	
	if( bSlow )
	pev->framerate = 0.2;

	if ( m_iNumGrenades <= 0 )
	m_fHandGrenades = 0;

	bPlayerVisible	= TRUE;

	iDistToShoot	= HUMAN_RIFLE_RANGE;//normal shoot

	if(m_bKamikaze)
	{
		Vector vecHand;
		Vector vecAngle;

		GetAttachment( 2, vecHand, vecAngle );
				
		Vector	vecGunPos;
		Vector	vecGunAngles;

		GetAttachment( 3, vecGunPos, vecGunAngles );

		// switch to body group with no gun.
	//	SetBodygroup( HUMAN_BODYGROUP_WEAPON, GUN_NONE );
		REMOVE_P_MODEL;

	}

	//this moves the torso forward-backward - useful for "flinch" anims
	
	if ( yaw_adj > 0.0 )
	{
		yaw_adj -= 5.0;

		if ( yaw_adj < 0.0 )
		yaw_adj = 0.0;
	}
	else
	{
		yaw_adj += 5.0;

		if ( yaw_adj > 0.0 )
		yaw_adj = 0.0;
	}

	SetBoneController( 3, yaw_adj );

	if ( pev->deadflag == DEAD_DYING || pev->health <= 0 )
	{
	//	SetBoneController(1, 0);
	//	SetBoneController(2, 0);
	//	SetBoneController( 3, 0 );
		InitBoneControllers();
	}


//	pev->iuser1 = MAKE_STRING("models/p_test.mdl");//this could be devil!

	//ALERT(at_console, "pev->weaponmodel = %i\n", pev->weaponmodel);




//	ALERT (at_console, "b_FacingEnemyForShoot = %i\n", b_FacingEnemyForShoot);
	//SHOOT AND RUN CODE
	//SHOOT AND RUN CODE
	//SHOOT AND RUN CODE

//*********** LOD
//*********** LOD
//*********** LOD
//*********** LOD
	
/*	if(!bTerrorist)
	{
		if ( GetBodygroup( 0 ) != 4 )
		SetBodygroup( HUMAN_BODYGROUP_BODY, 4 );//900 polys

		CBaseEntity *pEntidadLODcheck1 = NULL;

		while ((pEntidadLODcheck1 = UTIL_FindEntityByClassname(pEntidadLODcheck1, "player")) != NULL) 
		{	
			float flDist = (pEntidadLODcheck1->Center() - pev->origin).Length();

			if ( CVAR_GET_FLOAT( "cl_modeldetail" ) >= 3 )
			{
			}
			else if ( CVAR_GET_FLOAT( "cl_modeldetail" ) == 2 )
			{
				flDist += 256;
			}
			else if ( CVAR_GET_FLOAT( "cl_modeldetail" ) == 1 )
			{
				flDist += 512;
			}
			else
				flDist += 1024;

			if ( flDist <= 1024)
				SetBodygroup( HUMAN_BODYGROUP_BODY, 3 );//1100 polys
				
			if ( flDist <= 512)
				SetBodygroup( HUMAN_BODYGROUP_BODY, 2 );//1500 polys
				
			if ( flDist <= 256)
				SetBodygroup( HUMAN_BODYGROUP_BODY, 1 );//2400 polys

			if ( flDist <= 128)
				SetBodygroup( HUMAN_BODYGROUP_BODY, 0 );//3600 polys

			if ( IsAlive() && pev->deadflag != DEAD_DYING )
			{
					SET_P_MODEL;
			}

		}
	}*/
//*********** LOD
//*********** LOD
//*********** LOD
//*********** LOD

	// If it's been a long time since my last squad command was given, forget about it
	if ( m_flLastSquadCmdTime && m_flLastSquadCmdTime < gpGlobals->time - SQUAD_COMMAND_MEMORY_TIME )
	{
		m_nLastSquadCommand = SQUADCMD_NONE;
		m_flLastSquadCmdTime = 0;
	}
	
	if ( InSquad() && m_hEnemy != NULL )
	{
		if ( HasConditions ( bits_COND_SEE_ENEMY ) )
		{
			// update the squad's last enemy sighting time.
			MySquadLeader()->m_flLastEnemySightTime = gpGlobals->time;
		}
		else
		{
			if ( gpGlobals->time - MySquadLeader()->m_flLastEnemySightTime > 5 )
			{
				// been a while since we've seen the enemy
				MySquadLeader()->m_fEnemyEluded = TRUE;
			}
		}
	}

	CSquadMonster::PrescheduleThink();
}


//=========================================================
// GetSchedule - Decides which type of schedule best suits
// the monster's current squad command and conditions. Then calls
// monster's member function to get a pointer to a schedule
// of the proper type.
//=========================================================

Schedule_t *CBaseHuman :: GetScheduleFromSquadCommand ( void )
{
	if ( !IsAlive() || pev->deadflag == DEAD_DYING ) return NULL;

	switch ( m_nLastSquadCommand )
	{
	case SQUADCMD_RETREAT:
		if ( m_hEnemy == NULL || HasConditions( bits_COND_ENEMY_OCCLUDED ) )
		{
			// I can't see anything bad so the retreat was successful.  Yay!
			m_nLastSquadCommand = SQUADCMD_NONE;
			
			if (!IsFollowingPlayer() && InSquad() && IsLeader())
			{
				// Now tell everyone to regroup for another attack
				return GetScheduleOfType( SCHED_HUMAN_SIGNAL_COME_TO_ME );
			}
		}
		else if ( !HasMemory( bits_MEMORY_HUMAN_NO_COVER ) )
		{
			return GetScheduleOfType( SCHED_HUMAN_RETREAT );
		}
		break;

	case SQUADCMD_COME_HERE:
		if ( IsFollowingPlayer() && (m_hTargetEnt->pev->origin - pev->origin).Length() > 64 ) 
		{
			return GetScheduleOfType( SCHED_HUMAN_FOLLOW_CLOSE );
		}
		else if	( IsFollowingHuman() && (m_hTargetEnt->pev->origin - pev->origin).Length() > 256  )
		{
			return GetScheduleOfType( SCHED_HUMAN_FOLLOW );
		}
		else
		{
			// I am now close to my target (or have stopped following)
			m_nLastSquadCommand = SQUADCMD_NONE;
			if ( IsFollowingHuman() ) StopFollowing( FALSE );
		}
		break;

	case SQUADCMD_DEFENSE:
		if ( !SquadGetWounded() )
		{
			// There is no wounded, so defense was successful.  Yay!  Or he could have died, of course.
			m_nLastSquadCommand = SQUADCMD_NONE;
			if ( IsFollowingHuman() ) StopFollowing( FALSE );
		}
		else
		{
			return GetScheduleOfType( SCHED_TARGET_CHASE );
		}
		break;

	case SQUADCMD_ATTACK:
		if ( ( m_hEnemy != NULL && m_hEnemy->IsAlive() ) || SquadGetCommanderEnemy() )	// Try and get new enemy
		{
			if ( HasConditions( bits_COND_CAN_MELEE_ATTACK1 ) )
			{
				return GetScheduleOfType( SCHED_MELEE_ATTACK1 );
			}
			else if ( HasConditions( bits_COND_CAN_RANGE_ATTACK1 ) && OccupySlot( bits_SLOTS_HUMAN_ENGAGE ) )
			{
				return GetScheduleOfType( SCHED_RANGE_ATTACK1 );
			}
			else if ( HasConditions( bits_COND_CAN_RANGE_ATTACK2 ) && OccupySlot( bits_SLOTS_HUMAN_GRENADE ) )
			{
				return GetScheduleOfType( SCHED_RANGE_ATTACK2 );
			}
			else
			{
				return GetScheduleOfType( SCHED_HUMAN_ESTABLISH_LINE_OF_FIRE );
			}
		}

		// Squad Commander has no enemy, so attack was successful.  Yay!
		m_nLastSquadCommand = SQUADCMD_NONE;
		break;

	case SQUADCMD_SEARCH_AND_DESTROY:
		if ( m_hEnemy != NULL && m_hEnemy->IsAlive() )	// If I have an enemy then go after him
		{
			return GetScheduleOfType( SCHED_HUMAN_ESTABLISH_LINE_OF_FIRE );
		}
		else if ( HasConditions(bits_COND_HEAR_SOUND) )
		{
			return GetScheduleOfType( SCHED_INVESTIGATE_SOUND );
		}
		else
		{
			return GetScheduleOfType( SCHED_HUMAN_SEARCH_AND_DESTROY );
		}
		break;

	case SQUADCMD_SURPRESSING_FIRE:
		if ( m_vecEnemyLKP != g_vecZero && FVisible( m_vecEnemyLKP ) )
		{
			if ( NoFriendlyFire() && OccupySlot( bits_SLOTS_HUMAN_ENGAGE ) )
			{
				return GetScheduleOfType( SCHED_HUMAN_SURPRESS );
			}
			else if ( HasConditions( bits_COND_CAN_RANGE_ATTACK2 ) && m_fHandGrenades && 
				OccupySlot( bits_SLOTS_HUMAN_GRENADE ) )
			{
				return GetScheduleOfType( SCHED_RANGE_ATTACK2 );
			}
			else
			{
				// If there is someone in the way then give up
				m_nLastSquadCommand = SQUADCMD_NONE;
			}
		}
		else
		{
			return GetScheduleOfType( SCHED_HUMAN_MOVE_TO_ENEMY_LKP );
		}
		break;
	}

	return NULL;
}


//=========================================================
// GetSchedule - Decides which type of schedule best suits
// the monster's current state and conditions. Then calls
// monster's member function to get a pointer to a schedule
// of the proper type.
//=========================================================

Schedule_t *CBaseHuman :: GetSchedule ( void )
{
	if( _STRAFE_CODE_TEST )
	return GetScheduleOfType( SCHED_TAKE_COVER_FROM_ENEMY );

	CSound *pSound = NULL;
	CBaseEntity *pPlayer = UTIL_PlayerByIndex( 1 );

	if (m_bSurrender)
		return  GetScheduleOfType ( SCHED_TARGET_FACE );

	if (bOnFire)
		return  GetScheduleOfType ( SCHED_TARGET_FACE );

/*	if ( !m_bWakeUp )
	{
//		FOkToSpeak() = FALSE;

		pev->movetype		= MOVETYPE_NONE;

		m_flFieldOfView		= 0;
		m_flDistLook		= 0;

		pev->view_ofs = g_vecZero;

		//m_MonsterState = MONSTERSTATE_SCRIPT;

		if ( HasConditions(bits_COND_HEAR_SOUND) )
		{
			pSound = PBestSound();

			if ( pSound )
			return GetScheduleOfType( SCHED_HUMAN_WAKEUP );
		}
	}
*/
	if ( (!bTerrorist) && (m_hEnemy == NULL) )
	{
		CBaseEntity *pEntidadMarker = NULL;

		while ((pEntidadMarker = UTIL_FindEntityByClassname(pEntidadMarker, "ai_marker")) != NULL) 
		{		
			float flDist = (pEntidadMarker->Center() - pev->origin).Length();

			if ( flDist <= 128)//already there
			{
				ALERT(at_console, "ai_marker-already there\n");

				iTries = 0;

				CBaseEntity *pPreviousMarker;

				pPreviousMarker = UTIL_FindEntityByClassname(NULL, "ai_marker");

				if (pPreviousMarker)
				{
					ClientPrint(pev, HUD_PRINTCENTER, "#AI_RemovingPreviousMarker");

					UTIL_Remove( pPreviousMarker );
					pPreviousMarker = NULL;
				}
			}
			else
			{
			//	if( !bSchedSended )
			//	{
				return GetScheduleOfType ( SCHED_GO_TO_AI_MARKER );
			//		bSchedSended = true;
			//	}
			}
		}
	}

	// Flying? If PRONE, barnacle has me. IF not, it's assumed I am rapelling. 
	if ( pev->movetype == MOVETYPE_FLY && m_MonsterState != MONSTERSTATE_PRONE )
	{
		if (pev->flags & FL_ONGROUND)
		{
			// If I have landed on top of someone
			if ( pev->groundentity )	
			{
				CBaseEntity * pEntity = Instance( pev->groundentity );
				
				// If it is an enemy, shoot them
				
				if ( pEntity && IRelationship( pEntity ) > R_NO && pEntity->IsAlive() )	
				{
					CBaseMonster *pMonster = pEntity->MyMonsterPointer();
					if ( pMonster && pMonster->pev->deadflag == DEAD_NO && pMonster->pev->movetype != MOVETYPE_FLY &&
						pMonster->m_MonsterState == MONSTERSTATE_IDLE &&
						pMonster->m_MonsterState == MONSTERSTATE_ALERT &&
						pMonster->m_MonsterState == MONSTERSTATE_COMBAT )
					{
						pMonster->ChangeSchedule( pMonster->GetScheduleOfType( SCHED_TAKE_COVER_FROM_ORIGIN ) );
					}

					m_hEnemy = pEntity;
					return GetScheduleOfType ( SCHED_HUMAN_REPEL_ATTACK );
				}
				
				// If it is a friend, tell them to get out the way
				else if ( pEntity && IRelationship( pEntity ) == R_AL && pEntity->IsAlive() )	
				{
					CBaseHuman * pHuman = pEntity->MyHumanPointer();
					
					if ( pHuman && !pHuman->IsMoving() && pHuman->SafeToChangeSchedule() )
					{
						pHuman->ChangeSchedule( pHuman->GetScheduleOfType( SCHED_TAKE_COVER_FROM_ORIGIN ) );
						return GetScheduleOfType ( SCHED_HUMAN_REPEL );	// Keep repelling a while longer till they are gone
					}
				}
			}

			// just landed
			pev->movetype = MOVETYPE_STEP;

			// If squad leader, tell my squad to go looking for trouble as soon as we've landed
			if ( InSquad() && IsLeader() && m_hEnemy == NULL )
			{
				return GetScheduleOfType ( SCHED_HUMAN_REPEL_LAND_SEARCH_AND_DESTROY );
			}
			else
			{
				return GetScheduleOfType ( SCHED_HUMAN_REPEL_LAND );
			}
		}
		else
		{
			// repel down a rope, 
			if ( m_MonsterState == MONSTERSTATE_COMBAT && !HasConditions( bits_COND_NO_AMMO_LOADED ) &&
				HasConditions( bits_COND_CAN_RANGE_ATTACK1 ) ) 
			{
				return GetScheduleOfType ( SCHED_HUMAN_REPEL_ATTACK );
			}
			else
			{
				return GetScheduleOfType ( SCHED_HUMAN_REPEL );
			}
		}
	}


	// Humans place HIGH priority on running away from danger sounds.

	if ( HasConditions(bits_COND_HEAR_SOUND) && m_MonsterState != MONSTERSTATE_SCRIPT )
	{
		pSound = PBestSound();

		if ( pSound )
		{
			if (pSound->m_iType & bits_SOUND_DANGER)
			{
				return GetScheduleOfType( SCHED_TAKE_COVER_FROM_BEST_SOUND );
			}
		}
	}

	switch( m_MonsterState )
	{
	case MONSTERSTATE_COMBAT:
		{
			// Squad Commands that are urgent enough to respond to even in the thick of combat
			if ( m_nLastSquadCommand >= SQUADCMD_RETREAT ) 
			{
				Schedule_t * pSchedule = GetScheduleFromSquadCommand();
				if ( pSchedule != NULL ) return pSchedule;
			}

			// dead enemy
			if ( HasConditions( bits_COND_ENEMY_DEAD ) )
			{
				m_bitsSaid = 0;	// Clear conversation bits

				// call base class, all code to handle dead enemies is centralized there.
				return CSquadMonster :: GetSchedule();
			}

		/*	CBaseEntity *pEntidadLODcheck1 = NULL;

			while ((pEntidadLODcheck1 = UTIL_FindEntityByClassname(pEntidadLODcheck1, "player")) != NULL) 
			{	
				float flDist = (pEntidadLODcheck1->Center() - pev->origin).Length();

			//	if ( flDist <= 256)
				{
			//		if( LookupSequence(	"sidestep_right" ) != ACTIVITY_NOT_AVAILABLE )//check if we have these anims first
					if ( bTerrorist && ( bPlayerIsPoitingMe || (RANDOM_LONG(0,99) < 50) ) )
					{
						if (RANDOM_LONG(0,9) == 0)//90% of fail
						m_bUseSideJump = TRUE;
						else
						m_bUseSideJump = FALSE;

						float fHullMultiplier;
							
						if ( m_bUseSideJump )
						fHullMultiplier = 4.0;
						else
						fHullMultiplier = 2.0;

				//		bPlayerIsPoitingMe = FALSE;//I already jumped

						TraceResult tr;

						UTIL_MakeVectors( pev->angles );
						Vector vecStart = pev->origin;
						vecStart.z += pev->size.z;
						Vector vecEnd;
						Vector vecDown;
						Vector vecDownStart;
						Vector vecDownEnd;

						switch ( RANDOM_LONG(0,1) )
						{
							case 0:		
								
								vecEnd = vecStart + (gpGlobals->v_right * (pev->size.x * -1) * fHullMultiplier );

								UTIL_TraceHull( vecStart, vecEnd, dont_ignore_monsters, head_hull, ENT(pev), &tr );
								
								if ( tr.pHit ) //shit! we have found something, can't go left >>
								{
									ShowLine( vecStart, vecEnd, 1 );
									break;
								}
								else
								{		
									ShowLine( vecStart, vecEnd, 2 );

									vecDown = vecEnd + ( gpGlobals->v_up * (pev->size.z * -1)) ;

									TraceResult tr;
									UTIL_TraceLine( vecEnd, vecDown, ignore_monsters, edict(), &tr);

									if (tr.flFraction != 1.0)
									{
										ShowLine( vecEnd, vecDown, 1 );

										ALERT( at_console, "Traceline failed\n" );
										break;
									}
									else
									{
										ShowLine( vecEnd, vecDown, 2 );

										// lets see if we are dropping at ground
										vecDownStart = vecDown + ( gpGlobals->v_up * 8 );//move up a bit and trace down
										vecDownEnd = vecDown + ( gpGlobals->v_up * -16  );

										TraceResult tr;
										UTIL_TraceLine( vecDownStart, vecDownEnd, ignore_monsters, edict(), &tr);

										if (tr.flFraction != 1.0)//must be fraction to detect ground!!
										{
											ShowLine( vecDownStart, vecDownEnd, 2 );

											return GetScheduleOfType( SCHED_HUMAN_SIDESTEP_RIGHT ); 								
											break;
										}
										else
										{
											ShowLine( vecDownStart, vecDownEnd, 1 );
											break;
										}
									}
								}

							break;

							case 1:	
								
								vecEnd = vecStart + (gpGlobals->v_right * pev->size.x * fHullMultiplier );

								UTIL_TraceHull( vecStart, vecEnd, dont_ignore_monsters, head_hull, ENT(pev), &tr );
							
								if ( tr.pHit ) //shit! we have found something, can't go left >>
								{
									ShowLine( vecStart, vecEnd, 1 );
									break;
								}
								else
								{
									ShowLine( vecStart, vecEnd, 2 );

									vecDown = vecEnd + ( gpGlobals->v_up * (pev->size.z * -1)) ;

									TraceResult tr;
									UTIL_TraceLine( vecEnd, vecDown, ignore_monsters, edict(), &tr);

									if (tr.flFraction != 1.0)
									{
										ShowLine( vecEnd, vecDown, 1 );

										ALERT( at_console, "Traceline failed\n" );
										break;
									}
									else
									{
										ShowLine( vecEnd, vecDown, 2 );

										// lets see if we are dropping at ground
										vecDownStart = vecDown + ( gpGlobals->v_up * 8 );//move up a bit and trace down
										vecDownEnd = vecDown + ( gpGlobals->v_up * -16  );
									
										TraceResult tr;
										UTIL_TraceLine( vecDownStart, vecDownEnd, ignore_monsters, edict(), &tr);

										if (tr.flFraction != 1.0)//must be fraction to detect ground!!
										{
											ShowLine( vecDownStart, vecDownEnd, 2 );

											return GetScheduleOfType( SCHED_HUMAN_SIDESTEP_LEFT ); 											
											break;
										}
										else
										{
											ShowLine( vecDownStart, vecDownEnd, 1 );
											break;
										}
									}
								}
								
							break;
						}
					}
				}
			}*/
	

			if ( (!m_fGunDrawn) && (!m_bSurrender) && (!m_bKamikaze) /*&& (pev->weapons != HUMAN_WEAPON_NONE)*/ )//if weapon hasn't been deployed and we aren't surrendered and not kamikazed
			{	
				//this check is useful, because only want to deploy those pistols
				switch ( pev->weapons )
				{
					case HUMAN_WEAPON_PISTOL:	
					case HUMAN_WEAPON_DESERT:
					case HUMAN_WEAPON_USP:
					case HUMAN_WEAPON_357:

					return GetScheduleOfType( SCHED_HUMAN_DRAW_WEAPON );
					break;
				}
			}
	
			if ( bTerrorist && (!m_bSurrender) && (m_iFrustration >= m_iBreakPoint) && (!m_bKamikaze) && (!m_bDontReact) /*&& (pev->weapons != HUMAN_WEAPON_NONE)*/)
			{
				if ( !FClassnameIs(pev, "monster_terrorist_female" ) )
				{
					//speak something like: 'oh please! don't kill me superman!'
					switch ( RANDOM_LONG(0,1) )
					{
						case 0:	
							EMIT_SOUND( ENT(pev), CHAN_BODY, "npc_voices/spanish/surrender_1_voiceC.wav", 1, ATTN_NORM );	
							break;
						case 1:
							EMIT_SOUND( ENT(pev), CHAN_BODY, "npc_voices/spanish/surrender_2_voiceC.wav", 1, ATTN_NORM );	
							break;
					}
				}

				//moved here, was on TASK_HUMAN_SURRENDER
				//reason: NPC didn't clear monster state, now it does
/*
				if ( m_hEnemy != NULL )
				{
					m_MonsterState	= MONSTERSTATE_IDLE;
					m_hEnemy = NULL;//clear enemy
				}*/
			//	basd = TRUE;

				return GetScheduleOfType( SCHED_HUMAN_SURRENDER );
			}

		//	if( StartBurning )
		//	return GetScheduleOfType( SCHED_HUMAN_FIREUP );

			// no ammo
			if ( HasConditions ( bits_COND_NO_AMMO_LOADED ) )
			{
			//	++m_iFrustration;

				if (pev->frags == HUMAN_WEAPON_LAW)
				{
					if (!m_bDropped)//if isn't dropped... drop it! (once law is dropped this if wont be checked
					return GetScheduleOfType( SCHED_HUMAN_DROP_WEAPON );
				}
				//!!!KELLY - this individual just realized he's out of bullet ammo. 
				// He's going to try to find cover to run to and reload, but rarely, if 
				// none is available, he'll drop and reload in the open here. 

				//NEW: when the monster it's out of ammo it will try to reload, check if
				//we have clips and we doesn't drop anything yet, otherwise drop weapon

				if ( (m_iNumClips <=0) && (!m_bDropped) && m_bCanDropPrimWeapon )//this check is shared by both monsters
				{//if terrorist is not surrender and not kamikaze and random value

					//go to kamizaze or drop weapon if we don't have ammo
					if ( bTerrorist && (!m_bSurrender) && (!m_bKamikaze) && (RANDOM_LONG(0,99) < 50) )
					{		
						m_bKamikaze = TRUE;//go to kamikaze
					//	SetBodygroup( HUMAN_BODYGROUP_WEAPON, GUN_NONE );

						REMOVE_P_MODEL;

						m_flNextGrenadeCheck = + 4;

						//well, the monster reload the weapon and clear the bit COND_NO_AMMO_LOADED, only
						//in HandleAnimFunc. It needs the monster make the reload animation and I don't
						//want it. Just clear the bit COND_NO_AMMO_LOADED and "reload" the weapon here and now.
						//after this, the monster will try to attack, but only in melee mode.

						m_cAmmoLoaded = m_cClipSize;
						ClearConditions(bits_COND_NO_AMMO_LOADED);
											
						//EMIT_SOUND( ENT(pev), CHAN_VOICE, "npc_voices/English/voiceA/yell_macho_2_VoiceA.wav", 1, 1.0 );

						if ( m_fCrouching )//stand
						return GetScheduleOfType( SCHED_HUMAN_UNCROUCH );
					}				
					else//not terrorist, is surrender, is kamikaze or the random fails
					{
						return GetScheduleOfType( SCHED_HUMAN_DROP_WEAPON );
					}
				}
				else//we have clips and we have a weapon
				return GetScheduleOfType ( SCHED_HUMAN_COVER_AND_RELOAD );
			}

			// If the enemy is facing directly at me, duck
		//	if (pPlayer->m_bFacingTerrorist)
/*
			if ( CVAR_GET_FLOAT( "cl_crosshair_ontarget" ) == 1 )
			{
				if ((!m_bSurrender) && (!m_bKamikaze))
				{
					if(m_fCrouching)//if Im already ducked, abort
					{
						if ( OccupySlot ( bits_SLOTS_HUMAN_ENGAGE ) )
						{
							// try to take an available ENGAGE slot
							return GetScheduleOfType( SCHED_RANGE_ATTACK1 );
						}
						else if ( HasConditions ( bits_COND_CAN_RANGE_ATTACK2 ) && OccupySlot( bits_SLOTS_HUMAN_GRENADE ) )
						{
							// throw a grenade if can and no engage slots are available
							return GetScheduleOfType( SCHED_RANGE_ATTACK2 );
						}
						else
						{
							// hide!
							return GetScheduleOfType( SCHED_TAKE_COVER_FROM_ENEMY );
						}		
					}
					else
					{
						return GetScheduleOfType( SCHED_COWER );//SCHED_COWER//SCHED_STANDOFF
					}
				}
			}
*/
			// new enemy
			if ( HasConditions(bits_COND_NEW_ENEMY) )
			{
				CBasePlayer *pPlayer = NULL;
				pPlayer = (CBasePlayer *)UTIL_PlayerByIndex( 1 );
				if ( pPlayer )
					pPlayer->PlayActionMusic();

				if ( !FBitSet( m_afCapability, ( bits_CAP_RANGE_ATTACK1 | bits_CAP_RANGE_ATTACK2 | bits_CAP_MELEE_ATTACK1 ) ) )
				{
					return GetScheduleOfType( SCHED_TAKE_COVER_FROM_ENEMY );
				}
				else if ( InSquad() )
				{
					CSquadMonster *pSquadLeader = MySquadLeader();
					if ( pSquadLeader ) pSquadLeader->m_fEnemyEluded = FALSE;
					
					if (!IsLeader())
					{
						if ( SquadAnyIdle() )	// If anyone in my squad isn't doing anything, inform them
						{
							return GetScheduleOfType ( SCHED_HUMAN_FOUND_ENEMY );
						}
					}
					else if ( HasConditions( bits_COND_CAN_RANGE_ATTACK1 ) )
					{
						if ( SquadIsHealthy() )
						{
							// Attack this target and signal to my squad to do likewise

							if ( m_nLastSquadCommand < SQUADCMD_ATTACK && OccupySlot( bits_SLOTS_HUMAN_ENGAGE ) )
							{
								return GetScheduleOfType( SCHED_HUMAN_SIGNAL_ATTACK );
							}
						}
						else if ( m_nLastSquadCommand < SQUADCMD_RETREAT )
						{
							// Decide to fire a few shots and signal an orderly retreat
							return GetScheduleOfType( SCHED_HUMAN_SIGNAL_RETREAT );
						}
					}
				}

				return GetScheduleOfType ( SCHED_WAKE_ANGRY );
			}

			// damaged
			if ( HasConditions( bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE ) )
			{
				if ( pev->health < pev->max_health / 3/* && m_flLastMedicSearch + MEDIC_SEARCH_TIME < gpGlobals->time*/ )
				{
					// Find a medic, this schedule will also call my squad to defend me (or at least the weakest member)
					return GetScheduleOfType( SCHED_HUMAN_FIND_MEDIC_COMBAT );
				}
				else if ( pev->health < pev->max_health - ( pev->max_health / 3 ) && !HasMemory( bits_MEMORY_HUMAN_NO_COVER ) )
				{
					return GetScheduleOfType( SCHED_TAKE_COVER_FROM_ENEMY );
				}
				else
				{
				//	++m_iFrustration;
					if (!m_bSurrender)
					return GetScheduleOfType( SCHED_SMALL_FLINCH );
				}
			}

			// can kick
			if ( HasConditions ( bits_COND_CAN_MELEE_ATTACK1 ) )
			{
				return GetScheduleOfType ( SCHED_MELEE_ATTACK1 );
			}
			
			// can shoot
			if ( HasConditions ( bits_COND_CAN_RANGE_ATTACK1 ) )
			{
				if ( InSquad() )
				{
					CSquadMonster *pSquadLeader = MySquadLeader();
					if ( pSquadLeader ) pSquadLeader->m_fEnemyEluded = FALSE;

					// if the enemy has eluded the squad and a squad member has just located the enemy
					// and the enemy does not see the squad member, issue a call to the squad to waste a 
					// little time and give the player a chance to turn.
					if ( !HasConditions ( bits_COND_ENEMY_FACING_ME ) && SquadAnyIdle() )
					{
					//	++m_iFrustration;
						return GetScheduleOfType ( SCHED_HUMAN_FOUND_ENEMY );
					}

					if ( IsLeader() )
					{
						if ( SquadIsHealthy() && m_nLastSquadCommand < SQUADCMD_ATTACK )
						{
							// We are fit so pro-actively attack the bastards
							return GetScheduleOfType( SCHED_HUMAN_SIGNAL_ATTACK );
						}
						else if ( m_nLastSquadCommand < SQUADCMD_SURPRESSING_FIRE )
						{
							// Things are not looking so good, we are less confident, so lay down surpressing fire
							return GetScheduleOfType( SCHED_HUMAN_SIGNAL_SURPRESS );
						}
					}
				}

				if ( OccupySlot ( bits_SLOTS_HUMAN_ENGAGE ) )
				{
					// try to take an available ENGAGE slot
					return GetScheduleOfType( SCHED_RANGE_ATTACK1 );
				}
				else if ( HasConditions ( bits_COND_CAN_RANGE_ATTACK2 ) && OccupySlot( bits_SLOTS_HUMAN_GRENADE ) )
				{
					// throw a grenade if can and no engage slots are available
					return GetScheduleOfType( SCHED_RANGE_ATTACK2 );
				}
				else
				{
					// hide!
					return GetScheduleOfType( SCHED_TAKE_COVER_FROM_ENEMY );
				}
			}

			// can't see enemy (even if I turn round)
			if ( HasConditions( bits_COND_ENEMY_OCCLUDED ) )
			{
				// Squad Commands that I don't care about if I'm in the thick of battle but may pay attention to
				// if I can't see my enemy even if he's not dead
				Schedule_t * pSchedule = GetScheduleFromSquadCommand();
				if ( pSchedule != NULL ) return pSchedule;

				if ( HasConditions( bits_COND_CAN_RANGE_ATTACK2 ) && OccupySlot( bits_SLOTS_HUMAN_GRENADE ) )
				{
					return GetScheduleOfType( SCHED_RANGE_ATTACK2 );
				}
				else if ( pev->health < pev->max_health / 3 && m_flLastMedicSearch + MEDIC_SEARCH_TIME < gpGlobals->time )
				{
					// Find a medic
					return GetScheduleOfType( SCHED_HUMAN_FIND_MEDIC_COMBAT );
				}
				else if ( FBitSet( m_afCapability, bits_CAP_RANGE_ATTACK1 ) )
				{
					return GetScheduleOfType( SCHED_HUMAN_ESTABLISH_LINE_OF_FIRE );
				}
				else
				{
					return GetScheduleOfType( SCHED_STANDOFF );
				}
			}

			// If we can see the enemy but can't attack him then we need to establish a line of fire whatever our
			// slot or squad command is
			if ( HasConditions( bits_COND_SEE_ENEMY ) && !HasConditions ( bits_COND_CAN_RANGE_ATTACK1 ) )
			{
				if ( FBitSet( m_afCapability, bits_CAP_RANGE_ATTACK1 ) )
				{
					return GetScheduleOfType( SCHED_HUMAN_ESTABLISH_LINE_OF_FIRE );
				}
				else
				{
					return GetScheduleOfType( SCHED_TAKE_COVER_FROM_ENEMY );
				}
			}
			/*		
			if ( m_hEnemy != NULL )
			{
				if (gpGlobals->time < m_flNextReportEnemyWeaponTime )
				{
					ReportEnemyType();
					m_flNextReportEnemyWeaponTime = gpGlobals->time + 5;
				}
			}*/
		}
		break;

	case MONSTERSTATE_ALERT:
		{
		//ASSASIN CODE
	//		if (m_iPlayerReact == 0)
			{
				if (!m_bSurrender)
				if ( HasConditions ( bits_COND_HEAR_SOUND ))
				{
					CSound *pSound;
					pSound = PBestSound();

					ASSERT( pSound != NULL );
					if ( pSound && (pSound->m_iType & bits_SOUND_DANGER) )
					{
						return GetScheduleOfType( SCHED_TAKE_COVER_FROM_BEST_SOUND );
					}
					if ( pSound && (pSound->m_iType & bits_SOUND_COMBAT) )
					{
						return GetScheduleOfType( SCHED_INVESTIGATE_SOUND );
					}
				}
			}
			
			//ASSASIN CODE
			//this work, but I need to check the weapon and make a decent animation pick up
		/*	switch ( pev->weapons )
			{
				case HUMAN_WEAPON_DESERT:
				case HUMAN_WEAPON_PISTOL:
				{*/
			/*		CBaseEntity *pEntidadItem = NULL;

					while ((pEntidadItem = UTIL_FindEntityByClassname(pEntidadItem, "weapon_9mmAR")) != NULL) 
					{		
						float flDist = (pEntidadItem->Center() - pev->origin).Length();
					
						if ( flDist <= 8)//already there
						{
							ALERT(at_console, "weapon-already there\n");
						}
						else
						return GetScheduleOfType ( SCHED_GO_TO_ITEM );
					}*/
		/*		}
				break;
			}
*/
			//sys - im in a alert state, check where that noise came from
			//EDIT: isn't so good to override another schedule, let's use original AI
			/*
			if (!m_bSurrender)
			{
				if ( m_hEnemy == NULL )
				{
					if ( HasConditions ( bits_COND_HEAR_SOUND ))
					{
						CSound *pSound;
						pSound = PBestSound();

						ASSERT( pSound != NULL );
						if ( pSound && (pSound->m_iType & bits_SOUND_DANGER) )
						{
							return GetScheduleOfType( SCHED_TAKE_COVER_FROM_BEST_SOUND );
						}
						if ( pSound && (pSound->m_iType & bits_SOUND_COMBAT) )
						{
							return GetScheduleOfType( SCHED_INVESTIGATE_SOUND );
						}
					}
				}
			}*/
			


			// Taking damage code is in the BaseMonster Schedule and has high priority
			if ( HasConditions( bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE) )
			{
				return CSquadMonster::GetSchedule();
			}

			// If we have just killed the enemy and haven't got anyone else to shoot
			if ( HasConditions( bits_COND_ENEMY_DEAD ) )
			{
				Forget( bits_MEMORY_HUMAN_NO_COVER ); // If my enemy is killed the cover situation may have changed

				if ( InSquad() && !IsFollowingPlayer() && IsLeader() )
				{
					// If I am the squad leader, now would be the time to issue some kind of squad order
					if ( m_nLastSquadCommand < SQUADCMD_CHECK_IN )	
					{
						// Try and get a new enemy from one of my squad members
						return GetScheduleOfType( SCHED_HUMAN_SIGNAL_CHECK_IN );
					}
					else if ( SquadIsScattered() && m_nLastSquadCommand < SQUADCMD_COME_HERE )
					{
						// Order my squad to report back in and regroup
						return GetScheduleOfType( SCHED_HUMAN_SIGNAL_COME_TO_ME );
					} 
					else if ( m_nLastSquadCommand < SQUADCMD_SEARCH_AND_DESTROY )
					{
						// If we have no enemies and are close together it's time for some action
						return GetScheduleOfType( SCHED_HUMAN_SIGNAL_SEARCH_AND_DESTROY );
					}
				}

				if ( pev->health <= ( 2 * pev->max_health ) / 3 && m_flLastMedicSearch + MEDIC_SEARCH_TIME < gpGlobals->time )
				{
					return GetScheduleOfType( SCHED_HUMAN_FIND_MEDIC );
				}
			
				//--m_iFrustration;

				return GetScheduleOfType ( SCHED_VICTORY_DANCE );
			}
		}

	case MONSTERSTATE_IDLE:
		{
		//ASSASIN CODE
			if (!m_bSurrender)
			if ( HasConditions ( bits_COND_HEAR_SOUND ))
			{
				CSound *pSound;
				pSound = PBestSound();

				ASSERT( pSound != NULL );
				if ( pSound && (pSound->m_iType & bits_SOUND_DANGER) )
				{
					return GetScheduleOfType( SCHED_TAKE_COVER_FROM_BEST_SOUND );
				}
				if ( pSound && (pSound->m_iType & bits_SOUND_COMBAT) )
				{
					return GetScheduleOfType( SCHED_INVESTIGATE_SOUND );
				}
			}
			//ASSASIN CODE
			// no ammo
			if ( HasConditions ( bits_COND_NO_AMMO_LOADED ) )
			{
				return GetScheduleOfType ( SCHED_HUMAN_RELOAD );
			}

			// Squad Commands that only need a response when not fighting
			// ( This applies to all squad commands )

			Schedule_t * pSchedule = GetScheduleFromSquadCommand();
			if ( pSchedule != NULL ) return pSchedule;

			// If have been crouching for long time get up
			if ( m_fCrouching && ( m_flCrouchTime <= gpGlobals->time || pev->waterlevel == 3 ) )
			{
				return GetScheduleOfType( SCHED_HUMAN_UNCROUCH );
			}

			// Follow leader
			if ( IsFollowing() )
			{
				if ( m_hEnemy == NULL )
				{
					if ( !m_hTargetEnt->IsAlive() )
					{
						// UNDONE: Comment about the recently dead player here?
						StopFollowing( FALSE );
						break;
					}
					return GetScheduleOfType( SCHED_TARGET_FACE );
				}
			}

			// If you hear a sound that you can't see the source of (and isn't a grenade) 
			// then crouch and say "Shhh... I hear something".  
	//		if (m_iPlayerReact == 0)
	//		{
				if ( pSound && pSound->FIsSound() )
				{
					if ( !FVisible( pSound->m_vecOrigin ) )
					{
						return GetScheduleOfType( SCHED_HUMAN_HEAR_SOUND );
					}
					else
					{
				//		if (m_iPlayerReact == 0 || FStringNull(m_iPlayerReact))
						{
							if ( pSound && (pSound->m_iType & bits_SOUND_COMBAT) )
							return GetScheduleOfType( SCHED_INVESTIGATE_SOUND );
							else
							return GetScheduleOfType( SCHED_ALERT_FACE );
						}
					}
				}
	//		}
		}

		break;
	}

	return CSquadMonster::GetSchedule();
}


//=========================================================
// GetIdealState - well you know
//=========================================================

MONSTERSTATE CBaseHuman :: GetIdealState ( void )
{
	return CSquadMonster::GetIdealState();
}


//=========================================================
// Custom Schedules
//=========================================================

//=========================================================
// run to cover.
//=========================================================

Task_t	tlHumanTakeCover[] =
{
	{ TASK_STOP_MOVING,				(float)0							},
	{ TASK_SET_FAIL_SCHEDULE,		(float)SCHED_HUMAN_TAKECOVER_FAILED	},
	{ TASK_WAIT,					(float)0.1							},
	{ TASK_FIND_COVER_FROM_ENEMY,	(float)0							},
	{ TASK_HUMAN_SOUND_COVER,		(float)0.5							},
	{ TASK_RUN_PATH,				(float)0							},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0							},
	{ TASK_REMEMBER,				(float)bits_MEMORY_INCOVER			},
	{ TASK_SET_SCHEDULE,			(float)SCHED_HUMAN_WAIT_FACE_ENEMY	},
};

Schedule_t	slHumanTakeCover[] =
{
	{ 
		tlHumanTakeCover,
		ARRAYSIZE ( tlHumanTakeCover ), 
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"HumanTakeCover"
	},
};

//=========================================================
// Retreat - like slHumanTakeCover except in response to an
// order by squad commander
//=========================================================

Task_t	tlHumanRetreat[] =
{
	{ TASK_STOP_MOVING,				(float)0							},
	{ TASK_SET_FAIL_SCHEDULE,		(float)SCHED_HUMAN_TAKECOVER_FAILED	},
	{ TASK_FIND_COVER_FROM_ENEMY,	(float)0							},
	{ TASK_HUMAN_SOUND_RETREATING,	(float)0							},
	{ TASK_RUN_PATH,				(float)0							},	
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0							},
	{ TASK_REMEMBER,				(float)bits_MEMORY_INCOVER			},
	{ TASK_SET_SCHEDULE,			(float)SCHED_HUMAN_WAIT_FACE_ENEMY	},
};

Schedule_t	slHumanRetreat[] =
{
	{ 
		tlHumanRetreat,
		ARRAYSIZE ( tlHumanRetreat ), 
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"HumanRetreat"
	},
};

//=========================================================
// drop grenade then run to cover.
//=========================================================

Task_t	tlHumanGrenadeCover[] =
{
	{ TASK_STOP_MOVING,						(float)0							},
	{ TASK_SET_FAIL_SCHEDULE,				(float)SCHED_HUMAN_TAKE_COVER_FROM_ENEMY_NO_GRENADE },
	{ TASK_FIND_FAR_NODE_COVER_FROM_ENEMY,	(float)384							},
	{ TASK_PLAY_SEQUENCE,					(float)ACT_SPECIAL_ATTACK1			},
	{ TASK_CLEAR_MOVE_WAIT,					(float)0							},
	{ TASK_HUMAN_SOUND_COVER,				(float)0							},
	{ TASK_RUN_PATH,						(float)0							},
	{ TASK_WAIT_FOR_MOVEMENT,				(float)0							},
	{ TASK_SET_SCHEDULE,					(float)SCHED_HUMAN_WAIT_FACE_ENEMY	},
};

Schedule_t	slHumanGrenadeCover[] =
{
	{ 
		tlHumanGrenadeCover,
		ARRAYSIZE ( tlHumanGrenadeCover ), 
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"HumanGrenadeCover"
	},
};


//=========================================================
// toss grenade then run to cover.
//=========================================================
Task_t	tlHumanTossGrenadeCover[] =
{
	{ TASK_FACE_ENEMY,						(float)0							},
	{ TASK_HUMAN_SOUND_THROW,				(float)0.5							},
	{ TASK_RANGE_ATTACK2, 					(float)0							},
	{ TASK_SET_SCHEDULE,					(float)SCHED_HUMAN_TAKE_COVER_FROM_ENEMY_NO_GRENADE	},
};

Schedule_t	slHumanTossGrenadeCover[] =
{
	{ 
		tlHumanTossGrenadeCover,
		ARRAYSIZE ( tlHumanTossGrenadeCover ), 
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"HumanTossGrenadeCover"
	},
};


//=========================================================
// hide from the loudest sound source (to run from grenade)
// Over-rides base because we want human to crouch and say
// "grenade!"
//=========================================================

Task_t	tlHumanTakeCoverFromBestSound[] =
{
	{ TASK_SET_FAIL_SCHEDULE,			(float)SCHED_COWER			},// duck and cover if cannot move from explosion
	{ TASK_STOP_MOVING,					(float)0					},
	{ TASK_FIND_COVER_FROM_BEST_SOUND,	(float)0					},
	{ TASK_HUMAN_SOUND_GRENADE,			(float)0.5					},
	{ TASK_HUMAN_CROUCH,				(float)0					},
	{ TASK_RUN_PATH,					(float)0					},
	{ TASK_WAIT_FOR_MOVEMENT,			(float)0					},
	{ TASK_REMEMBER,					(float)bits_MEMORY_INCOVER	},
	{ TASK_TURN_LEFT,					(float)179					},
};

Schedule_t	slHumanTakeCoverFromBestSound[] =
{
	{ 
		tlHumanTakeCoverFromBestSound,
		ARRAYSIZE ( tlHumanTakeCoverFromBestSound ), 
		0,
		0,
		"HumanTakeCoverFromBestSound"
	},
};


//=========================================================
// Cower - this is what is usually done when attempts
// to escape danger fail.
// Over-rides base Cower because we want the human to crouch
//=========================================================

Task_t	tlHumanCower[] =
{
	{ TASK_STOP_MOVING,			0					},
	{ TASK_HUMAN_CROUCH,		(float)0			},
	{ TASK_PLAY_SEQUENCE,		(float)ACT_COWER	},
};

Schedule_t	slHumanCower[] =
{
	{
		tlHumanCower,
		ARRAYSIZE ( tlHumanCower ),
		0,
		0,
		"HumanCower"
	},
};


//=========================================================
// Hear Sound Schedule - crouch and go "Shh"
//=========================================================

Task_t	tlHumanHearSound[] =
{
	{ TASK_STOP_MOVING,				0				},
	{ TASK_HUMAN_CROUCH,			(float)0		},
	{ TASK_HUMAN_SOUND_HEAR,		(float)0.5		},
	{ TASK_SET_ACTIVITY,			(float)ACT_IDLE	},
	{ TASK_FACE_IDEAL,				(float)0		},
	{ TASK_WAIT,					(float)3		},
};

Schedule_t	slHumanHearSound[] =
{
	{ 
		tlHumanHearSound,
		ARRAYSIZE ( tlHumanHearSound ),
		bits_COND_NEW_ENEMY		|
		bits_COND_SEE_FEAR		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_PROVOKED		|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"Human Hear Sound"
	},
};


//=========================================================
// HumanInvestigateSound - sends a monster to the location of the
// sound that was just heard, to check things out. 
// Over-rides base InvestigateSound because we want the human
// to crouch and say "Shh"
//=========================================================

Task_t tlHumanInvestigateSound[] =
{
	{ TASK_STOP_MOVING,				(float)0				},
	{ TASK_STORE_LASTPOSITION,		(float)0				},
	{ TASK_GET_PATH_TO_BESTSOUND,	(float)0				},
	{ TASK_HUMAN_CROUCH,			(float)0				},
	{ TASK_HUMAN_SOUND_HEAR,		(float)0.5				},
	{ TASK_FACE_IDEAL,				(float)0				},
	{ TASK_WALK_PATH,				(float)0				},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0				},
	{ TASK_SET_ACTIVITY,			(float)ACT_IDLE_ANGRY	},
	{ TASK_WAIT,					(float)10				},
	{ TASK_GET_PATH_TO_LASTPOSITION,(float)0				},
	{ TASK_WALK_PATH,				(float)0				},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0				},
	{ TASK_CLEAR_LASTPOSITION,		(float)0				},
};

Schedule_t	slHumanInvestigateSound[] =
{
	{ 
		tlHumanInvestigateSound,
		ARRAYSIZE ( tlHumanInvestigateSound ), 
		bits_COND_NEW_ENEMY			|
		bits_COND_SEE_FEAR			|
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"HumanInvestigateSound"
	},
};


//=========================================================
// Victory Dance
//=========================================================

Task_t tlHumanVictoryDance[] =
{
	{ TASK_STOP_MOVING,			0							},
	{ TASK_HUMAN_UNCROUCH,		(float)0					},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE				},
	{ TASK_HUMAN_SOUND_VICTORY,	(float)0					},
	{ TASK_WAIT,				(float)4					},
	{ TASK_SUGGEST_STATE,		(float)MONSTERSTATE_IDLE	},
};

Schedule_t slHumanVictoryDance[] =
{
	{
		tlHumanVictoryDance,
		ARRAYSIZE( tlHumanVictoryDance ),
		bits_COND_NEW_ENEMY			|
		bits_COND_SEE_FEAR			|
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_HEAR_SOUND |
		bits_COND_PROVOKED,
		bits_SOUND_DANGER,
		"Human Victory Dance"
	},
};


//=========================================================
// Hide and reload schedule
//=========================================================

Task_t	tlHumanHideReload[] =
{
	{ TASK_STOP_MOVING,				(float)0					},
	{ TASK_HUMAN_CROUCH,			(float)0					},
	{ TASK_SET_FAIL_SCHEDULE,		(float)SCHED_RELOAD			},
	{ TASK_FIND_COVER_FROM_ENEMY,	(float)0					},
	{ TASK_RUN_PATH,				(float)0					},	
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0					},
	{ TASK_REMEMBER,				(float)bits_MEMORY_INCOVER	},
	{ TASK_FACE_ENEMY,				(float)0					},
	{ TASK_PLAY_SEQUENCE,			(float)ACT_RELOAD			},
};

Schedule_t slHumanHideReload[] = 
{
	{
		tlHumanHideReload,
		ARRAYSIZE ( tlHumanHideReload ),
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_HEAR_SOUND,

		bits_SOUND_DANGER,
		"HumanHideReload"
	}
};


//=========================================================
// Reload schedule, overrides base class because we need
// to crouch
//=========================================================

Task_t	tlHumanReload[] =
{
	{ TASK_STOP_MOVING,				(float)0					},
	{ TASK_HUMAN_CROUCH,			(float)0					},
	{ TASK_PLAY_SEQUENCE,			(float)ACT_RELOAD			},
};

Schedule_t slHumanReload[] = 
{
	{
		tlHumanReload,
		ARRAYSIZE ( tlHumanReload ),
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_HEAR_SOUND,

		bits_SOUND_DANGER,
		"HumanReload"
	}
};


//=========================================================
// Signal squad to defend me, Find Medic, 
// or if you can't just run like a bastard
//=========================================================

Task_t tlHumanFindMedicCombat[] =
{
	{ TASK_SET_FAIL_SCHEDULE,			(float)SCHED_TAKE_COVER_FROM_ENEMY	},
	{ TASK_STOP_MOVING,					(float)0					},
	{ TASK_HUMAN_SOUND_HELP,			(float)0					},
	{ TASK_HUMAN_FIND_MEDIC,			(float)0					},
	{ TASK_GET_PATH_TO_TARGET,			(float)0					},
	{ TASK_RUN_PATH,					(float)0					},
	{ TASK_WAIT_FOR_MOVEMENT,			(float)0					},
	{ TASK_FACE_TARGET,					(float)179					},
};

Schedule_t	slHumanFindMedicCombat[] =
{
	{ 
		tlHumanFindMedicCombat,
		ARRAYSIZE ( tlHumanFindMedicCombat ), 
		bits_COND_HEAR_SOUND,
		bits_SOUND_DANGER,
		"HumanFindMedicCombat"
	},
};


//=========================================================
// Find Medic
//=========================================================

Task_t tlHumanFindMedic[] =
{
	{ TASK_STOP_MOVING,					(float)0					},
	{ TASK_HUMAN_SOUND_MEDIC,			(float)0					},
	{ TASK_HUMAN_FIND_MEDIC,			(float)0					},
	{ TASK_GET_PATH_TO_TARGET,			(float)0					},
	{ TASK_RUN_PATH,					(float)0					},
	{ TASK_WAIT_FOR_MOVEMENT,			(float)0					},
	{ TASK_FACE_TARGET,					(float)179					},
};

Schedule_t	slHumanFindMedic[] =
{
	{ 
		tlHumanFindMedic,
		ARRAYSIZE ( tlHumanFindMedic ), 
		bits_COND_NEW_ENEMY			|
		bits_COND_SEE_FEAR			|
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_HEAR_SOUND		|
		bits_COND_PROVOKED,
		bits_SOUND_DANGER,
		"HumanFindMedic"
	},
};


//=========================================================
// SignalAttack - Signal Attack and then start shooting
//=========================================================

Task_t	tlHumanSignalAttack[] =
{
	{ TASK_STOP_MOVING,					0						},
	{ TASK_FACE_IDEAL,					(float)0				},
	{ TASK_HUMAN_SOUND_ATTACK,			(float)0				},		// This will do a hand signal as well
	{ TASK_SET_SCHEDULE,				(float)SCHED_RANGE_ATTACK1 },
};

Schedule_t	slHumanSignalAttack[] =
{
	{ 
		tlHumanSignalAttack,
		ARRAYSIZE ( tlHumanSignalAttack ), 
		bits_COND_ENEMY_DEAD		|
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_HEAR_SOUND		|
		bits_COND_NO_AMMO_LOADED,

		bits_SOUND_DANGER,
		"HumanSignalAttack"
	},
};


//=========================================================
// SignalSurpress - Signal to lay down supressing fire and then 
// lay some down yourself
//=========================================================

Task_t	tlHumanSignalSurpress[] =
{
	{ TASK_STOP_MOVING,					0						},
	{ TASK_FACE_IDEAL,					(float)0				},
	{ TASK_HUMAN_SOUND_SURPRESS,		(float)0				},
	{ TASK_SET_SCHEDULE,				(float)SCHED_HUMAN_SURPRESS },
};

Schedule_t	slHumanSignalSurpress[] =
{
	{ 
		tlHumanSignalSurpress,
		ARRAYSIZE ( tlHumanSignalSurpress ), 
		bits_COND_HEAR_SOUND,

		bits_SOUND_DANGER,
		"HumanSignalSurpress"
	},
};


//=========================================================
// SignalSearchAndDestroy - Signal to Search and Destroy
// lay some down yourself
//=========================================================

Task_t	tlHumanSignalSearchAndDestroy[] =
{
	{ TASK_STOP_MOVING,					0						},
	{ TASK_FACE_IDEAL,					(float)0				},
	{ TASK_HUMAN_SOUND_SEARCH_AND_DESTROY,	(float)0			},
	{ TASK_SET_ACTIVITY,				(float)ACT_IDLE_ANGRY	},
	{ TASK_WAIT,						(float)2				}, // Wait for your troops to get going before you follow
	{ TASK_SET_SCHEDULE,				(float)SCHED_HUMAN_SEARCH_AND_DESTROY },
};

Schedule_t	slHumanSignalSearchAndDestroy[] =
{
	{ 
		tlHumanSignalSearchAndDestroy,
		ARRAYSIZE ( tlHumanSignalSearchAndDestroy ), 
		bits_COND_HEAR_SOUND		|
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE,

		bits_SOUND_DANGER,
		"HumanSignalSearchAndDestroy"
	},
};


//=========================================================
// SignalRetreat - Signal to retreat, then shoot at the
// enemy for a bit so the others get a chance to run before
// running yourself
//=========================================================

Task_t	tlHumanSignalRetreat[] =
{
	{ TASK_STOP_MOVING,				0									},
	{ TASK_FACE_IDEAL,				(float)0							},
	{ TASK_HUMAN_SOUND_RETREAT,		(float)0							},	// Plays retreat signal as well
	{ TASK_SET_FAIL_SCHEDULE,		(float)SCHED_TAKE_COVER_FROM_ENEMY	}, // May fail if CheckFire fails, 
	{ TASK_FACE_ENEMY,				(float)0							},// i.e. I could hit a friendly
	{ TASK_HUMAN_CHECK_FIRE,		(float)0							},
	{ TASK_RANGE_ATTACK1,			(float)0							},
	{ TASK_FACE_ENEMY,				(float)0							},
	{ TASK_HUMAN_CHECK_FIRE,		(float)0							},
	{ TASK_RANGE_ATTACK1,			(float)0							},
	{ TASK_FACE_ENEMY,				(float)0							},
	{ TASK_HUMAN_CHECK_FIRE,		(float)0							},
	{ TASK_RANGE_ATTACK1,			(float)0							},
	{ TASK_FACE_ENEMY,				(float)0							},
	{ TASK_HUMAN_CHECK_FIRE,		(float)0							},
	{ TASK_RANGE_ATTACK1,			(float)0							},
	{ TASK_SET_FAIL_SCHEDULE,		(float)SCHED_HUMAN_TAKECOVER_FAILED	},
	{ TASK_WAIT,					(float)0.2							},
	{ TASK_FIND_COVER_FROM_ENEMY,	(float)0							},
	{ TASK_RUN_PATH,				(float)0							},	
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0							},
	{ TASK_REMEMBER,				(float)bits_MEMORY_INCOVER			},
	{ TASK_SET_SCHEDULE,			(float)SCHED_HUMAN_WAIT_FACE_ENEMY	},
};

Schedule_t	slHumanSignalRetreat[] =
{
	{ 
		tlHumanSignalRetreat,
		ARRAYSIZE ( tlHumanSignalRetreat ), 
		bits_COND_HEAR_SOUND		|
		bits_COND_NEW_ENEMY			|
		bits_COND_ENEMY_DEAD		|
		bits_COND_NO_AMMO_LOADED	|
		bits_COND_PROVOKED,

		bits_SOUND_DANGER,
		"HumanSignalRetreat"
	},
};


//=========================================================
// SignalComeToMe - Stand up and signal to come to me
//=========================================================

Task_t	tlHumanSignalComeToMe[] =
{
	{ TASK_STOP_MOVING,					0						},
	{ TASK_FACE_IDEAL,					(float)0				},
	{ TASK_HUMAN_UNCROUCH,				(float)0				},
	{ TASK_HUMAN_SOUND_COME_TO_ME,		(float)0				},
	{ TASK_SET_ACTIVITY,				(float)ACT_IDLE			},
	{ TASK_WAIT,						(float)10				},	// Wait a good long time for everyone to get there
};

Schedule_t	slHumanSignalComeToMe[] =
{
	{ 
		tlHumanSignalComeToMe,
		ARRAYSIZE ( tlHumanSignalComeToMe ), 
		bits_COND_HEAR_SOUND		|
		bits_COND_NEW_ENEMY			|
		bits_COND_SEE_FEAR			|
		bits_COND_SEE_DISLIKE		|
		bits_COND_SEE_HATE			|
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_PROVOKED,

		bits_SOUND_DANGER,
		"HumanSignalComeToMe"
	},
};


//=========================================================
// SignalCheckIn - get reports from all my troops to find
// if they are fighting anyone
//=========================================================

Task_t	tlHumanSignalCheckIn[] =
{
	{ TASK_STOP_MOVING,					0							},
	{ TASK_SET_ACTIVITY,				(float)ACT_IDLE				},
	{ TASK_HUMAN_SOUND_CHECK_IN,		(float)0					},
	{ TASK_WAIT,						(float)MAX_SQUAD_MEMBERS	},	// Wait for my squad to reply
};

Schedule_t	slHumanSignalCheckIn[] =
{
	{ 
		tlHumanSignalCheckIn,
		ARRAYSIZE ( tlHumanSignalCheckIn ), 
		bits_COND_HEAR_SOUND		|
		bits_COND_NEW_ENEMY			|
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_PROVOKED,

		bits_SOUND_DANGER,
		"HumanSignalCheckIn"
	},
};


//=========================================================
// SignalCheckIn - get reports from all my troops to find
// if they are fighting anyone
//=========================================================

Task_t	tlHumanCheckIn[] =
{
	{ TASK_STOP_MOVING,					0							},
	{ TASK_SET_ACTIVITY,				(float)ACT_IDLE				},
	{ TASK_HUMAN_SOUND_CLEAR,			(float)0					},
	{ TASK_WAIT,						(float)1					},
};

Schedule_t	slHumanCheckIn[] =
{
	{ 
		tlHumanCheckIn,
		ARRAYSIZE ( tlHumanCheckIn ), 
		bits_COND_HEAR_SOUND		|
		bits_COND_NEW_ENEMY			|
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_PROVOKED,

		bits_SOUND_DANGER,
		"HumanCheckIn"
	},
};


//=========================================================
// HumanFoundEnemy - human established sight with an enemy
// that was hiding from the squad.
//=========================================================

Task_t	tlHumanFoundEnemy[] =
{
	{ TASK_STOP_MOVING,				0							},
	{ TASK_FACE_ENEMY,				(float)0					},
	{ TASK_HUMAN_SOUND_FOUND_ENEMY,	(float)0					},
};

Schedule_t	slHumanFoundEnemy[] =
{
	{ 
		tlHumanFoundEnemy,
		ARRAYSIZE ( tlHumanFoundEnemy ), 
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"HumanFoundEnemy"
	},
};


//=========================================================
// primary range attack. Overriden because base class stops attacking when the enemy is occluded.
// human's grenade toss requires the enemy be occluded.
//=========================================================

Task_t	tlHumanRangeAttack1[] =
{
	{ TASK_STOP_MOVING,			(float)0		},
	{ TASK_SET_FAIL_SCHEDULE,	(float)SCHED_HUMAN_ESTABLISH_LINE_OF_FIRE }, // May fail if CheckFire fails, 
	{ TASK_FACE_ENEMY,			(float)0		},// i.e. I could hit a friendly
	{ TASK_WAIT_FACE_ENEMY,		(float)0.1		},//sys new - dont act like robots + SEXON didn't like it

	{ TASK_HUMAN_CHECK_FIRE,	(float)0		},
	{ TASK_HUMAN_SOUND_CHARGE,	(float)0.1		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_HUMAN_CHECK_FIRE,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_HUMAN_CHECK_FIRE,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_HUMAN_CHECK_FIRE,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_HUMAN_CHECK_FIRE,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
};

Schedule_t	slHumanRangeAttack1[] =
{
	{ 
		tlHumanRangeAttack1,
		ARRAYSIZE ( tlHumanRangeAttack1 ), 
		bits_COND_NEW_ENEMY			|
		bits_COND_ENEMY_DEAD		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_ENEMY_OCCLUDED	|
		bits_COND_HEAR_SOUND		|
		bits_COND_NO_AMMO_LOADED,
		
		bits_SOUND_DANGER,
		"Range Attack1"
	},
};


//=========================================================
// primary range attack. Overriden because base class stops attacking when the enemy is occluded.
// human's grenade toss requires the enemy be occluded.
//=========================================================

Task_t	tlHumanRangeSniperAttack1[] =
{
	{ TASK_STOP_MOVING,			(float)0		},
	{ TASK_SET_FAIL_SCHEDULE,	(float)SCHED_HUMAN_ESTABLISH_LINE_OF_FIRE }, // May fail if CheckFire fails, 
	{ TASK_FACE_ENEMY,			(float)0		},// i.e. I could hit a friendly
	{ TASK_WAIT_FACE_ENEMY,		(float)3.0		},

	{ TASK_HUMAN_CHECK_FIRE,	(float)0		},
	{ TASK_HUMAN_SOUND_CHARGE,	(float)0.1		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_HUMAN_CHECK_FIRE,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_HUMAN_CHECK_FIRE,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_HUMAN_CHECK_FIRE,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_HUMAN_CHECK_FIRE,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
};

Schedule_t	slHumanRangeSniperAttack1[] =
{
	{ 
		tlHumanRangeSniperAttack1,
		ARRAYSIZE ( tlHumanRangeSniperAttack1 ), 
		bits_COND_NEW_ENEMY			|
		bits_COND_ENEMY_DEAD		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_ENEMY_OCCLUDED	|
		bits_COND_HEAR_SOUND		|
		bits_COND_NO_AMMO_LOADED,
		
		bits_SOUND_DANGER,
		"Range Sniper Attack1"
	},
};


//=========================================================
// secondary range attack. Overriden because base class stops attacking when the enemy is occluded.
// grenade toss requires the enemy be occluded.
//=========================================================

Task_t	tlHumanRangeAttack2[] =
{
	{ TASK_STOP_MOVING,				(float)0					},
	{ TASK_HUMAN_FACE_TOSS_DIR,		(float)0					},
	{ TASK_HUMAN_SOUND_THROW,		(float)0.5					},
	{ TASK_PLAY_SEQUENCE,			(float)ACT_RANGE_ATTACK2	},
	{ TASK_SET_SCHEDULE,			(float)SCHED_HUMAN_WAIT_FACE_ENEMY	},// don't run immediately after throwing grenade.
};

Schedule_t	slHumanRangeAttack2[] =
{
	{ 
		tlHumanRangeAttack2,
		ARRAYSIZE ( tlHumanRangeAttack2 ), 
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"HumanRangeAttack2"
	},
};


//=========================================================
// Establish line of fire - move to a position that allows
// the human to attack.
//=========================================================

Task_t tlHumanEstablishLineOfFire[] = 
{
	{ TASK_SET_FAIL_SCHEDULE,	(float)SCHED_HUMAN_ELOF_FAIL	},
	{ TASK_STOP_MOVING,			(float)0						},
	{ TASK_FACE_ENEMY,			(float)0						},
	{ TASK_GET_PATH_TO_ENEMY,	(float)0						},
	{ TASK_HUMAN_SOUND_CHARGE,	(float)0.7						},
	{ TASK_RUN_PATH,			(float)0						},
	{ TASK_WAIT_FOR_MOVEMENT,	(float)0						},
};

Schedule_t slHumanEstablishLineOfFire[] =
{
	{ 
		tlHumanEstablishLineOfFire,
		ARRAYSIZE ( tlHumanEstablishLineOfFire ),
		bits_COND_NEW_ENEMY			|
		bits_COND_ENEMY_DEAD		|
		bits_COND_CAN_RANGE_ATTACK1	|
		bits_COND_CAN_MELEE_ATTACK1	|
		bits_COND_CAN_RANGE_ATTACK2	|
		bits_COND_CAN_MELEE_ATTACK2	|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"HumanEstablishLineOfFire"
	},
};


//=========================================================
// Explosive Establish line of fire - move to a position 
// that allows the human to attack with an explosive weapon 
// and not damage himself.
//=========================================================

Task_t tlHumanExplosiveELOF[] = 
{
	{ TASK_SET_FAIL_SCHEDULE,	(float)SCHED_HUMAN_MELEE_ELOF	},
	{ TASK_HUMAN_GET_EXPLOSIVE_PATH_TO_ENEMY,	(float)0		},
	{ TASK_HUMAN_SOUND_CHARGE,	(float)0.7						},
	{ TASK_RUN_PATH,			(float)0						},
	{ TASK_WAIT_FOR_MOVEMENT,	(float)0						},
};

Schedule_t slHumanExplosiveELOF[] =
{
	{ 
		tlHumanExplosiveELOF,
		ARRAYSIZE ( tlHumanExplosiveELOF ),
		bits_COND_NEW_ENEMY			|
		bits_COND_ENEMY_DEAD		|
		bits_COND_CAN_RANGE_ATTACK1	|
		bits_COND_CAN_MELEE_ATTACK1	|
		bits_COND_CAN_RANGE_ATTACK2	|
		bits_COND_CAN_MELEE_ATTACK2	|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"HumanExplosiveELOF"
	},
};


//=========================================================
// Melee Establish line of fire - move to a position 
//	where I can kick the enemy
//=========================================================

Task_t tlHumanMeleeELOF[] = 
{
	{ TASK_SET_FAIL_SCHEDULE,	(float)SCHED_TAKE_COVER_FROM_ENEMY	},
	{ TASK_HUMAN_GET_MELEE_PATH_TO_ENEMY,	(float)0			},
	{ TASK_HUMAN_SOUND_CHARGE,	(float)0.7						},
	{ TASK_RUN_PATH,			(float)0						},
	{ TASK_WAIT_FOR_MOVEMENT,	(float)0						},
};

Schedule_t slHumanMeleeELOF[] =
{
	{ 
		tlHumanMeleeELOF,
		ARRAYSIZE ( tlHumanMeleeELOF ),
		bits_COND_NEW_ENEMY			|
		bits_COND_ENEMY_DEAD		|
		bits_COND_CAN_RANGE_ATTACK1	|
		bits_COND_CAN_MELEE_ATTACK1	|
		bits_COND_CAN_RANGE_ATTACK2	|
		bits_COND_CAN_MELEE_ATTACK2	|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"HumanMeleeELOF"
	},
};


//=========================================================
// Standoff schedule. Used in combat when a monster is 
// hiding in cover or the enemy has moved out of sight. 
// Should we look around in this schedule?
// Over-ridden because we want human to crouch and possibly taunt enemy
//=========================================================

Task_t	tlHumanStandoff[] =
{
	{ TASK_STOP_MOVING,				(float)0					},
	{ TASK_HUMAN_CROUCH,			(float)0					},
	{ TASK_SET_ACTIVITY,			(float)ACT_IDLE				},
	{ TASK_HUMAN_SOUND_TAUNT,		(float)0.2					},
	{ TASK_WAIT_FACE_ENEMY,			(float)0					},//3
};

Schedule_t slHumanStandoff[] = 
{
	{
		tlHumanStandoff,
		ARRAYSIZE ( tlHumanStandoff ),
		bits_COND_CAN_MELEE_ATTACK1		|
		bits_COND_CAN_MELEE_ATTACK2		|
		bits_COND_CAN_RANGE_ATTACK1		|
		bits_COND_CAN_RANGE_ATTACK2		|
		bits_COND_ENEMY_DEAD			|
		bits_COND_NEW_ENEMY				|
		bits_COND_LIGHT_DAMAGE			|
		bits_COND_HEAVY_DAMAGE			|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_PLAYER				|
		bits_SOUND_DANGER,
		"HumanStandoff"
	}
};


//=========================================================
// UnCrouch - get up and make a remark
//=========================================================

Task_t tlHumanUnCrouch[] =
{
	{ TASK_HUMAN_UNCROUCH,			(float)0					},
	{ TASK_SOUND_IDLE,				(float)0					},
	{ TASK_SET_ACTIVITY,			(float)ACT_IDLE				},
	{ TASK_WAIT,					(float)0.1					},//2
};

Schedule_t slHumanUnCrouch[] =
{
	{ 
		tlHumanUnCrouch,
		ARRAYSIZE ( tlHumanUnCrouch ),

		bits_COND_PROVOKED			|
		bits_COND_NEW_ENEMY			|
		bits_COND_SEE_FEAR			|
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"HumanUnCrouch"
	}
};


//=========================================================
// UnCrouch - get up and perform a scripted sequence
//=========================================================

Task_t tlHumanUnCrouchScript[] =
{
	{ TASK_HUMAN_UNCROUCH,			(float)0					},
	{ TASK_SET_SCHEDULE,			(float)SCHED_AISCRIPT		},
};

Schedule_t slHumanUnCrouchScript[] =
{
	{ 
		tlHumanUnCrouchScript,
		ARRAYSIZE ( tlHumanUnCrouchScript ),
		0,
		0,
		"HumanUnCrouchScript"
	}
};


//=========================================================
// Human wait in cover - we don't allow danger or the ability
// to attack to break a human's run to cover schedule, but
// when a human is in cover, we do want them to attack if they can.
//=========================================================

Task_t	tlHumanWaitInCover[] =
{
	{ TASK_STOP_MOVING,				(float)0					},
	{ TASK_SET_ACTIVITY,			(float)ACT_IDLE				},
	{ TASK_WAIT_FACE_ENEMY,			(float)1					},
};

Schedule_t	slHumanWaitInCover[] =
{
	{ 
		tlHumanWaitInCover,
		ARRAYSIZE ( tlHumanWaitInCover ), 
		bits_COND_NEW_ENEMY			|
		bits_COND_HEAR_SOUND		|
		bits_COND_CAN_RANGE_ATTACK1	|
		bits_COND_CAN_RANGE_ATTACK2	|
		bits_COND_CAN_MELEE_ATTACK1	|
		bits_COND_CAN_MELEE_ATTACK2,

		bits_SOUND_DANGER,
		"HumanWaitInCover"
	},
};


//=========================================================
// HumanFollow - Move to within 128 of target ent
//=========================================================

Task_t	tlHumanFollow[] =
{
	{ TASK_MOVE_TO_TARGET_RANGE,(float)128					},	// Move within 128 of target ent
	{ TASK_SET_SCHEDULE,		(float)SCHED_TARGET_FACE	},
};

Schedule_t	slHumanFollow[] =
{
	{
		tlHumanFollow,
		ARRAYSIZE ( tlHumanFollow ),
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_HEAR_SOUND |
		bits_COND_PROVOKED,
		bits_SOUND_DANGER,
		"HumanFollow"
	},
};


//=========================================================
// HumanFollowClose - Move to within 64 of target ent
//=========================================================

Task_t	tlHumanFollowClose[] =
{
	{ TASK_MOVE_TO_TARGET_RANGE,(float)64					},	// Move within 64 of target ent
	{ TASK_SET_SCHEDULE,		(float)SCHED_TARGET_FACE	},
};

Schedule_t	slHumanFollowClose[] =
{
	{
		tlHumanFollowClose,
		ARRAYSIZE ( tlHumanFollowClose ),
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_HEAR_SOUND |
		bits_COND_PROVOKED,
		bits_SOUND_DANGER,
		"HumanFollowClose"
	},
};


//=========================================================
// HumanTurnRound - spin round very fast
//=========================================================

Task_t	tlHumanTurnRound[] =
{
	{ TASK_STOP_MOVING,			(float)0				},
	{ TASK_TURN_LEFT,			(float)179				},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE_ANGRY	},
	{ TASK_WAIT,				(float)1				},
};

Schedule_t	slHumanTurnRound[] =
{
	{
		tlHumanTurnRound,
		ARRAYSIZE ( tlHumanTurnRound ),
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_PROVOKED		|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"HumanTurnRound"
	},
};


//=========================================================
// Surpress - just fire like a mad bastard at the last 
// place you saw the enemy until your clip is all gone
//=========================================================

Task_t	tlHumanSuppress[] =
{
	{ TASK_STOP_MOVING,					(float)0			},
	{ TASK_FACE_ENEMY,					(float)0			},
	{ TASK_HUMAN_CHECK_FIRE,			(float)0			},
	{ TASK_HUMAN_SOUND_SURPRESSING,		(float)0			},
	{ TASK_RANGE_ATTACK1,				(float)0			},
	{ TASK_FACE_ENEMY,					(float)0			},
	{ TASK_HUMAN_CHECK_FIRE,			(float)0			},
	{ TASK_RANGE_ATTACK1,				(float)0			},
	{ TASK_FACE_ENEMY,					(float)0			},
	{ TASK_HUMAN_CHECK_FIRE,			(float)0			},
	{ TASK_RANGE_ATTACK1,				(float)0			},
	{ TASK_FACE_ENEMY,					(float)0			},
	{ TASK_HUMAN_CHECK_FIRE,			(float)0			},
	{ TASK_RANGE_ATTACK1,				(float)0			},
	{ TASK_FACE_ENEMY,					(float)0			},
	{ TASK_HUMAN_CHECK_FIRE,			(float)0			},
	{ TASK_RANGE_ATTACK1,				(float)0			},
	{ TASK_FACE_ENEMY,					(float)0			},
	{ TASK_HUMAN_CHECK_FIRE,			(float)0			},
	{ TASK_RANGE_ATTACK1,				(float)0			},
	{ TASK_FACE_ENEMY,					(float)0			},
	{ TASK_HUMAN_CHECK_FIRE,			(float)0			},
	{ TASK_RANGE_ATTACK1,				(float)0			},
	{ TASK_FACE_ENEMY,					(float)0			},
	{ TASK_HUMAN_CHECK_FIRE,			(float)0			},
	{ TASK_RANGE_ATTACK1,				(float)0			},
	{ TASK_FACE_ENEMY,					(float)0			},
	{ TASK_HUMAN_CHECK_FIRE,			(float)0			},
	{ TASK_RANGE_ATTACK1,				(float)0			},
	{ TASK_FACE_ENEMY,					(float)0			},
	{ TASK_HUMAN_CHECK_FIRE,			(float)0			},
	{ TASK_RANGE_ATTACK1,				(float)0			},
};

Schedule_t	slHumanSurpress[] =
{
	{ 
		tlHumanSuppress,
		ARRAYSIZE ( tlHumanSuppress ), 
		bits_COND_NEW_ENEMY			|
		bits_COND_ENEMY_DEAD		|
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_HEAR_SOUND		|
		bits_COND_NO_AMMO_LOADED	|
		bits_COND_PROVOKED,

		bits_SOUND_DANGER,
		"HumanSuppress"
	},
};


//=========================================================
// Human - move to within range of the enemy's last known position
//=========================================================

Task_t tlHumanMoveToEnemyLKP[] = 
{
	{ TASK_SET_FAIL_SCHEDULE,			(float)SCHED_HUMAN_SURPRESS		},
	{ TASK_GET_PATH_TO_ENEMY_LKP,		(float)0						},
	{ TASK_RUN_PATH,					(float)0						},
	{ TASK_HUMAN_WAIT_GOAL_VISIBLE,		(float)0						},
	{ TASK_STOP_MOVING,					(float)0						},
};

Schedule_t slHumanMoveToEnemyLKP[] =
{
	{ 
		tlHumanMoveToEnemyLKP,
		ARRAYSIZE ( tlHumanMoveToEnemyLKP ),
		bits_COND_NEW_ENEMY			|
		bits_COND_CAN_RANGE_ATTACK1	|
		bits_COND_CAN_MELEE_ATTACK1	|
		bits_COND_CAN_RANGE_ATTACK2	|
		bits_COND_CAN_MELEE_ATTACK2	|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"HumanMoveToEnemyLKP"
	},
};


//=========================================================
// HumanSearchAndDestroy - Go on a wide loop in the hopes of
// locating an enemy
//=========================================================

Task_t tlHumanSearchAndDestroy[] =
{
	{ TASK_STOP_MOVING,				(float)0				},
	{ TASK_STORE_LASTPOSITION,		(float)0				},
	{ TASK_FIND_FAR_NODE_COVER_FROM_ORIGIN,  (float)512		},
	{ TASK_FACE_IDEAL,				(float)0				},
	{ TASK_HUMAN_SOUND_SEARCHING,	(float)0				},
	{ TASK_RUN_PATH,				(float)0				},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0				},
	{ TASK_PLAY_SEQUENCE,			(float)ACT_IDLE_ANGRY	},
	{ TASK_TURN_RIGHT,				(float)179				},
	{ TASK_PLAY_SEQUENCE,			(float)ACT_IDLE_ANGRY	},
	{ TASK_GET_PATH_TO_LASTPOSITION,(float)0				},
	{ TASK_RUN_PATH,				(float)0				},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0				},
	{ TASK_CLEAR_LASTPOSITION,		(float)0				},
};

Schedule_t	slHumanSearchAndDestroy[] =
{
	{ 
		tlHumanSearchAndDestroy,
		ARRAYSIZE ( tlHumanSearchAndDestroy ), 
		bits_COND_NEW_ENEMY			|
		bits_COND_CAN_ATTACK		|
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"HumanSearchAndDestroy"
	},
};


//=========================================================
// IdleStand - If standing idly, try and say something
//=========================================================

Task_t	tlHumanIdleStand[] =
{
	{ TASK_STOP_MOVING,		0				},
	{ TASK_SET_ACTIVITY,	(float)ACT_IDLE },
	{ TASK_SOUND_IDLE,		(float)0		},// Try to say something
	{ TASK_HUMAN_IDEALYAW,	(float)0		},
	{ TASK_FACE_IDEAL,		(float)0		}, 
	{ TASK_HUMAN_EYECONTACT,(float)0		},
	{ TASK_WAIT,			(float)10		},// wait a bit
};

Schedule_t	slHumanIdleStand[] =
{
	{ 
		tlHumanIdleStand,
		ARRAYSIZE ( tlHumanIdleStand ), 
		bits_COND_NEW_ENEMY			|
		bits_COND_SEE_FEAR			|
		bits_COND_SEE_HATE			|
		bits_COND_SEE_DISLIKE		|
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_PUSHED			|
		bits_COND_PROVOKED			|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_WORLD	|
		bits_SOUND_COMBAT	|
		bits_SOUND_CARCASS	|
		bits_SOUND_MEAT		|
		bits_SOUND_GARBAGE	|
		bits_SOUND_DANGER	|
		bits_SOUND_PLAYER,
		
		"Human Idle Stand"
	},
};


//=========================================================
// IdleResponse - reply to someone else
//=========================================================

Task_t	tlHumanIdleResponse[] =
{
	{ TASK_SET_ACTIVITY,	(float)ACT_IDLE	},// Stop and listen
	{ TASK_WAIT,			(float)7.5		},// Wait until sure it's me they are talking to
	{ TASK_HUMAN_EYECONTACT,(float)0		},// Wait until speaker is done
	{ TASK_HUMAN_SOUND_RESPOND,	(float)0	},// Wait and then say my response
	{ TASK_HUMAN_IDEALYAW,	(float)0		},// look at who I'm talking to
	{ TASK_FACE_IDEAL,		(float)0		}, 
	{ TASK_HUMAN_EYECONTACT,(float)0		},// Wait until speaker is done
};

Schedule_t	slHumanIdleResponse[] =
{
	{ 
		tlHumanIdleResponse,
		ARRAYSIZE ( tlHumanIdleResponse ), 
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"Human Idle Response"

	},
};


//=========================================================
// WaitHeal - Stand still while the doctor jabs me
//=========================================================

Task_t	tlHumanWaitHeal[] =
{
	{ TASK_STOP_MOVING,		(float)0		},
	{ TASK_SET_ACTIVITY,	(float)ACT_IDLE	},// Stop and listen
	{ TASK_WAIT,			(float)1		},// Wait while he stabs me with his fucking great needle
	{ TASK_HUMAN_EYECONTACT,(float)0		},
	{ TASK_HUMAN_SOUND_HEALED,(float)0		},// Wait and then say thanks doc
	{ TASK_HUMAN_IDEALYAW,	(float)0		},// look at who I'm talking to
	{ TASK_FACE_IDEAL,		(float)0		}, 
	{ TASK_HUMAN_EYECONTACT,(float)0		},// Wait until speaker is done
};

Schedule_t	slHumanWaitHeal[] =
{
	{ 
		tlHumanWaitHeal,
		ARRAYSIZE ( tlHumanWaitHeal ), 
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"Human Wait Heal"

	},
};


//=========================================================
// Follow the player or a squad member who I am defending
//=========================================================

Task_t	tlHumanFaceTarget[] =
{
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_FACE_TARGET,			(float)0		},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_SET_SCHEDULE,		(float)SCHED_TARGET_CHASE },
};

Schedule_t	slHumanFaceTarget[] =
{
	{
		tlHumanFaceTarget,
		ARRAYSIZE ( tlHumanFaceTarget ),
		bits_COND_PUSHED		|
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_HEAR_SOUND |
		bits_COND_PROVOKED,
		bits_SOUND_DANGER,
		"HumanFaceTarget"
	},
};


//=========================================================
// Fly through the air
//=========================================================

Task_t tlHumanExplosionDie[] =
{
	{ TASK_STOP_MOVING,			0							},
	{ TASK_HUMAN_SOUND_EXPL,	(float)0					},
	{ TASK_PLAY_SEQUENCE,		(float)ACT_EXPLOSION_HIT	},
	{ TASK_HUMAN_EXPLOSION_FLY,	(float)0					},
	{ TASK_HUMAN_EXPLOSION_LAND,(float)0					},
};

Schedule_t slHumanExplosionDie[] =
{
	{
		tlHumanExplosionDie,
		ARRAYSIZE( tlHumanExplosionDie ),
		0,
		0,
		"HumanExplosionDie"
	},
};


//=========================================================
// Kick (over-rides base Primary Melee Attack to uncrouch)
//=========================================================

Task_t	tlHumanPrimaryMeleeAttack1[] =
{
	{ TASK_STOP_MOVING,			(float)0		},
	{ TASK_HUMAN_UNCROUCH,		(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_MELEE_ATTACK1,		(float)0		},
};

Schedule_t	slHumanPrimaryMeleeAttack[] =
{
	{ 
		tlHumanPrimaryMeleeAttack1,
		ARRAYSIZE ( tlHumanPrimaryMeleeAttack1 ), 
		0,
		0,
		"Human Primary Melee Attack"
	},
};


//=========================================================
// Popup Attack - pop up from behind cover, fire off a few
// rounds, and then duck back down again before the bastards
// know what hit them
//=========================================================

Task_t	tlHumanPopupAttack[] =
{
	{ TASK_STOP_MOVING,			(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},// i.e. I could hit a friendly
	{ TASK_HUMAN_UNCROUCH,		(float)0		},
	{ TASK_HUMAN_CHECK_FIRE,	(float)0		},
//	{ TASK_HUMAN_SOUND_CHARGE,	(float)0.3		},//edit, not all terrorist should call for attack, this attack its a surprise
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_HUMAN_CHECK_FIRE,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_HUMAN_CHECK_FIRE,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_HUMAN_CROUCH,		(float)0		},
	{ TASK_WAIT,				(float)0.5		},//0.5
};

Schedule_t	slHumanPopupAttack[] =
{
	{ 
		tlHumanPopupAttack,
		ARRAYSIZE ( tlHumanPopupAttack ), 
		bits_COND_NEW_ENEMY			|
		bits_COND_ENEMY_DEAD		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_ENEMY_OCCLUDED	|
		bits_COND_HEAR_SOUND		|
		bits_COND_NO_AMMO_LOADED,
		
		bits_SOUND_DANGER,
		"Human Popup Attack"
	},
};


//=========================================================
// repel 
//=========================================================

Task_t	tlHumanRepel[] =
{
	{ TASK_STOP_MOVING,			(float)0			},
	{ TASK_SET_ACTIVITY,		(float)ACT_GLIDE 	},
	{ TASK_FACE_IDEAL,			(float)0			},
	{ TASK_PLAY_SEQUENCE,		(float)ACT_GLIDE 	},
};

Schedule_t	slHumanRepel[] =
{
	{ 
		tlHumanRepel,
		ARRAYSIZE ( tlHumanRepel ), 
		bits_COND_SEE_ENEMY			|
		bits_COND_NEW_ENEMY			|
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER			|
		bits_SOUND_COMBAT			|
		bits_SOUND_PLAYER, 
		"Repel"
	},
};


//=========================================================
// repel 
//=========================================================

Task_t	tlHumanRepelAttack[] =
{
	{ TASK_STOP_MOVING,			(float)0			},
	{ TASK_SET_ACTIVITY,		(float)ACT_GLIDE	},
	{ TASK_FACE_ENEMY,			(float)0			},
	{ TASK_PLAY_SEQUENCE,		(float)ACT_FLY 		},
};

Schedule_t	slHumanRepelAttack[] =
{
	{ 
		tlHumanRepelAttack,
		ARRAYSIZE ( tlHumanRepelAttack ), 
		bits_COND_ENEMY_OCCLUDED,
		0,
		"Repel Attack"
	},
};

//=========================================================
// repel land
//=========================================================
Task_t	tlHumanRepelLand[] =
{
	{ TASK_STOP_MOVING,			(float)0					},
	{ TASK_PLAY_SEQUENCE,		(float)ACT_LAND				},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE				},
	{ TASK_GET_PATH_TO_LASTPOSITION,(float)0				},
	{ TASK_RUN_PATH,				(float)0				},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0				},
	{ TASK_CLEAR_LASTPOSITION,		(float)0				},
	{ TASK_STOP_MOVING,			(float)0					},
};

Schedule_t	slHumanRepelLand[] =
{
	{ 
		tlHumanRepelLand,
		ARRAYSIZE ( tlHumanRepelLand ), 
		bits_COND_SEE_ENEMY			|
		bits_COND_NEW_ENEMY			|
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER			|
		bits_SOUND_COMBAT			|
		bits_SOUND_PLAYER, 
		"Repel Land"
	},
};


//=========================================================
// repel land search and destroy
//=========================================================
Task_t	tlHumanRepelLandSearch[] =
{
	{ TASK_STOP_MOVING,			(float)0					},
	{ TASK_PLAY_SEQUENCE,		(float)ACT_LAND				},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE				},
	{ TASK_PLAY_SEQUENCE,		(float)ACT_IDLE_ANGRY		},	// Do a sweep of the area, giving my squad time to land
	{ TASK_TURN_RIGHT,			(float)179					},
	{ TASK_PLAY_SEQUENCE,		(float)ACT_IDLE_ANGRY		},
	{ TASK_TURN_RIGHT,			(float)179					},
	{ TASK_SET_SCHEDULE,		(float)SCHED_HUMAN_SIGNAL_SEARCH_AND_DESTROY },	// move out
};

Schedule_t	slHumanRepelLandSearch[] =
{
	{ 
		tlHumanRepelLandSearch,
		ARRAYSIZE ( tlHumanRepelLandSearch ), 
		bits_COND_SEE_ENEMY			|
		bits_COND_NEW_ENEMY			|
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER			|
		bits_SOUND_PLAYER, 
		"Repel Land Search and Destroy"
	},
};


//=========================================================
// Fail - over-rides default so human faces his enemy when failed
//=========================================================

Task_t	tlHumanFail[] =
{
	{ TASK_STOP_MOVING,			0							},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE				},
	{ TASK_FACE_IDEAL,			(float)0					},
	{ TASK_WAIT,				(float)0.1					},//2 seconds wait?
	{ TASK_WAIT_PVS,			(float)0					},
};

Schedule_t	slHumanFail[] =
{
	{
		tlHumanFail,
		ARRAYSIZE ( tlHumanFail ),
		bits_COND_CAN_ATTACK,
		0,
		"HumanFail"
	},
};

//=========================================================
// Surrender - play a sentence, drop weapon and clear classify
//=========================================================
Task_t tlHumanSurrender[] =
{
	{ TASK_STOP_MOVING,				0						},
	{ TASK_HUMAN_UNCROUCH,			(float)0				},
	{ TASK_HUMAN_SURRENDER,			(float)0				},
	{ TASK_SET_ACTIVITY,			(float)ACT_IDLE			},
};

Schedule_t slHumanSurrender[] =
{
	{ 
		tlHumanSurrender,
		ARRAYSIZE ( tlHumanSurrender ),
		0,//play it NO MATTER ANYTHING
		0,
		"HumanSurrender"
	}
};

//=========================================================
// FireUp - play a sentence, drop weapon and clear classify
//=========================================================
Task_t tlHumanFireUp[] =
{
	{ TASK_STOP_MOVING,				0						},
	{ TASK_HUMAN_UNCROUCH,			(float)0				},
	{ TASK_HUMAN_ONFIRE,			(float)0				},
	{ TASK_SET_ACTIVITY,			(float)ACT_IDLE			},
};

Schedule_t slHumanFireUp[] =
{
	{ 
		tlHumanFireUp,
		ARRAYSIZE ( tlHumanFireUp ),
		0,//play it NO MATTER ANYTHING
		0,
		"FireUp"
	}
};

//=========================================================
// Draw weapon: face enemy and play a sequence
//=========================================================
Task_t	tlHumanEnemyDraw[] =
{
	{ TASK_STOP_MOVING,					0				},
	{ TASK_FACE_ENEMY,					0				},
	{ TASK_PLAY_SEQUENCE_FACE_ENEMY,	(float) ACT_ARM },
};

Schedule_t slHumanEnemyDraw[] = 
{
	{
		tlHumanEnemyDraw,
		ARRAYSIZE ( tlHumanEnemyDraw ),
		0,
		0,
		"HumanEnemyDraw"
	}
};

//=========================================================
// Side Step right: face enemy, jump right and play a sequence
//=========================================================
Task_t	tlHumanSideStepRight[] =
{
	{ TASK_STOP_MOVING,				0						},
	{ TASK_HUMAN_UNCROUCH,			(float)0				},
	{ TASK_FACE_ENEMY,				(float)0				},
//	{ TASK_FIND_COVER_FROM_ENEMY,	(float)0							},//strafe test

	{ TASK_HUMAN_SIDESTEP_RIGHT,	(float)0				},
};

Schedule_t slHumanSideStepRight[] = 
{
	{
		tlHumanSideStepRight,
		ARRAYSIZE ( tlHumanSideStepRight ),
		0,
		0,
		"HumanSideStepRight"
	}
};

//=========================================================
// Side Step right: face enemy, jump left and play a sequence
//=========================================================
Task_t	tlHumanSideStepLeft[] =
{
	{ TASK_STOP_MOVING,				0						},
	{ TASK_HUMAN_UNCROUCH,			(float)0				},
	{ TASK_FACE_ENEMY,				(float)0				},
//	{ TASK_FIND_COVER_FROM_ENEMY,	(float)0							},//strafe test

	{ TASK_HUMAN_SIDESTEP_LEFT,		(float)0				},
};

Schedule_t slHumanSideStepLeft[] = 
{
	{
		tlHumanSideStepLeft,
		ARRAYSIZE ( tlHumanSideStepLeft ),
		0,
		0,
		"HumanSideStepLeft"
	}
};

//=========================================================
// Drop Weapon
//=========================================================
Task_t tlHumanDropWeapon[] =
{
	{ TASK_STOP_MOVING,				0						},
	{ TASK_HUMAN_UNCROUCH,			(float)0				},
	{ TASK_HUMAN_DROP_WEAPON,		(float)0				},
	{ TASK_SET_ACTIVITY,			(float)ACT_DROP_GUN		},
};

Schedule_t slHumanDropWeapon[] =
{
	{ 
		tlHumanDropWeapon,
		ARRAYSIZE ( tlHumanDropWeapon ),
		0,//play it NO MATTER ANYTHING
		0,
		"HumanDropWeapon"
	}
};

//=========================================================
// Kamikaze - stand, play a sentence and a animation (?)
//=========================================================
//EDIT, using another schedule
Task_t tlHumanGoToKamikaze[] =
{
	{ TASK_STOP_MOVING,				0						},
	{ TASK_HUMAN_UNCROUCH,			(float)0				},
//	{ TASK_HUMAN_KAMIKAZE,			(float)0				},
	{ TASK_SET_ACTIVITY,			(float)ACT_IDLE			},
};

Schedule_t slHumanGoToKamikaze[] =
{
	{ 
		tlHumanGoToKamikaze,
		ARRAYSIZE ( tlHumanGoToKamikaze ),
		0,//play it NO MATTER ANYTHING
		0,
		"HumanGoToKamikaze"
	}
};


//=========================================================
// Get path to markers
//=========================================================
Task_t	tlHumanGoToMarker[] =
{
	{ TASK_STOP_MOVING,						(float)0					},
	{ TASK_FACE_IDEAL,						(float)0					},
	{ TASK_GET_PATH_TO_AI_MARKER,			(float)0					},
	{ TASK_WALK_PATH,						(float)0					},//walk

	{ TASK_WAIT_FOR_MOVEMENT,				(float)0.1					},
	{ TASK_PLAY_SEQUENCE,					(float)ACT_SIGNAL1			},

	{ TASK_WAIT,							(float)0.1					},
	{ TASK_SET_SCHEDULE,					(float)SCHED_TARGET_FACE	},
};

Schedule_t	slHumanGoToMarker[] =
{
	{ 
		tlHumanGoToMarker,
		ARRAYSIZE ( tlHumanGoToMarker ), 
		bits_COND_NEW_ENEMY			|
		bits_COND_SEE_FEAR			|
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		/*
		bits_COND_NEW_ENEMY		|//this must be overriden
		bits_COND_SEE_ENEMY			|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE,
		0,
		*/
		"HumanGoToMarker"
	},
};

//=========================================================
// Try to pick up a item
//=========================================================
Task_t	tlHumanGoToItem[] =
{
	{ TASK_STOP_MOVING,						(float)0					},
	{ TASK_GET_PATH_TO_ITEM,				(float)0					},
	{ TASK_FACE_IDEAL,						(float)0					},
	{ TASK_RUN_PATH,						(float)0					},//walk???????
	{ TASK_WAIT_FOR_MOVEMENT,				(float)0.1					},
	{ TASK_PLAY_SEQUENCE,					(float)ACT_PICK_ITEM		},
//	{ TASK_REMOVE_ITEM,						(float)0					},
};

Schedule_t	slHumanGoToItem[] =
{
	{ 
		tlHumanGoToItem,
		ARRAYSIZE ( tlHumanGoToItem ), 
		bits_COND_NEW_ENEMY		|//this must be overriden
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE,
		0,
		"HumanGoToItem"
	},
};


//=========================================================
// Get path to hotspots
//=========================================================
Task_t	tlHumanGoToHotSpot[] =
{
/*	{ TASK_STOP_MOVING,				(float)0				},
	{ TASK_STORE_LASTPOSITION,		(float)0				},
	{ TASK_GET_PATH_TO_AI_HOTSPOT,			(float)0					},

	{ TASK_FACE_IDEAL,				(float)0				},
	{ TASK_WALK_PATH,				(float)0				},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0				},
	{ TASK_PLAY_SEQUENCE,					(float)ACT_SIGNAL2			},
	{ TASK_WAIT,					(float)10				},
	{ TASK_GET_PATH_TO_LASTPOSITION,(float)0				},
	{ TASK_WALK_PATH,				(float)0				},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0				},
	{ TASK_CLEAR_LASTPOSITION,		(float)0				},
	{ TASK_TURN_LEFT,			(float)179	},*/


	{ TASK_STOP_MOVING,						(float)0					},
	{ TASK_GET_PATH_TO_AI_HOTSPOT,			(float)0					},
	
	{ TASK_FACE_IDEAL,						(float)0					},
	{ TASK_WALK_PATH,						(float)0					},//walk

	{ TASK_WAIT_FOR_MOVEMENT,				(float)0.1					},
	{ TASK_PLAY_SEQUENCE,					(float)ACT_SIGNAL2			},

	{ TASK_WAIT,							(float)0.1					},
	{ TASK_SET_SCHEDULE,					(float)SCHED_TARGET_FACE	},
};

Schedule_t	slHumanGoToHotSpot[] =
{
	{ 
		tlHumanGoToHotSpot,
		ARRAYSIZE ( tlHumanGoToHotSpot ), 
		bits_COND_NEW_ENEMY		|//this must be overriden
		bits_COND_SEE_ENEMY			|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE,
		0,
		"HumanGoToHotSpot"
	},
};

//=========================================================
// Wakeup -
//=========================================================
Task_t tlHumanWakeUp[] =
{
	{ TASK_STOP_MOVING,				0						},
//	{ TASK_HUMAN_UNCROUCH,			(float)0				},
	{ TASK_HUMAN_WAKEUP,			(float)0				},
	{ TASK_SET_ACTIVITY,			(float)ACT_IDLE			},
};

Schedule_t slHumanWakeUp[] =
{
	{ 
		tlHumanWakeUp,
		ARRAYSIZE ( tlHumanWakeUp ),
		0,//WakeUp NO MATTER ANYTHING
		0,
		"WakeUp"
	}
};

DEFINE_CUSTOM_SCHEDULES( CBaseHuman )
{
	slHumanTakeCover,
	slHumanGrenadeCover,
	slHumanTossGrenadeCover,
	slHumanTakeCoverFromBestSound,
	slHumanCower,
	slHumanHearSound,
	slHumanInvestigateSound,
	slHumanVictoryDance,
	slHumanHideReload,
	slHumanReload,
	slHumanFindMedic,
	slHumanFindMedicCombat,
	slHumanSignalAttack,
	slHumanSignalSurpress,
	slHumanSignalSearchAndDestroy,
	slHumanSignalRetreat,
	slHumanSignalComeToMe,
	slHumanSignalCheckIn,
	slHumanCheckIn,
	slHumanFoundEnemy,
	slHumanRangeAttack1,
	slHumanRangeAttack2,
	slHumanEstablishLineOfFire,
	slHumanExplosiveELOF,
	slHumanMeleeELOF,
	slHumanStandoff,
	slHumanUnCrouch,
	slHumanUnCrouchScript,
	slHumanWaitInCover,
	slHumanFollow,
	slHumanFollowClose,
	slHumanTurnRound,
	slHumanSurpress,
	slHumanMoveToEnemyLKP,
	slHumanSearchAndDestroy,
	slHumanIdleStand,
	slHumanIdleResponse,
	slHumanFaceTarget,
	slHumanExplosionDie,
	slHumanPrimaryMeleeAttack,
	slHumanWaitHeal,
	slHumanPopupAttack,
	slHumanRepel,
	slHumanRepelAttack,
	slHumanRepelLand,
	slHumanRepelLandSearch,
	slHumanFail,
	slHumanRetreat,//fix
	slHumanSurrender,//new
	slHumanFireUp,//new
	slHumanEnemyDraw,//new
	slHumanSideStepRight,//new
	slHumanSideStepLeft,//new
	slHumanDropWeapon,
	slHumanGoToKamikaze,
	slHumanGoToMarker,
	slHumanGoToItem,
	slHumanGoToHotSpot,
	slHumanWakeUp,
	slHumanRangeSniperAttack1
};

IMPLEMENT_CUSTOM_SCHEDULES( CBaseHuman, CSquadMonster );

//=========================================================
// AI Schedules Specific to this monster
//=========================================================

Schedule_t* CBaseHuman :: GetScheduleOfType ( int Type )
{
	switch( Type )
	{
	case SCHED_HUMAN_COVER_AND_RELOAD:
		{
			return &slHumanHideReload[ 0 ];
		}
		break;

	case SCHED_HUMAN_RELOAD:
		{
			return &slHumanReload[ 0 ];
		}
		break;

	case SCHED_TAKE_COVER_FROM_BEST_SOUND:
		{
			return &slHumanTakeCoverFromBestSound[ 0 ];
		}
		break;

	case SCHED_HUMAN_TAKECOVER_FAILED:
		{
			Remember( bits_MEMORY_HUMAN_NO_COVER );
			return GetScheduleOfType ( SCHED_FAIL );
		}
		break;

	case SCHED_COWER:
		{
			return &slHumanCower[ 0 ];
		}
		break;

	case SCHED_HUMAN_HEAR_SOUND:
		{
			return &slHumanHearSound[ 0 ];
		}
		break;

	case SCHED_INVESTIGATE_SOUND:
		{
			return &slHumanInvestigateSound[ 0 ];
		}
		break;

	case SCHED_VICTORY_DANCE:
		{
			return &slHumanVictoryDance[ 0 ];
		}
		break;

	case SCHED_HUMAN_FIND_MEDIC:
		{
			return &slHumanFindMedic[ 0 ];
		}
		break;

	case SCHED_HUMAN_FIND_MEDIC_COMBAT:
		{
			return &slHumanFindMedicCombat[ 0 ];
		}
		break;

	case SCHED_HUMAN_SIGNAL_ATTACK:
		{
			return &slHumanSignalAttack[ 0 ];
		}
		break;

	case SCHED_HUMAN_SIGNAL_SURPRESS:
		{
			return &slHumanSignalSurpress[ 0 ];
		}
		break;

	case SCHED_HUMAN_SIGNAL_SEARCH_AND_DESTROY:
		{
			return &slHumanSignalSearchAndDestroy[ 0 ];
		}
		break;

	case SCHED_HUMAN_SIGNAL_RETREAT:
		{
			return &slHumanSignalRetreat[ 0 ];
		}
		break;

	case SCHED_HUMAN_SIGNAL_COME_TO_ME:
		{
			return &slHumanSignalComeToMe[ 0 ];
		}
		break;

	case SCHED_HUMAN_SIGNAL_CHECK_IN:
		{
			return &slHumanSignalCheckIn[ 0 ];
		}
		break;

	case SCHED_HUMAN_CHECK_IN:
		{
			return &slHumanCheckIn[ 0 ];
		}
		break;

	case SCHED_HUMAN_FOUND_ENEMY:
		{
			return &slHumanFoundEnemy[ 0 ];
		}
		break;

	case SCHED_RANGE_ATTACK1:
		{
			if ( m_fStopCrouching )
			{
				return GetScheduleOfType( SCHED_HUMAN_POPUP_ATTACK );
			}
			else
			{
				if ( pev->weapons == HUMAN_WEAPON_AWP )
				{
				//	if (RANDOM_LONG( 0, 99 ) < 80)
					return &slHumanRangeSniperAttack1[ 0 ];
				//	else
				//	return &slHumanRangeAttack1[ 0 ];
				}
				else
				return &slHumanRangeAttack1[ 0 ];
			}
		}
		break;

	case SCHED_RANGE_ATTACK2:
		{
			return &slHumanRangeAttack2[ 0 ];
		}
		break;

	case SCHED_HUMAN_ESTABLISH_LINE_OF_FIRE:
		{

			switch ( pev->weapons )
			{
				
			case HUMAN_WEAPON_LAW:
				return &slHumanExplosiveELOF[ 0 ];
				break;

			default:
				return &slHumanEstablishLineOfFire[ 0 ];
				break;

			}
		}
		break;

	case SCHED_HUMAN_MELEE_ELOF:
		{
			return &slHumanMeleeELOF[ 0 ];
		}
		break;

	case SCHED_HUMAN_ELOF_FAIL:
		{
			// human is unable to move to a position that allows him to attack the enemy.
			return GetScheduleOfType ( SCHED_STANDOFF );
		}
		break;

	case SCHED_STANDOFF:
		{
			return &slHumanStandoff[ 0 ];
		}
		break;

	case SCHED_HUMAN_UNCROUCH:
		{
			return &slHumanUnCrouch[ 0 ];
		}
		break;

	case SCHED_HUMAN_DROP_WEAPON:
		{
			return &slHumanDropWeapon[ 0 ];
		}
		break;

	case SCHED_HUMAN_UNCROUCH_SCRIPT:
		{
			return &slHumanUnCrouchScript[ 0 ];
		}
		break;

	case SCHED_HUMAN_WAIT_FACE_ENEMY:
		{
			return &slHumanWaitInCover[ 0 ];
		}
		break;

	case SCHED_TARGET_CHASE:
	case SCHED_HUMAN_FOLLOW:
		{
			return &slHumanFollow[ 0 ];
		}
		break;

	case SCHED_HUMAN_FOLLOW_CLOSE:
		{
			return &slHumanFollowClose[ 0 ];
		}
		break;

	case SCHED_TAKE_COVER_FROM_ENEMY:
		{
			if ( m_fHandGrenades )
			{
				if ( RANDOM_LONG(0,1) && HasConditions( bits_COND_CAN_RANGE_ATTACK2 ) && 
					!HasConditions( bits_COND_HEAVY_DAMAGE | bits_COND_LIGHT_DAMAGE ) && 
					OccupySlot( bits_SLOTS_HUMAN_GRENADE ) )
				{
					return &slHumanTossGrenadeCover[ 0 ];
				}

				if ( RANDOM_LONG(0,1) && !( InSquad() && SquadMemberInRange( pev->origin, 256 ) ) )
				{
					return &slHumanGrenadeCover[ 0 ];
				}
			}
			else
			{
				return &slHumanTakeCover[ 0 ];
			}
		}
		break;

	case SCHED_HUMAN_RETREAT:
		{
			return &slHumanRetreat[ 0 ];
		}
		break;

	case SCHED_HUMAN_TAKE_COVER_FROM_ENEMY_NO_GRENADE:
		{
			return &slHumanTakeCover[ 0 ];
		}
		break;

	case SCHED_HUMAN_TURN_ROUND:
		{
			return &slHumanTurnRound[ 0 ];
		}
		break;

	case SCHED_HUMAN_SURPRESS:
		{
			if ( m_fStopCrouching )
			{
				return GetScheduleOfType( SCHED_HUMAN_POPUP_ATTACK );
			}
			return &slHumanSurpress[ 0 ];
		}
		break;

	case SCHED_HUMAN_MOVE_TO_ENEMY_LKP:
		{
			return &slHumanMoveToEnemyLKP[ 0 ];
		}
		break;

	case SCHED_HUMAN_SEARCH_AND_DESTROY:
		{
			return &slHumanSearchAndDestroy[ 0 ];
		}
		break;

	case SCHED_IDLE_STAND:
		{
			return &slHumanIdleStand[ 0 ];
		}
		break;

	case SCHED_HUMAN_IDLE_RESPONSE:
		{
			return &slHumanIdleResponse[ 0 ];
		}
		break;

	case SCHED_TARGET_FACE:
		{
			return &slHumanFaceTarget[ 0 ];
		}
		break;

	case SCHED_HUMAN_EXPLOSION_DIE:
		{
			return &slHumanExplosionDie[ 0 ];
		}
		break;

	case SCHED_MELEE_ATTACK1:
		{
			return &slHumanPrimaryMeleeAttack[ 0 ];
		}
		break;

	case SCHED_HUMAN_WAIT_HEAL:
		{
			return &slHumanWaitHeal[ 0 ];
		}
		break;

	case SCHED_HUMAN_POPUP_ATTACK:
		{
			return &slHumanPopupAttack[ 0 ];
		}
		break;

	case SCHED_HUMAN_REPEL:
		{
			if (pev->velocity.z > -128)
				pev->velocity.z -= 32;
			return &slHumanRepel[ 0 ];
		}
		break;

	case SCHED_HUMAN_REPEL_ATTACK:
		{
			if (pev->velocity.z > -128)
				pev->velocity.z -= 32;
			return &slHumanRepelAttack[ 0 ];
		}
		break;

	case SCHED_HUMAN_REPEL_LAND:
		{
			m_flCrouchTime = gpGlobals->time + CROUCH_TIME;
		//	pev->view_ofs = Vector( 0, 0, 36 );
			m_fCrouching = TRUE;
			UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_DUCK);

			return &slHumanRepelLand[ 0 ];
		}
		break;

	case SCHED_HUMAN_REPEL_LAND_SEARCH_AND_DESTROY:
		{
			m_flCrouchTime = gpGlobals->time + CROUCH_TIME;
		//	pev->view_ofs = Vector( 0, 0, 36 );
			m_fCrouching = TRUE;
			UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_DUCK);

			return &slHumanRepelLandSearch[ 0 ];
		}
		break;

	case SCHED_FAIL:
		{
			return &slHumanFail[ 0 ];
		}
		break;

	//new
	case SCHED_HUMAN_SURRENDER:
		{
			return &slHumanSurrender[ 0 ];
		}
		break;

	case SCHED_HUMAN_FIREUP:
		{
			return &slHumanFireUp[ 0 ];
		}
		break;

	case SCHED_HUMAN_WAKEUP:
		{
			return &slHumanWakeUp[ 0 ];
		}
		break;

	case SCHED_HUMAN_DRAW_WEAPON:
		if ( m_hEnemy != NULL )
		{
			return &slHumanEnemyDraw[ 0 ];
		}
		break;

	case SCHED_HUMAN_SIDESTEP_RIGHT:
		if ( m_hEnemy != NULL )
		{
			return &slHumanSideStepRight[ 0 ];
		}
		break;

	case SCHED_HUMAN_SIDESTEP_LEFT:
		if ( m_hEnemy != NULL )
		{
			return &slHumanSideStepLeft[ 0 ];
		}
		break;

	case SCHED_AISCRIPT:
		{
			if ( m_fCrouching )
			{
				return GetScheduleOfType( SCHED_HUMAN_UNCROUCH_SCRIPT );
			}
			else
			{
				return CSquadMonster::GetScheduleOfType( Type );
			}
		}
		break;

	case SCHED_GO_TO_AI_MARKER:
		{
			iTries++;
			ALERT( at_console, "INFO: Trying to go to position.\n");

			return &slHumanGoToMarker[ 0 ];
		}

	case SCHED_GO_TO_ITEM:
		{
			ALERT( at_console, "INFO: Trying to pick up a item\n");

			return &slHumanGoToItem[ 0 ];
		}
	
	case SCHED_GO_TO_AI_HOTSPOT:
		{
			ALERT( at_console, "INFO: Trying to go to position.\n");

			return &slHumanGoToHotSpot[ 0 ];
		}
	}

	return CSquadMonster::GetScheduleOfType( Type );
}

//=========================================================
// PickUpGun
//=========================================================

void CBaseHuman :: PickUpGun( int gun )
{
	pev->weapons = gun;
//	SetBodygroup( HUMAN_BODYGROUP_BODY, 0 );
	EMIT_SOUND( ENT(pev), CHAN_ITEM, "items/gunpickup1.wav", 1, ATTN_NORM );

	m_cAmmoLoaded = m_cClipSize;
	m_afCapability |= bits_CAP_RANGE_ATTACK1;
}
/*
bool bCreated = false;*/
int iHotSpots = 0;

void CBaseHuman :: CreateMarker(void)
{
	if (iHotSpots >= 1)//20? :A
		return;

	TraceResult tr;

	UTIL_MakeVectors( pev->angles );

	Vector vecSrc = EyePosition();
	Vector vecEndPos = vecSrc + (gpGlobals->v_forward * 2000);//4444

	UTIL_TraceLine( vecSrc, vecEndPos, dont_ignore_monsters, edict(), &tr);

	if (tr.flFraction != 1.0)
	{
		CBaseEntity *pEntity = CBaseEntity::Instance(tr.pHit);

		if ( pEntity->IsBSPModel() )
		{
		//	CBaseEntity *pPreviousMarker;

		//	pPreviousMarker = UTIL_FindEntityByClassname(NULL, "ai_hotspot");

		//	if(!bCreated)
			{
				ALERT ( at_console, "#AI_HotSpotCreated\n" );
				CBaseEntity *pAIMarker = CBaseEntity::Create( "ai_hotspot", tr.vecEndPos + (gpGlobals->v_forward * -50), g_vecZero, pEntity->edict() );
			
			//	bCreated = true;
				++iHotSpots;
			}
		/*	else
			{
				if(pPreviousMarker)
				{
					ALERT ( at_console, "#AI_RemovingPreviousHotSpot\n" );
					UTIL_Remove( pPreviousMarker );
					pPreviousMarker = NULL;
								
					bCreated = false;
					--iHotSpots;
				}
			}*/
		}
		else
		{
			ALERT ( at_console, "#AI_CantCreateHotSpotOnPos\n" );

			//ClientPrint(pev, HUD_PRINTCENTER, "#AI_CantCreateMarkerOnPos");
		}
	}
}




//=========================================================
// MonsterThink, overridden for roaches.
//=========================================================
void CBaseHuman :: MonsterThink( void  )
{
	if ( pev->movetype == MOVETYPE_FLY && pev->velocity.z >= 0 ) 
	{
		pev->velocity.z -= 10;
	}
	
	CBaseMonster::MonsterThink();
}

//=========================================================
// Picks a new spot for roach to run to.(
//=========================================================
void CBaseHuman :: PickNewDest ( int iCondition )
{
	Vector	vecNewDir;
	Vector	vecDest;
	float	flDist;

	m_iMode = iCondition;

	if ( m_iMode == ROACH_SMELL_FOOD )
	{
		// find the food and go there.
		CSound *pSound;

		pSound = CSoundEnt::SoundPointerForIndex( m_iAudibleList );

		if ( pSound )
		{
			m_Route[ 0 ].vecLocation.x = pSound->m_vecOrigin.x + ( 3 - RANDOM_LONG(0,5) );
			m_Route[ 0 ].vecLocation.y = pSound->m_vecOrigin.y + ( 3 - RANDOM_LONG(0,5) );
			m_Route[ 0 ].vecLocation.z = pSound->m_vecOrigin.z;
			m_Route[ 0 ].iType = bits_MF_TO_LOCATION;
			m_movementGoal = RouteClassify( m_Route[ 0 ].iType );
			return;
		}
	}

	do 
	{
		// picks a random spot, requiring that it be at least 128 units away
		// else, the roach will pick a spot too close to itself and run in 
		// circles. this is a hack but buys me time to work on the real monsters.
		vecNewDir.x = RANDOM_FLOAT( -1, 1 );
		vecNewDir.y = RANDOM_FLOAT( -1, 1 );
		flDist		= 256 + ( RANDOM_LONG(0,255) );
		vecDest = pev->origin + vecNewDir * flDist;

	} while ( ( vecDest - pev->origin ).Length2D() < 128 );

	m_Route[ 0 ].vecLocation.x = vecDest.x;
	m_Route[ 0 ].vecLocation.y = vecDest.y;
	m_Route[ 0 ].vecLocation.z = pev->origin.z;
	m_Route[ 0 ].iType = bits_MF_TO_LOCATION;
	m_movementGoal = RouteClassify( m_Route[ 0 ].iType );

	if ( RANDOM_LONG(0,9) == 1 )
	{
		// every once in a while, a roach will play a skitter sound when they decide to run
		EMIT_SOUND_DYN(ENT(pev), CHAN_BODY, "roach/rch_walk.wav", 1, ATTN_NORM, 0, 80 + RANDOM_LONG(0,39) );
	}
}

//=========================================================
// roach's move function
//=========================================================
void CBaseHuman :: Move ( float flInterval ) 
{
	/*
	if (m_hEnemy != NULL)
	{

	}
	else//no enemy
	{
		if(IsFollowingPlayer())//im followin' client
		{
			CBaseMonster::Move( flInterval );
		}
		else
		{
			float		flWaypointDist;
			Vector		vecApex;

			// local move to waypoint.
			flWaypointDist = ( m_Route[ m_iRouteIndex ].vecLocation - pev->origin ).Length2D();
			MakeIdealYaw ( m_Route[ m_iRouteIndex ].vecLocation );

			ChangeYaw ( pev->yaw_speed );
			UTIL_MakeVectors( pev->angles );

			if ( RANDOM_LONG(0,7) == 1 )
			{
				// randomly check for blocked path.(more random load balancing)
				if ( !WALK_MOVE( ENT(pev), pev->ideal_yaw, 4, WALKMOVE_NORMAL ) )
				{
					// stuck, so just pick a new spot to run off to
					PickNewDest( m_iMode );
				}
			}
			
			WALK_MOVE( ENT(pev), pev->ideal_yaw, m_flGroundSpeed * flInterval, WALKMOVE_NORMAL );

			// if the waypoint is closer than step size, then stop after next step (ok for roach to overshoot)
			if ( flWaypointDist <= m_flGroundSpeed * flInterval )
			{
				// take truncated step and stop

				SetActivity ( ACT_IDLE );
				m_flLastLightLevel = GETENTITYILLUM( ENT ( pev ) );// this is roach's new comfortable light level

				if ( m_iMode == ROACH_SMELL_FOOD )
				{
					m_iMode = ROACH_EAT;
				}
				else
				{
					m_iMode = ROACH_IDLE;
				}
			}

			if ( RANDOM_LONG(0,149) == 1 && m_iMode != ROACH_SCARED_BY_LIGHT && m_iMode != ROACH_SMELL_FOOD )
			{
				// random skitter while moving as long as not on a b-line to get out of light or going to food
				PickNewDest( FALSE );
			}
		}
	}
	*/
	CBaseMonster::Move( flInterval );
}

BOOL CBaseHuman::IsFacing( entvars_t *pevTest, const Vector &reference )
{
	Vector vecDir = (reference - pevTest->origin);
	vecDir.z = 0;
	vecDir = vecDir.Normalize();
	Vector forward, angle;
	angle = pevTest->v_angle;
	angle.x = 0;
	UTIL_MakeVectorsPrivate( angle, forward, NULL, NULL );
	// He's facing me, he meant it
	if ( DotProduct( forward, vecDir ) > 0.96 )	// +/- 15 degrees or so
	{
		return TRUE;
	}
	return FALSE;
}

void CBaseHuman::DetectEnviroment()
{
	if (bTerrorist)
	{
		if(m_hEnemy == NULL)
			return;

		CBaseEntity *pPlayer = CBaseEntity::Instance(m_hEnemy);

		if (pPlayer->b_PlayerIsNearOf == DOOR)
		{
			PlayLabelledSentence( "DOOR" );
		}
		else if (pPlayer->b_PlayerIsNearOf == LADDER)
		{
			PlayLabelledSentence( "LADDER" );
		}
		else if (pPlayer->b_PlayerIsNearOf == BREKABLE)	
		{
			PlayLabelledSentence( "BOX" );
		}
		else if (pPlayer->b_PlayerIsNearOf == TRAIN)	
		{
			PlayLabelledSentence( "ELEV" );
		}
		else if (pPlayer->b_PlayerIsNearOf == PLAT)	
		{
			PlayLabelledSentence( "ELEV" );
		}
	}
}



//Check where the monster is and report sound
//USED FOR ALLIES TO SAY WHERE THE BADASS IS AT
void CBaseHuman::ReportEnemyPosition( void )
{
//	CBaseEntity *pPlayer = UTIL_PlayerByIndex( 1 );
	CBaseEntity *pPlayer = CBaseEntity::Instance( FIND_ENTITY_BY_CLASSNAME( NULL, "player" ) );

	if (m_hEnemy != NULL)//if we have a enemy (MONSTERSTATE_COMBAT/ALERT)
	{		
		Talk( 2 );
		m_hTalkTarget = m_hTargetEnt;
	}

	return;
}

//Check monster's type and report sound
void CBaseHuman::ReportEnemyType( void )
{
//	CBaseEntity *pPlayer = UTIL_PlayerByIndex( 1 );
	CBaseEntity *pPlayer = CBaseEntity::Instance( FIND_ENTITY_BY_CLASSNAME( NULL, "player" ) );

	if ( m_hEnemy != NULL )
	{		
		if ( FClassnameIs(m_hEnemy->pev, "monster_terrorist") || FClassnameIs(m_hEnemy->pev, "monster_human_ak"))
		{
			switch ( pev->weapons )
			{
				case HUMAN_WEAPON_MP5:			ClientPrint(pPlayer->pev, HUD_PRINTTALK, "AG Member(RADIO): Suspect has MP5 Navy" ); break;
				case HUMAN_WEAPON_SHOTGUN:		ClientPrint(pPlayer->pev, HUD_PRINTTALK, "AG Member(RADIO): Watch out, Suspect has a Shotgun" ); break;
				case HUMAN_WEAPON_LAW:			ClientPrint(pPlayer->pev, HUD_PRINTTALK, "AG Member(RADIO): HOLY SHIT! RUN! Suspect has a Launcer!" ); break;
				case HUMAN_WEAPON_AWP:			ClientPrint(pPlayer->pev, HUD_PRINTTALK, "AG Member(RADIO): Go cover! Sniper ahead!" ); break;
				case HUMAN_WEAPON_ASSAULT:		ClientPrint(pPlayer->pev, HUD_PRINTTALK, "AG Member(RADIO): Careful, Suspect has a AK47" ); break;
				case HUMAN_WEAPON_PISTOL:		ClientPrint(pPlayer->pev, HUD_PRINTTALK, "AG Member(RADIO): Suspect has a Glock" ); break;
				case HUMAN_WEAPON_DESERT:		ClientPrint(pPlayer->pev, HUD_PRINTTALK, "AG Member(RADIO): Suspect has a Desert Eagle" ); break;
				case HUMAN_WEAPON_M249:			ClientPrint(pPlayer->pev, HUD_PRINTTALK, "AG Member(RADIO): Cover! Suspect has a M249" ); break;
				case HUMAN_WEAPON_UZI:			ClientPrint(pPlayer->pev, HUD_PRINTTALK, "AG Member(RADIO): Suspect has a P90" ); break;
				case HUMAN_WEAPON_USAS:			ClientPrint(pPlayer->pev, HUD_PRINTTALK, "AG Member(RADIO): Suspect has a USAS" ); break;
			}
			/*
			if (m_hEnemy->pev->frags == MP5)
			{
				PlaySentence( "FG_WEP_MP", 2, VOL_NORM, ATTN_IDLE );
				ClientPrint(pPlayer->pev, HUD_PRINTTALK, "AG Member(RADIO): Suspect has MP5 Navy" );
			}
			else if (m_hEnemy->pev->frags == SHOTGUN)
			{
				PlaySentence( "FG_WEP_SHOTGUN", 2, VOL_NORM, ATTN_IDLE );
				ClientPrint(pPlayer->pev, HUD_PRINTTALK, "AG Member(RADIO): Watch out, Suspect has a Shotgun" );
			}
			else if (m_hEnemy->pev->frags == LAW)
			{
				PlaySentence( "FG_WEP_", 2, VOL_NORM, ATTN_IDLE );
				ClientPrint(pPlayer->pev, HUD_PRINTTALK, "AG Member(RADIO): HOLY SHIT! RUN! Suspect has a Launcer!" );
			}
			else if (m_hEnemy->pev->frags == AWP)
			{
				PlaySentence( "FG_WEP_SNIPER", 2, VOL_NORM, ATTN_IDLE );
				ClientPrint(pPlayer->pev, HUD_PRINTTALK, "AG Member(RADIO): Go cover! Sniper ahead!" );
			}
			else if (m_hEnemy->pev->frags == ASSAULT)
			{
				PlaySentence( "FG_WEP_AK", 2, VOL_NORM, ATTN_IDLE );
				ClientPrint(pPlayer->pev, HUD_PRINTTALK, "AG Member(RADIO): Careful, Suspect has a AK47" );
			}
			else if (m_hEnemy->pev->frags == PISTOL)
			{
				PlaySentence( "FG_WEP_PISTOL", 2, VOL_NORM, ATTN_IDLE );
				ClientPrint(pPlayer->pev, HUD_PRINTTALK, "AG Member(RADIO): Suspect has a Glock" );
			}
			else if (m_hEnemy->pev->frags == DESERT)
			{
				PlaySentence( "FG_WEP_PISTOL", 2, VOL_NORM, ATTN_IDLE );
				ClientPrint(pPlayer->pev, HUD_PRINTTALK, "AG Member(RADIO): Suspect has a Desert Eagle" );
			}
			else if (m_hEnemy->pev->frags == M249)
			{
				PlaySentence( "FG_WEP_M249", 2, VOL_NORM, ATTN_IDLE );
				ClientPrint(pPlayer->pev, HUD_PRINTTALK, "AG Member(RADIO): Cover! Suspect has a M249" );
			}
			else if (m_hEnemy->pev->frags == 9)
			{
				PlaySentence( "FG_WEP_KMKZ", 2, VOL_NORM, ATTN_IDLE );
				ClientPrint(pPlayer->pev, HUD_PRINTTALK, "AG Member(RADIO): Kamikaze! SHOOT IT DOWN!" );
			}
			else
				ALERT(at_console, "TERRORISTA!\n");
			*/			
			
		}
	}
	
	return;
}

#define NO_RESISTENCE				0
#define LOW_RESISTENCE				50
#define MED_RESISTENCE				100
#define HIGH_RESISTENCE				250
#define SUPER_RESISTENCE			500

void CBaseHuman::ReportEnemyFrustration( void )
{
//	CBaseEntity *pPlayer = UTIL_PlayerByIndex( 1 );
	CBaseEntity *pPlayer = CBaseEntity::Instance( FIND_ENTITY_BY_CLASSNAME( NULL, "player" ) );

	if ( m_hEnemy != NULL )
	{		
		if ( FClassnameIs(m_hEnemy->pev, "monster_terrorist") || FClassnameIs(m_hEnemy->pev, "monster_human_ak"))
		{
			if (m_hEnemy->m_iBreakPoint == LOW_RESISTENCE)
			{
//				PlaySentence( "FG_RES_LOW", 2, VOL_NORM, ATTN_IDLE );
				ClientPrint(pPlayer->pev, HUD_PRINTTALK, "AG Member(RADIO): Suspect has low resistence" );
			}
			else if (m_hEnemy->m_iBreakPoint == MED_RESISTENCE)
			{
//				PlaySentence( "FG_RES_MED", 2, VOL_NORM, ATTN_IDLE );
				ClientPrint(pPlayer->pev, HUD_PRINTTALK, "AG Member(RADIO): Suspect resists" );
			}
			else if (m_hEnemy->m_iBreakPoint == HIGH_RESISTENCE)
			{
//				PlaySentence( "FG_RES_HIGH", 2, VOL_NORM, ATTN_IDLE );
				ClientPrint(pPlayer->pev, HUD_PRINTTALK, "AG Member(RADIO): Suspect has low high resistence" );
			}
			else if (m_hEnemy->m_iBreakPoint == SUPER_RESISTENCE)
			{
//				PlaySentence( "FG_RES_SUPER", 2, VOL_NORM, ATTN_IDLE );
				ClientPrint(pPlayer->pev, HUD_PRINTTALK, "AG Member(RADIO): Shit, he's crazy, he won't surrender" );
			}
			else
			{
//				PlaySentence( "FG_RES_NONE", 2, VOL_NORM, ATTN_IDLE );
				ClientPrint(pPlayer->pev, HUD_PRINTTALK, "AG Member(RADIO): Easy now, don't shoot'em" );
			}
		}
	}

	return;
}


//=========================================================
//  FValidateHintType 
//=========================================================
BOOL CBaseHuman :: FValidateHintType ( short sHint )
{
	int i;

	static short sHoundHints[] =
	{
		HINT_WORLD_MACHINERY,
		HINT_WORLD_BLINKING_LIGHT,
		HINT_WORLD_HUMAN_BLOOD,
		HINT_WORLD_ALIEN_BLOOD,
	};

	for ( i = 0 ; i < ARRAYSIZE ( sHoundHints ) ; i++ )
	{
		if ( sHoundHints[ i ] == sHint )
		{
			return TRUE;
		}
	}

	ALERT ( at_aiconsole, "Couldn't validate hint type" );
	return FALSE;
}

//=========================================================
// FCanActiveIdle
//=========================================================
BOOL CBaseHuman :: FCanActiveIdle ( void )
{
	if ( InSquad() )
	{
		CSquadMonster *pSquadLeader = MySquadLeader();

		for (int i = 0; i < MAX_SQUAD_MEMBERS;i++)
		{
			CSquadMonster *pMember = pSquadLeader->MySquadMember(i);
			 
			if ( pMember != NULL && pMember != this && pMember->m_iHintNode != NO_NODE )
			{
				// someone else in the group is active idling right now!
				ALERT( at_console, ">someone else in the group is active idling right now!\n");
				return FALSE;
			}
		}
			
		ALERT( at_console, ">IN SQUAD FCanActiveIdle TRUE!\n");
		return TRUE;
	}
		
	ALERT( at_console, ">FCanActiveIdle TRUE!\n");

	return TRUE;
}

// make the entity enter a scripted sequence - NOT USED
void CBaseHuman :: PossessEntity( void )
{
	m_IdealMonsterState = MONSTERSTATE_SCRIPT;
	ALERT( at_aiconsole, "script \"%s\" using monster \"%s\"\n", STRING( "sidestep" ), STRING( "monster_human" ) );
}

// clean up and set final movement
BOOL CBaseHuman :: CineCleanup( )
{
	CCineMonster *pOldCine = m_pCine;

	// arg, punt
	pev->movetype = MOVETYPE_STEP;// this is evil
	pev->solid = SOLID_SLIDEBOX;
	
	m_pCine = NULL;
	m_hTargetEnt = NULL;
	m_pGoalEnt = NULL;

	// reset position
	Vector new_origin, new_angle;
	GetBonePosition( 0, new_origin, new_angle );

	Vector oldOrigin = pev->origin;

	if ((oldOrigin - new_origin).Length2D() < 8.0)
		new_origin = oldOrigin;

	pev->origin.x = new_origin.x;
	pev->origin.y = new_origin.y;
	pev->origin.z += 1;

	pev->flags |= FL_ONGROUND;
	int drop = DROP_TO_FLOOR( ENT(pev) );
	
	// Origin in solid?  Set to org at the end of the sequence
	if ( drop < 0 )
		pev->origin = oldOrigin;
	else if ( drop == 0 ) // Hanging in air?
	{
		pev->origin.z = new_origin.z;
		pev->flags &= ~FL_ONGROUND;
	}
	// else entity hit floor, leave there

	UTIL_SetOrigin( pev, pev->origin );//SP FIX
	pev->effects |= EF_NOINTERP;

	m_Activity = ACT_RESET;
	
	pev->enemy = NULL;

	if ( pev->health > 0 )
		m_IdealMonsterState = MONSTERSTATE_IDLE; // m_previousState;

	return TRUE;
}


void CBaseHuman :: ShowLine( Vector vecStart, Vector vecEnd, int iColor )
{
	if ( CVAR_GET_FLOAT( "developer" ) == 0 )
	return;
/*
	int r = 0;
	int g = 0;
	int b = 0;

	if( iColor == 1 )
	{
		r = 255;
		g = 0;
		b = 0;
	}

	if( iColor == 2 )
	{
		r = 0;
		g = 255;
		b = 0;
	}

	if( iColor == 3 )
	{
		r = 0;
		g = 0;
		b = 255;
	}

	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_BEAMPOINTS );
		WRITE_COORD( vecStart.x );
		WRITE_COORD( vecStart.y );
		WRITE_COORD( vecStart.z );

		WRITE_COORD( vecEnd.x );
		WRITE_COORD( vecEnd.y );
		WRITE_COORD( vecEnd.z );
		WRITE_SHORT( g_sModelIndexLaser );
		WRITE_BYTE( 0 ); // framerate
		WRITE_BYTE( 0 ); // framerate

		WRITE_BYTE( 3 ); // life
		WRITE_BYTE( 20 );  // width
		WRITE_BYTE( 0 );   // noise

		WRITE_BYTE( r );   // r, g, b
		WRITE_BYTE( g );   // r, g, b
		WRITE_BYTE( b );   // r, g, b
		WRITE_BYTE( 255 );	// brightness
		WRITE_BYTE( 10 );		// speed
	MESSAGE_END();*/
}

CBaseEntity* CBaseHuman::DropGun(char *szName, Vector origin, Vector angles)
{
	edict_t *pEnt = CREATE_NAMED_ENTITY(MAKE_STRING(szName));

	if (FNullEnt(pEnt))
	{
	//	ALERT(at_de, "NULL Ent in DropGun!\n");
		return NULL;
	}

	CBaseEntity *pGun = Instance(pEnt);

	if (pGun)
	{
		pGun->pev->armorvalue = m_cAmmoLoaded;

		pGun->pev->owner = ENT(pev);
		pGun->pev->origin = pev->origin;
		pGun->pev->angles = pev->angles;

		pGun->pev->absmin = pGun->pev->origin - Vector(1, 1, 1);
		pGun->pev->absmax = pGun->pev->origin + Vector(1, 1, 1);

		pGun->Spawn();

		return pGun;
	}
	else
	{
		ALERT(at_console, "DropGun: Cannot create entity %s!\n", szName);
		return NULL;
	}
}