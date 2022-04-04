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
//=========================================================
// skill.h - skill level concerns
//=========================================================

struct skilldata_t
{

	int iSkillLevel; // game skill level

	//==================================================
	// MONSTERS
	//==================================================

	float agruntHealth;
	float agruntDmgPunch;

	float apacheHealth;

	float barneyHealth;

	float bigmommaHealthFactor;		// Multiply each node's health by this
	float bigmommaDmgSlash;			// melee attack damage
	float bigmommaDmgBlast;			// mortar attack damage
	float bigmommaRadiusBlast;		// mortar attack radius

	float bullsquidHealth;
	float bullsquidDmgBite;
	float bullsquidDmgWhip;
	float bullsquidDmgSpit;

	float gargantuaHealth;
	float gargantuaDmgSlash;
	float gargantuaDmgFire;
	float gargantuaDmgStomp;

	float hassassinHealth;

	float headcrabHealth;
	float headcrabDmgBite;

	float hgruntHealth;
	float hgruntDmgKick;
	float hgruntShotgunPellets;
	float hgruntGrenadeSpeed;

	float houndeyeHealth;
	float houndeyeDmgBlast;

	float slaveHealth;
	float slaveDmgClaw;
	float slaveDmgClawrake;
	float slaveDmgZap;

	float ichthyosaurHealth;
	float ichthyosaurDmgShake;

	float leechHealth;
	float leechDmgBite;

	float controllerHealth;
	float controllerDmgZap;
	float controllerSpeedBall;
	float controllerDmgBall;

	float nihilanthHealth;
	float nihilanthZap;

	float scientistHealth;

	float snarkHealth;
	float snarkDmgBite;
	float snarkDmgPop;

	float zombieHealth;
	float zombieDmgOneSlash;
	float zombieDmgBothSlash;

	float turretHealth;
	float miniturretHealth;
	float sentryHealth;

	//==================================================
	// PLAYER WEAPONS
	//==================================================

	float plrDmgKnife;
	float plrDmgKnifeStab;
	float plrDmgFists;
	float plrDmgSemtex;
	float plrDmgDeagle;
	float plrDmg1911;
	float plrDmgBeretta;
	float plrDmgGlock;
	float plrDmgAnaconda;
	float plrDmgMP5;
	float plrDmgP90;
	float plrDmgBizon;
	float plrDmgFAMAS;
	float plrDmgM4A1;
	float plrDmgAK47;
	float plrDmgM249;
	float plrDmgSPAS12;
	float plrDmgLessLethal;
	float plrDmgUSAS;
	float plrDmgM14;
	float plrDmgL96A1;
	float plrDmgRPG;
	// Half-Life backward compatibility
	float plrDmgM203Grenade;
	float plrDmgTripmine;

	//==================================================
	// MONSTERS WEAPONS
	//==================================================

	float monDmg9MM;
	float monDmgMP5;
	float monDmg12MM;
	float monDmgHornet;

	//==================================================
	// HEALTH AND ARMOR
	//==================================================

	float suitchargerCapacity;
	float batteryCapacity;
	float healthchargerCapacity;
	float healthkitCapacity;
	float scientistHeal;

	//==================================================
	// MONSTERS DAMAGE MODIFIERS
	//==================================================

	float monHead;
	float monChest;
	float monStomach;
	float monLeg;
	float monArm;

	//==================================================
	// PLAYER DAMAGE MODIFIERS
	//==================================================

	float plrHead;
	float plrChest;
	float plrStomach;
	float plrLeg;
	float plrArm;
};

extern	DLL_GLOBAL	skilldata_t	gSkillData;
float GetSkillCvar( char *pName );

extern DLL_GLOBAL int		g_iSkillLevel;

#define SKILL_EASY		1
#define SKILL_MEDIUM	2
#define SKILL_HARD		3
