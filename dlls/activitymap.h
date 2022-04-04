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

#define _A( a ) { a, #a }

activity_map_t activity_map[] =
{
	_A(ACT_IDLE),//1
	_A(ACT_GUARD),//2
	_A(ACT_WALK),//4
	_A(ACT_RUN),//3
	_A(ACT_FLY),//5
	_A(ACT_SWIM),//6
	_A(ACT_HOP),//7
	_A(ACT_LEAP),//8
	_A(ACT_FALL),//9
	_A(ACT_LAND),//10
	_A(ACT_STRAFE_LEFT),//11
	_A(ACT_STRAFE_RIGHT),//12
	_A(ACT_ROLL_LEFT),//13
	_A(ACT_ROLL_RIGHT),//14
	_A(ACT_TURN_LEFT),//15
	_A(ACT_TURN_RIGHT),//16
	_A(ACT_CROUCH),//17
	_A(ACT_CROUCHIDLE),//18
	_A(ACT_STAND),//19
	_A(ACT_USE),//20
	_A(ACT_SIGNAL1),//21
	_A(ACT_SIGNAL2),//22
	_A(ACT_SIGNAL3),//23
	_A(ACT_TWITCH),//24
	_A(ACT_COWER),//25
	_A(ACT_SMALL_FLINCH),//26
	_A(ACT_BIG_FLINCH),//27
	_A(ACT_RANGE_ATTACK1),//28
	_A(ACT_RANGE_ATTACK2),//29
	_A(ACT_MELEE_ATTACK1),//30
	_A(ACT_MELEE_ATTACK2),//31
	_A(ACT_RELOAD),//32
	_A(ACT_ARM),//33
	_A(ACT_DISARM),//34
	_A(ACT_EAT),//35
	_A(ACT_DIESIMPLE),//36
	_A(ACT_DIEBACKWARD),//37
	_A(ACT_DIEFORWARD),//38
	_A(ACT_DIEVIOLENT),//39
	_A(ACT_BARNACLE_HIT),//40
	_A(ACT_BARNACLE_PULL),//41
	_A(ACT_BARNACLE_CHOMP),//42
	_A(ACT_BARNACLE_CHEW),//43
	_A(ACT_SLEEP),//44
	_A(ACT_INSPECT_FLOOR),//45
	_A(ACT_INSPECT_WALL),//46
	_A(ACT_IDLE_ANGRY),//47
	_A(ACT_WALK_HURT),//48
	_A(ACT_RUN_HURT),//49
	_A(ACT_HOVER),//50
	_A(ACT_GLIDE),//51
	_A(ACT_FLY_LEFT),//52
	_A(ACT_FLY_RIGHT),//53
	_A(ACT_DETECT_SCENT),//54
	_A(ACT_SNIFF),//55
	_A(ACT_BITE),//56
	_A(ACT_THREAT_DISPLAY),//57
	_A(ACT_FEAR_DISPLAY),//58
	_A(ACT_EXCITED),//59
	_A(ACT_SPECIAL_ATTACK1),//60
	_A(ACT_SPECIAL_ATTACK2),//61
	_A(ACT_COMBAT_IDLE),//62
	_A(ACT_WALK_SCARED),//63
	_A(ACT_RUN_SCARED),//64
	_A(ACT_VICTORY_DANCE),//65
	_A(ACT_DIE_HEADSHOT),//66
	_A(ACT_DIE_CHESTSHOT),//67
	_A(ACT_DIE_GUTSHOT),//68
	_A(ACT_DIE_BACKSHOT),//69
	_A(ACT_FLINCH_HEAD),//70
	_A(ACT_FLINCH_CHEST),//71
	_A(ACT_FLINCH_STOMACH),//72
	_A(ACT_FLINCH_LEFTARM),//73
	_A(ACT_FLINCH_RIGHTARM),//74
	_A(ACT_FLINCH_LEFTLEG),//75
	_A(ACT_FLINCH_RIGHTLEG),//76
	_A(ACT_PRONE),//77
	_A(ACT_PRONESTAND),//78
	_A(ACT_DIEPRONE),//79
	_A(ACT_UNCROUCH),//80

	_A(ACT_EXPLOSION_HIT),//81
	_A(ACT_EXPLOSION_FLY),//82
	_A(ACT_EXPLOSION_LAND),//83

	//test
	_A(ACT_JUMP_OFF),//84
	_A(ACT_JUMP_LOOP),//85
	_A(ACT_JUMP_LAND),//86

	_A(ACT_SURRENDER),//87
	_A(ACT_DROP_WEAPON),//88
	_A(ACT_PICK_ITEM),//89
	_A(ACT_FLINCH_GUT),//90
	_A(ACT_DIE_HEADSHOT_VIOLENT),//91
	_A(ACT_DIE_COLLIDE),//92
	_A(ACT_WAKEUP),//93
	_A(ACT_DIE_SLEEPING),//94
	_A(ACT_SIDESTEP_LEFT),//95
	_A(ACT_SIDESTEP_RIGHT),//96
	_A(ACT_ON_FIRE),//97
	0, NULL
};
