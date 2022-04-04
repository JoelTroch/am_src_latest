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

#include "extdll.h"
#include "eiface.h"
#include "util.h"
#include "game.h"

cvar_t	displaysoundlist = {"displaysoundlist","0"};

// multiplayer server rules
cvar_t	fragsleft	= {"mp_fragsleft","0", FCVAR_SERVER | FCVAR_UNLOGGED };	  // Don't spam console/log files/users with this changing
cvar_t	timeleft	= {"mp_timeleft","0" , FCVAR_SERVER | FCVAR_UNLOGGED };	  // "      "

// multiplayer server rules
cvar_t	teamplay	= {"mp_teamplay","0", FCVAR_SERVER };
cvar_t	fraglimit	= {"mp_fraglimit","0", FCVAR_SERVER };
cvar_t	timelimit	= {"mp_timelimit","0", FCVAR_SERVER };
cvar_t	friendlyfire = {"mp_friendlyfire","0", FCVAR_SERVER };
cvar_t	falldamage	= {"mp_falldamage","0", FCVAR_SERVER };
cvar_t	weaponstay	= {"mp_weaponstay","0", FCVAR_SERVER };
cvar_t	forcerespawn= {"mp_forcerespawn","1", FCVAR_SERVER };
cvar_t	flashlight	= {"mp_flashlight","0", FCVAR_SERVER };
cvar_t	aimcrosshair = {"mp_autocrosshair","1", FCVAR_SERVER };
cvar_t	decalfrequency = {"decalfrequency","30", FCVAR_SERVER };
cvar_t	teamlist = {"mp_teamlist","hgrunt;scientist", FCVAR_SERVER };
cvar_t	teamoverride = {"mp_teamoverride","1" };
cvar_t	defaultteam = {"mp_defaultteam","0" };
cvar_t	allowmonsters = {"mp_allowmonsters","0", FCVAR_SERVER };

cvar_t  allow_spectators = { "allow_spectators", "0.0", FCVAR_SERVER };		// 0 prevents players from being spectators

cvar_t  mp_chattime = {"mp_chattime","10", FCVAR_SERVER };

// Shepard : Developers CVARs
#ifdef DEBUG
cvar_t am_infinite_stamina = { "am_infinite_stamina", "0", FCVAR_SERVER };
cvar_t am_wp_infinite_ammo = { "am_wp_infinite_ammo", "0", FCVAR_SERVER };
cvar_t am_wp_debug = { "am_wp_debug", "0", FCVAR_SERVER };
cvar_t am_wp_spread_x_max = { "am_wp_spread_x_max", "1.5", FCVAR_SERVER | FCVAR_UNLOGGED };
cvar_t am_wp_spread_y_max = { "am_wp_spread_y_max", "3.5", FCVAR_SERVER | FCVAR_UNLOGGED };
cvar_t am_wp_spread_x_base = { "am_wp_spread_x_base", "0.07", FCVAR_SERVER | FCVAR_UNLOGGED };
cvar_t am_wp_spread_y_base = { "am_wp_spread_y_base", "0.15", FCVAR_SERVER | FCVAR_UNLOGGED };
cvar_t am_wp_spread_x_mod = { "am_wp_spread_x_mod", "0.01", FCVAR_SERVER | FCVAR_UNLOGGED };
cvar_t am_wp_spread_y_mod = { "am_wp_spread_y_mod", "0.03", FCVAR_SERVER | FCVAR_UNLOGGED };
cvar_t am_wp_spread_dir_change = { "am_wp_spread_dir_change", "10", FCVAR_SERVER | FCVAR_UNLOGGED };
cvar_t am_wp_spread_drop = { "am_wp_spread_drop", "0.2", FCVAR_SERVER | FCVAR_UNLOGGED };
cvar_t am_wp_shake_amount = { "am_wp_shake_amount", "50", FCVAR_SERVER | FCVAR_UNLOGGED };
#endif

// Engine Cvars
cvar_t 	*g_psv_gravity = NULL;
cvar_t	*g_psv_aim = NULL;
cvar_t	*g_footsteps = NULL;

//CVARS FOR SKILL LEVEL SETTINGS

//==================================================
// HALF-LIFE'S MONSTERS
//==================================================

// Alien Grunt
cvar_t	sk_agrunt_health1 = {"sk_agrunt_health1","0"};
cvar_t	sk_agrunt_health2 = {"sk_agrunt_health2","0"};
cvar_t	sk_agrunt_health3 = {"sk_agrunt_health3","0"};

cvar_t	sk_agrunt_dmg_punch1 = {"sk_agrunt_dmg_punch1","0"};
cvar_t	sk_agrunt_dmg_punch2 = {"sk_agrunt_dmg_punch2","0"};
cvar_t	sk_agrunt_dmg_punch3 = {"sk_agrunt_dmg_punch3","0"};

// Apache
cvar_t	sk_apache_health1	= {"sk_apache_health1","0"};
cvar_t	sk_apache_health2	= {"sk_apache_health2","0"};
cvar_t	sk_apache_health3	= {"sk_apache_health3","0"};

// Barney
cvar_t	sk_barney_health1	= {"sk_barney_health1","0"};
cvar_t	sk_barney_health2	= {"sk_barney_health2","0"};
cvar_t	sk_barney_health3	= {"sk_barney_health3","0"};

// Bullsquid
cvar_t	sk_bullsquid_health1 = {"sk_bullsquid_health1","0"};
cvar_t	sk_bullsquid_health2 = {"sk_bullsquid_health2","0"};
cvar_t	sk_bullsquid_health3 = {"sk_bullsquid_health3","0"};

cvar_t	sk_bullsquid_dmg_bite1 = {"sk_bullsquid_dmg_bite1","0"};
cvar_t	sk_bullsquid_dmg_bite2 = {"sk_bullsquid_dmg_bite2","0"};
cvar_t	sk_bullsquid_dmg_bite3 = {"sk_bullsquid_dmg_bite3","0"};

cvar_t	sk_bullsquid_dmg_whip1 = {"sk_bullsquid_dmg_whip1","0"};
cvar_t	sk_bullsquid_dmg_whip2 = {"sk_bullsquid_dmg_whip2","0"};
cvar_t	sk_bullsquid_dmg_whip3 = {"sk_bullsquid_dmg_whip3","0"};

cvar_t	sk_bullsquid_dmg_spit1 = {"sk_bullsquid_dmg_spit1","0"};
cvar_t	sk_bullsquid_dmg_spit2 = {"sk_bullsquid_dmg_spit2","0"};
cvar_t	sk_bullsquid_dmg_spit3 = {"sk_bullsquid_dmg_spit3","0"};

// Big Momma
cvar_t	sk_bigmomma_health_factor1 = {"sk_bigmomma_health_factor1","1.0"};
cvar_t	sk_bigmomma_health_factor2 = {"sk_bigmomma_health_factor2","1.0"};
cvar_t	sk_bigmomma_health_factor3 = {"sk_bigmomma_health_factor3","1.0"};

cvar_t	sk_bigmomma_dmg_slash1 = {"sk_bigmomma_dmg_slash1","50"};
cvar_t	sk_bigmomma_dmg_slash2 = {"sk_bigmomma_dmg_slash2","50"};
cvar_t	sk_bigmomma_dmg_slash3 = {"sk_bigmomma_dmg_slash3","50"};

cvar_t	sk_bigmomma_dmg_blast1 = {"sk_bigmomma_dmg_blast1","100"};
cvar_t	sk_bigmomma_dmg_blast2 = {"sk_bigmomma_dmg_blast2","100"};
cvar_t	sk_bigmomma_dmg_blast3 = {"sk_bigmomma_dmg_blast3","100"};

cvar_t	sk_bigmomma_radius_blast1 = {"sk_bigmomma_radius_blast1","250"};
cvar_t	sk_bigmomma_radius_blast2 = {"sk_bigmomma_radius_blast2","250"};
cvar_t	sk_bigmomma_radius_blast3 = {"sk_bigmomma_radius_blast3","250"};

// Gargantua
cvar_t	sk_gargantua_health1 = {"sk_gargantua_health1","0"};
cvar_t	sk_gargantua_health2 = {"sk_gargantua_health2","0"};
cvar_t	sk_gargantua_health3 = {"sk_gargantua_health3","0"};

cvar_t	sk_gargantua_dmg_slash1	= {"sk_gargantua_dmg_slash1","0"};
cvar_t	sk_gargantua_dmg_slash2	= {"sk_gargantua_dmg_slash2","0"};
cvar_t	sk_gargantua_dmg_slash3	= {"sk_gargantua_dmg_slash3","0"};

cvar_t	sk_gargantua_dmg_fire1 = {"sk_gargantua_dmg_fire1","0"};
cvar_t	sk_gargantua_dmg_fire2 = {"sk_gargantua_dmg_fire2","0"};
cvar_t	sk_gargantua_dmg_fire3 = {"sk_gargantua_dmg_fire3","0"};

cvar_t	sk_gargantua_dmg_stomp1	= {"sk_gargantua_dmg_stomp1","0"};
cvar_t	sk_gargantua_dmg_stomp2	= {"sk_gargantua_dmg_stomp2","0"};
cvar_t	sk_gargantua_dmg_stomp3	= {"sk_gargantua_dmg_stomp3","0"};

// Hassassin
cvar_t	sk_hassassin_health1 = {"sk_hassassin_health1","0"};
cvar_t	sk_hassassin_health2 = {"sk_hassassin_health2","0"};
cvar_t	sk_hassassin_health3 = {"sk_hassassin_health3","0"};

// Headcrab
cvar_t	sk_headcrab_health1 = {"sk_headcrab_health1","0"};
cvar_t	sk_headcrab_health2 = {"sk_headcrab_health2","0"};
cvar_t	sk_headcrab_health3 = {"sk_headcrab_health3","0"};

cvar_t	sk_headcrab_dmg_bite1 = {"sk_headcrab_dmg_bite1","0"};
cvar_t	sk_headcrab_dmg_bite2 = {"sk_headcrab_dmg_bite2","0"};
cvar_t	sk_headcrab_dmg_bite3 = {"sk_headcrab_dmg_bite3","0"};

// Hgrunt
cvar_t	sk_hgrunt_health1 = {"sk_hgrunt_health1","0"};
cvar_t	sk_hgrunt_health2 = {"sk_hgrunt_health2","0"};
cvar_t	sk_hgrunt_health3 = {"sk_hgrunt_health3","0"};

cvar_t	sk_hgrunt_kick1 = {"sk_hgrunt_kick1","0"};
cvar_t	sk_hgrunt_kick2 = {"sk_hgrunt_kick2","0"};
cvar_t	sk_hgrunt_kick3 = {"sk_hgrunt_kick3","0"};

cvar_t	sk_hgrunt_pellets1 = {"sk_hgrunt_pellets1","0"};
cvar_t	sk_hgrunt_pellets2 = {"sk_hgrunt_pellets2","0"};
cvar_t	sk_hgrunt_pellets3 = {"sk_hgrunt_pellets3","0"};

cvar_t	sk_hgrunt_gspeed1 = {"sk_hgrunt_gspeed1","0"};
cvar_t	sk_hgrunt_gspeed2 = {"sk_hgrunt_gspeed2","0"};
cvar_t	sk_hgrunt_gspeed3 = {"sk_hgrunt_gspeed3","0"};

// Houndeye
cvar_t	sk_houndeye_health1 = {"sk_houndeye_health1","0"};
cvar_t	sk_houndeye_health2 = {"sk_houndeye_health2","0"};
cvar_t	sk_houndeye_health3 = {"sk_houndeye_health3","0"};

cvar_t	sk_houndeye_dmg_blast1 = {"sk_houndeye_dmg_blast1","0"};
cvar_t	sk_houndeye_dmg_blast2 = {"sk_houndeye_dmg_blast2","0"};
cvar_t	sk_houndeye_dmg_blast3 = {"sk_houndeye_dmg_blast3","0"};

// ISlave
cvar_t	sk_islave_health1 = {"sk_islave_health1","0"};
cvar_t	sk_islave_health2 = {"sk_islave_health2","0"};
cvar_t	sk_islave_health3 = {"sk_islave_health3","0"};

cvar_t	sk_islave_dmg_claw1 = {"sk_islave_dmg_claw1","0"};
cvar_t	sk_islave_dmg_claw2 = {"sk_islave_dmg_claw2","0"};
cvar_t	sk_islave_dmg_claw3 = {"sk_islave_dmg_claw3","0"};

cvar_t	sk_islave_dmg_clawrake1	= {"sk_islave_dmg_clawrake1","0"};
cvar_t	sk_islave_dmg_clawrake2	= {"sk_islave_dmg_clawrake2","0"};
cvar_t	sk_islave_dmg_clawrake3	= {"sk_islave_dmg_clawrake3","0"};

cvar_t	sk_islave_dmg_zap1 = {"sk_islave_dmg_zap1","0"};
cvar_t	sk_islave_dmg_zap2 = {"sk_islave_dmg_zap2","0"};
cvar_t	sk_islave_dmg_zap3 = {"sk_islave_dmg_zap3","0"};

// Icthyosaur
cvar_t	sk_ichthyosaur_health1	= {"sk_ichthyosaur_health1","0"};
cvar_t	sk_ichthyosaur_health2	= {"sk_ichthyosaur_health2","0"};
cvar_t	sk_ichthyosaur_health3	= {"sk_ichthyosaur_health3","0"};

cvar_t	sk_ichthyosaur_shake1	= {"sk_ichthyosaur_shake1","0"};
cvar_t	sk_ichthyosaur_shake2	= {"sk_ichthyosaur_shake2","0"};
cvar_t	sk_ichthyosaur_shake3	= {"sk_ichthyosaur_shake3","0"};

// Leech
cvar_t	sk_leech_health1 = {"sk_leech_health1","0"};
cvar_t	sk_leech_health2 = {"sk_leech_health2","0"};
cvar_t	sk_leech_health3 = {"sk_leech_health3","0"};

cvar_t	sk_leech_dmg_bite1 = {"sk_leech_dmg_bite1","0"};
cvar_t	sk_leech_dmg_bite2 = {"sk_leech_dmg_bite2","0"};
cvar_t	sk_leech_dmg_bite3 = {"sk_leech_dmg_bite3","0"};

// Controller
cvar_t	sk_controller_health1 = {"sk_controller_health1","0"};
cvar_t	sk_controller_health2 = {"sk_controller_health2","0"};
cvar_t	sk_controller_health3 = {"sk_controller_health3","0"};

cvar_t	sk_controller_dmgzap1 = {"sk_controller_dmgzap1","0"};
cvar_t	sk_controller_dmgzap2 = {"sk_controller_dmgzap2","0"};
cvar_t	sk_controller_dmgzap3 = {"sk_controller_dmgzap3","0"};

cvar_t	sk_controller_speedball1 = {"sk_controller_speedball1","0"};
cvar_t	sk_controller_speedball2 = {"sk_controller_speedball2","0"};
cvar_t	sk_controller_speedball3 = {"sk_controller_speedball3","0"};

cvar_t	sk_controller_dmgball1 = {"sk_controller_dmgball1","0"};
cvar_t	sk_controller_dmgball2 = {"sk_controller_dmgball2","0"};
cvar_t	sk_controller_dmgball3 = {"sk_controller_dmgball3","0"};

// Nihilanth
cvar_t	sk_nihilanth_health1 = {"sk_nihilanth_health1","0"};
cvar_t	sk_nihilanth_health2 = {"sk_nihilanth_health2","0"};
cvar_t	sk_nihilanth_health3 = {"sk_nihilanth_health3","0"};

cvar_t	sk_nihilanth_zap1 = {"sk_nihilanth_zap1","0"};
cvar_t	sk_nihilanth_zap2 = {"sk_nihilanth_zap2","0"};
cvar_t	sk_nihilanth_zap3 = {"sk_nihilanth_zap3","0"};

// Scientist
cvar_t	sk_scientist_health1 = {"sk_scientist_health1","0"};
cvar_t	sk_scientist_health2 = {"sk_scientist_health2","0"};
cvar_t	sk_scientist_health3 = {"sk_scientist_health3","0"};

// Snark
cvar_t	sk_snark_health1 = {"sk_snark_health1","0"};
cvar_t	sk_snark_health2 = {"sk_snark_health2","0"};
cvar_t	sk_snark_health3 = {"sk_snark_health3","0"};

cvar_t	sk_snark_dmg_bite1 = {"sk_snark_dmg_bite1","0"};
cvar_t	sk_snark_dmg_bite2 = {"sk_snark_dmg_bite2","0"};
cvar_t	sk_snark_dmg_bite3 = {"sk_snark_dmg_bite3","0"};

cvar_t	sk_snark_dmg_pop1 = {"sk_snark_dmg_pop1","0"};
cvar_t	sk_snark_dmg_pop2 = {"sk_snark_dmg_pop2","0"};
cvar_t	sk_snark_dmg_pop3 = {"sk_snark_dmg_pop3","0"};

// Zombie
cvar_t	sk_zombie_health1 = {"sk_zombie_health1","0"};
cvar_t	sk_zombie_health2 = {"sk_zombie_health2","0"};
cvar_t	sk_zombie_health3 = {"sk_zombie_health3","0"};

cvar_t	sk_zombie_dmg_one_slash1 = {"sk_zombie_dmg_one_slash1","0"};
cvar_t	sk_zombie_dmg_one_slash2 = {"sk_zombie_dmg_one_slash2","0"};
cvar_t	sk_zombie_dmg_one_slash3 = {"sk_zombie_dmg_one_slash3","0"};

cvar_t	sk_zombie_dmg_both_slash1 = {"sk_zombie_dmg_both_slash1","0"};
cvar_t	sk_zombie_dmg_both_slash2 = {"sk_zombie_dmg_both_slash2","0"};
cvar_t	sk_zombie_dmg_both_slash3 = {"sk_zombie_dmg_both_slash3","0"};

// Turret
cvar_t	sk_turret_health1 = {"sk_turret_health1","0"};
cvar_t	sk_turret_health2 = {"sk_turret_health2","0"};
cvar_t	sk_turret_health3 = {"sk_turret_health3","0"};


// Mini turret
cvar_t	sk_miniturret_health1 = {"sk_miniturret_health1","0"};
cvar_t	sk_miniturret_health2 = {"sk_miniturret_health2","0"};
cvar_t	sk_miniturret_health3 = {"sk_miniturret_health3","0"};

// Sentry turret
cvar_t	sk_sentry_health1 = {"sk_sentry_health1","0"};
cvar_t	sk_sentry_health2 = {"sk_sentry_health2","0"};
cvar_t	sk_sentry_health3 = {"sk_sentry_health3","0"};

// ==================================================
// PLAYER WEAPONS
// ==================================================

// ========== MELEE AND ITEMS ==========
// Knife whack (normal)
cvar_t	sk_plr_knife1 = { "sk_plr_knife1", "0" };
cvar_t	sk_plr_knife2 = { "sk_plr_knife2", "0" };
cvar_t	sk_plr_knife3 = { "sk_plr_knife3", "0" };

// Knife whack (stab)
cvar_t	sk_plr_knife_stab1 = { "sk_plr_knife_stab1", "0" };
cvar_t	sk_plr_knife_stab2 = { "sk_plr_knife_stab2", "0" };
cvar_t	sk_plr_knife_stab3 = { "sk_plr_knife_stab3", "0" };

// Fists
cvar_t	sk_plr_fists1 = { "sk_plr_fists1", "0" };
cvar_t	sk_plr_fists2 = { "sk_plr_fists2", "0" };
cvar_t	sk_plr_fists3 = { "sk_plr_fists3", "0" };

// Semtex charge
cvar_t	sk_plr_semtex1 = { "sk_plr_semtex1", "0" };
cvar_t	sk_plr_semtex2 = { "sk_plr_semtex2", "0" };
cvar_t	sk_plr_semtex3 = { "sk_plr_semtex3", "0" };

// ========== PISTOLS ==========
// IMI Desert Eagle .50 Round
cvar_t	sk_plr_deagle_bullet1 = { "sk_plr_deagle_bullet1", "0" };
cvar_t	sk_plr_deagle_bullet2 = { "sk_plr_deagle_bullet2", "0" };
cvar_t	sk_plr_deagle_bullet3 = { "sk_plr_deagle_bullet3", "0" };

// Colt M1911A2 45 ACP Round
cvar_t	sk_plr_1911_bullet1 = { "sk_plr_1911_bullet1", "0" };
cvar_t	sk_plr_1911_bullet2 = { "sk_plr_1911_bullet2", "0" };
cvar_t	sk_plr_1911_bullet3 = { "sk_plr_1911_bullet3", "0" };

// Beretta 92 FS 9mm Round
cvar_t	sk_plr_ber92f_bullet1 = { "sk_plr_ber92f_bullet1", "0" };
cvar_t	sk_plr_ber92f_bullet2 = { "sk_plr_ber92f_bullet2", "0" };
cvar_t	sk_plr_ber92f_bullet3 = { "sk_plr_ber92f_bullet3", "0" };

// Glock 18 9mm Round
cvar_t	sk_plr_glock18_bullet1 = { "sk_plr_glock18_bullet1", "0" };
cvar_t	sk_plr_glock18_bullet2 = { "sk_plr_glock18_bullet2", "0" };
cvar_t	sk_plr_glock18_bullet3 = { "sk_plr_glock18_bullet3", "0" };

// Colt Anaconda (Python) .44 Round
cvar_t	sk_plr_anaconda_bullet1 = { "sk_plr_python_bullet1", "0" };
cvar_t	sk_plr_anaconda_bullet2 = { "sk_plr_python_bullet2", "0" };
cvar_t	sk_plr_anaconda_bullet3 = { "sk_plr_python_bullet3", "0" };

// ========== SMGS ==========
// H&K MP5 9mm Round
cvar_t	sk_plr_mp5_bullet1 = { "sk_plr_mp5_bullet1", "0" };
cvar_t	sk_plr_mp5_bullet2 = { "sk_plr_mp5_bullet2", "0" };
cvar_t	sk_plr_mp5_bullet3 = { "sk_plr_mp5_bullet3", "0" };

// FN P90 .57 Round
cvar_t	sk_plr_p90_bullet1 = { "sk_plr_p90_bullet1", "0" };
cvar_t	sk_plr_p90_bullet2 = { "sk_plr_p90_bullet2", "0" };
cvar_t	sk_plr_p90_bullet3 = { "sk_plr_p90_bullet3", "0" };

// PP-19 Bizon 9mm Round
cvar_t	sk_plr_bizon_bullet1 = { "sk_plr_bizon_bullet1", "0" };
cvar_t	sk_plr_bizon_bullet2 = { "sk_plr_bizon_bullet2", "0" };
cvar_t	sk_plr_bizon_bullet3 = { "sk_plr_bizon_bullet3", "0" };

// ========== RIFLES ==========
// FAMAS G2 5.56 Round
cvar_t	sk_plr_famas_bullet1 = { "sk_plr_famas_bullet1", "0" };
cvar_t	sk_plr_famas_bullet2 = { "sk_plr_famas_bullet2", "0" };
cvar_t	sk_plr_famas_bullet3 = { "sk_plr_famas_bullet3", "0" };

// Colt M4A1 5.56 Round
cvar_t	sk_plr_m4a1_bullet1 = { "sk_plr_m4a1_bullet1", "0" };
cvar_t	sk_plr_m4a1_bullet2 = { "sk_plr_m4a1_bullet2", "0" };
cvar_t	sk_plr_m4a1_bullet3 = { "sk_plr_m4a1_bullet3", "0" };

// AK47 7.62 Round
cvar_t	sk_plr_ak47_bullet1 = { "sk_plr_ak47_bullet1", "0" };
cvar_t	sk_plr_ak47_bullet2 = { "sk_plr_ak47_bullet2", "0" };
cvar_t	sk_plr_ak47_bullet3 = { "sk_plr_ak47_bullet3", "0" };

// M249 5.56 Round
cvar_t	sk_plr_m249_bullet1 = { "sk_plr_m249_bullet1", "0" };
cvar_t	sk_plr_m249_bullet2 = { "sk_plr_m249_bullet2", "0" };
cvar_t	sk_plr_m249_bullet3 = { "sk_plr_m249_bullet3", "0" };

// ========== SHOTGUNS ==========
// SPAS 12 Round
cvar_t	sk_plr_spas12_buckshot1 = { "sk_plr_spas12_buckshot1", "0" };
cvar_t	sk_plr_spas12_buckshot2 = { "sk_plr_spas12_buckshot2", "0" };
cvar_t	sk_plr_spas12_buckshot3 = { "sk_plr_spas12_buckshot3", "0" };

// Less Lethal shotgun Round
cvar_t	sk_plr_less_lethal_buckshot1 = { "sk_plr_less_lethal_buckshot1", "0" };
cvar_t	sk_plr_less_lethal_buckshot2 = { "sk_plr_less_lethal_buckshot2", "0" };
cvar_t	sk_plr_less_lethal_buckshot3 = { "sk_plr_less_lethal_buckshot3", "0" };

// USAS-12 Round
cvar_t	sk_plr_usas_buckshot1 = { "sk_plr_usas_buckshot1", "0" };
cvar_t	sk_plr_usas_buckshot2 = { "sk_plr_usas_buckshot2", "0" };
cvar_t	sk_plr_usas_buckshot3 = { "sk_plr_usas_buckshot3", "0" };

// ========== SNIPERS ==========
// M14 7.62 Round
cvar_t	sk_plr_m14_bullet1 = { "sk_plr_m14_bullet1", "0" };
cvar_t	sk_plr_m14_bullet2 = { "sk_plr_m14_bullet2", "0" };
cvar_t	sk_plr_m14_bullet3 = { "sk_plr_m14_bullet3", "0" };

// L96A1 7.62 Round
cvar_t	sk_plr_l96a1_bullet1 = { "sk_plr_l96a1_bullet1", "0" };
cvar_t	sk_plr_l96a1_bullet2 = { "sk_plr_l96a1_bullet2", "0" };
cvar_t	sk_plr_l96a1_bullet3 = { "sk_plr_l96a1_bullet3", "0" };

// ========== EXPLOSIVES ==========
// RPG-7
cvar_t	sk_plr_rpg1 = { "sk_plr_rpg1", "0" };
cvar_t	sk_plr_rpg2 = { "sk_plr_rpg2", "0" };
cvar_t	sk_plr_rpg3 = { "sk_plr_rpg3", "0" };

// ========== HALF-LIFE BACKWARD COMPATIBILITY ==========
// HL's M203 grenade
cvar_t	sk_plr_9mmAR_grenade1 = {"sk_plr_9mmAR_grenade1","0"};
cvar_t	sk_plr_9mmAR_grenade2 = {"sk_plr_9mmAR_grenade2","0"};
cvar_t	sk_plr_9mmAR_grenade3 = {"sk_plr_9mmAR_grenade3","0"};

// HL's Tripmine
cvar_t	sk_plr_tripmine1 = { "sk_plr_tripmine1", "0" };
cvar_t	sk_plr_tripmine2 = { "sk_plr_tripmine2", "0" };
cvar_t	sk_plr_tripmine3 = { "sk_plr_tripmine3", "0" };

//==================================================
// MONSTERS WEAPONS
//==================================================

cvar_t	sk_12mm_bullet1 = {"sk_12mm_bullet1","0"};
cvar_t	sk_12mm_bullet2 = {"sk_12mm_bullet2","0"};
cvar_t	sk_12mm_bullet3 = {"sk_12mm_bullet3","0"};

cvar_t	sk_9mmAR_bullet1 = {"sk_9mmAR_bullet1","0"};
cvar_t	sk_9mmAR_bullet2 = {"sk_9mmAR_bullet2","0"};
cvar_t	sk_9mmAR_bullet3 = {"sk_9mmAR_bullet3","0"};

cvar_t	sk_9mm_bullet1 = {"sk_9mm_bullet1","0"};
cvar_t	sk_9mm_bullet2 = {"sk_9mm_bullet2","0"};
cvar_t	sk_9mm_bullet3 = {"sk_9mm_bullet3","0"};

cvar_t	sk_hornet_dmg1 = {"sk_hornet_dmg1","0"};
cvar_t	sk_hornet_dmg2 = {"sk_hornet_dmg2","0"};
cvar_t	sk_hornet_dmg3 = {"sk_hornet_dmg3","0"};

//==================================================
// HEALTH AND ARMOR
//==================================================

// HL's HEV Recharger
cvar_t	sk_suitcharger1	= { "sk_suitcharger1","0" };
cvar_t	sk_suitcharger2	= { "sk_suitcharger2","0" };
cvar_t	sk_suitcharger3	= { "sk_suitcharger3","0" };

// Kevlar vest
cvar_t	sk_battery1	= { "sk_battery1","0" };
cvar_t	sk_battery2	= { "sk_battery2","0" };
cvar_t	sk_battery3	= { "sk_battery3","0" };

// HL's Health Recharger
cvar_t	sk_healthcharger1	= { "sk_healthcharger1","0" };
cvar_t	sk_healthcharger2	= { "sk_healthcharger2","0" };
cvar_t	sk_healthcharger3	= { "sk_healthcharger3","0" };

// Medkit
cvar_t	sk_healthkit1	= { "sk_healthkit1","0" };
cvar_t	sk_healthkit2	= { "sk_healthkit2","0" };
cvar_t	sk_healthkit3	= { "sk_healthkit3","0" };

// HL's Scientist syringe
cvar_t	sk_scientist_heal1	= { "sk_scientist_heal1","0" };
cvar_t	sk_scientist_heal2	= { "sk_scientist_heal2","0" };
cvar_t	sk_scientist_heal3	= { "sk_scientist_heal3","0" };

//==================================================
// MONSTERS DAMAGE MODIFIERS
//==================================================

cvar_t	sk_monster_head1	= { "sk_monster_head1","2" };
cvar_t	sk_monster_head2	= { "sk_monster_head2","2" };
cvar_t	sk_monster_head3	= { "sk_monster_head3","2" };

cvar_t	sk_monster_chest1	= { "sk_monster_chest1","1" };
cvar_t	sk_monster_chest2	= { "sk_monster_chest2","1" };
cvar_t	sk_monster_chest3	= { "sk_monster_chest3","1" };

cvar_t	sk_monster_stomach1	= { "sk_monster_stomach1","1" };
cvar_t	sk_monster_stomach2	= { "sk_monster_stomach2","1" };
cvar_t	sk_monster_stomach3	= { "sk_monster_stomach3","1" };

cvar_t	sk_monster_arm1	= { "sk_monster_arm1","1" };
cvar_t	sk_monster_arm2	= { "sk_monster_arm2","1" };
cvar_t	sk_monster_arm3	= { "sk_monster_arm3","1" };

cvar_t	sk_monster_leg1	= { "sk_monster_leg1","1" };
cvar_t	sk_monster_leg2	= { "sk_monster_leg2","1" };
cvar_t	sk_monster_leg3	= { "sk_monster_leg3","1" };

//==================================================
// PLAYER DAMAGE MODIFIERS
//==================================================

cvar_t	sk_player_head1	= { "sk_player_head1","2" };
cvar_t	sk_player_head2	= { "sk_player_head2","2" };
cvar_t	sk_player_head3	= { "sk_player_head3","2" };

cvar_t	sk_player_chest1	= { "sk_player_chest1","1" };
cvar_t	sk_player_chest2	= { "sk_player_chest2","1" };
cvar_t	sk_player_chest3	= { "sk_player_chest3","1" };

cvar_t	sk_player_stomach1	= { "sk_player_stomach1","1" };
cvar_t	sk_player_stomach2	= { "sk_player_stomach2","1" };
cvar_t	sk_player_stomach3	= { "sk_player_stomach3","1" };

cvar_t	sk_player_arm1	= { "sk_player_arm1","1" };
cvar_t	sk_player_arm2	= { "sk_player_arm2","1" };
cvar_t	sk_player_arm3	= { "sk_player_arm3","1" };

cvar_t	sk_player_leg1	= { "sk_player_leg1","1" };
cvar_t	sk_player_leg2	= { "sk_player_leg2","1" };
cvar_t	sk_player_leg3	= { "sk_player_leg3","1" };

// END Cvars for Skill Level settings

// Register your console variables here
// This gets called one time when the game is initialied
void GameDLLInit( void )
{
	// Register cvars here:

	g_psv_gravity = CVAR_GET_POINTER( "sv_gravity" );
	g_psv_aim = CVAR_GET_POINTER( "sv_aim" );
	g_footsteps = CVAR_GET_POINTER( "mp_footsteps" );

	CVAR_REGISTER( &displaysoundlist );
	CVAR_REGISTER( &allow_spectators );

	CVAR_REGISTER( &teamplay );
	CVAR_REGISTER( &fraglimit );
	CVAR_REGISTER( &timelimit );

	CVAR_REGISTER( &fragsleft );
	CVAR_REGISTER( &timeleft );

	CVAR_REGISTER( &friendlyfire );
	CVAR_REGISTER( &falldamage );
	CVAR_REGISTER( &weaponstay );
	CVAR_REGISTER( &forcerespawn );
	CVAR_REGISTER( &flashlight );
	CVAR_REGISTER( &aimcrosshair );
	CVAR_REGISTER( &decalfrequency );
	CVAR_REGISTER( &teamlist );
	CVAR_REGISTER( &teamoverride );
	CVAR_REGISTER( &defaultteam );
	CVAR_REGISTER( &allowmonsters );

	CVAR_REGISTER( &mp_chattime );

// REGISTER CVARS FOR SKILL LEVEL STUFF

	//==================================================
	// HALF-LIFE MONSTERS
	//==================================================

	// Alien Grunt
	CVAR_REGISTER( &sk_agrunt_health1 );
	CVAR_REGISTER( &sk_agrunt_health2 );
	CVAR_REGISTER( &sk_agrunt_health3 );

	CVAR_REGISTER( &sk_agrunt_dmg_punch1 );
	CVAR_REGISTER( &sk_agrunt_dmg_punch2 );
	CVAR_REGISTER( &sk_agrunt_dmg_punch3 );

	// Apache
	CVAR_REGISTER( &sk_apache_health1 );
	CVAR_REGISTER( &sk_apache_health2 );
	CVAR_REGISTER( &sk_apache_health3 );

	// Barney
	CVAR_REGISTER( &sk_barney_health1 );
	CVAR_REGISTER( &sk_barney_health2 );
	CVAR_REGISTER( &sk_barney_health3 );

	// Bullsquid
	CVAR_REGISTER( &sk_bullsquid_health1 );
	CVAR_REGISTER( &sk_bullsquid_health2 );
	CVAR_REGISTER( &sk_bullsquid_health3 );

	CVAR_REGISTER( &sk_bullsquid_dmg_bite1 );
	CVAR_REGISTER( &sk_bullsquid_dmg_bite2 );
	CVAR_REGISTER( &sk_bullsquid_dmg_bite3 );

	CVAR_REGISTER( &sk_bullsquid_dmg_whip1 );
	CVAR_REGISTER( &sk_bullsquid_dmg_whip2 );
	CVAR_REGISTER( &sk_bullsquid_dmg_whip3 );

	CVAR_REGISTER( &sk_bullsquid_dmg_spit1 );
	CVAR_REGISTER( &sk_bullsquid_dmg_spit2 );
	CVAR_REGISTER( &sk_bullsquid_dmg_spit3 );

	// Big Momma
	CVAR_REGISTER( &sk_bigmomma_health_factor1 );
	CVAR_REGISTER( &sk_bigmomma_health_factor2 );
	CVAR_REGISTER( &sk_bigmomma_health_factor3 );

	CVAR_REGISTER( &sk_bigmomma_dmg_slash1 );
	CVAR_REGISTER( &sk_bigmomma_dmg_slash2 );
	CVAR_REGISTER( &sk_bigmomma_dmg_slash3 );

	CVAR_REGISTER( &sk_bigmomma_dmg_blast1 );
	CVAR_REGISTER( &sk_bigmomma_dmg_blast2 );
	CVAR_REGISTER( &sk_bigmomma_dmg_blast3 );

	CVAR_REGISTER( &sk_bigmomma_radius_blast1 );
	CVAR_REGISTER( &sk_bigmomma_radius_blast2 );
	CVAR_REGISTER( &sk_bigmomma_radius_blast3 );

	// Gargantua
	CVAR_REGISTER( &sk_gargantua_health1 );
	CVAR_REGISTER( &sk_gargantua_health2 );
	CVAR_REGISTER( &sk_gargantua_health3 );

	CVAR_REGISTER( &sk_gargantua_dmg_slash1 );
	CVAR_REGISTER( &sk_gargantua_dmg_slash2 );
	CVAR_REGISTER( &sk_gargantua_dmg_slash3 );

	CVAR_REGISTER( &sk_gargantua_dmg_fire1 );
	CVAR_REGISTER( &sk_gargantua_dmg_fire2 );
	CVAR_REGISTER( &sk_gargantua_dmg_fire3 );

	CVAR_REGISTER( &sk_gargantua_dmg_stomp1 );
	CVAR_REGISTER( &sk_gargantua_dmg_stomp2 );
	CVAR_REGISTER( &sk_gargantua_dmg_stomp3	);

	// Hassassin
	CVAR_REGISTER( &sk_hassassin_health1 );
	CVAR_REGISTER( &sk_hassassin_health2 );
	CVAR_REGISTER( &sk_hassassin_health3 );

	// Headcrab
	CVAR_REGISTER( &sk_headcrab_health1 );
	CVAR_REGISTER( &sk_headcrab_health2 );
	CVAR_REGISTER( &sk_headcrab_health3 );

	CVAR_REGISTER( &sk_headcrab_dmg_bite1 );
	CVAR_REGISTER( &sk_headcrab_dmg_bite2 );
	CVAR_REGISTER( &sk_headcrab_dmg_bite3 );

	// Hgrunt
	CVAR_REGISTER( &sk_hgrunt_health1 );
	CVAR_REGISTER( &sk_hgrunt_health2 );
	CVAR_REGISTER( &sk_hgrunt_health3 );

	CVAR_REGISTER( &sk_hgrunt_kick1 );
	CVAR_REGISTER( &sk_hgrunt_kick2 );
	CVAR_REGISTER( &sk_hgrunt_kick3 );

	CVAR_REGISTER( &sk_hgrunt_pellets1 );
	CVAR_REGISTER( &sk_hgrunt_pellets2 );
	CVAR_REGISTER( &sk_hgrunt_pellets3 );

	CVAR_REGISTER( &sk_hgrunt_gspeed1 );
	CVAR_REGISTER( &sk_hgrunt_gspeed2 );
	CVAR_REGISTER( &sk_hgrunt_gspeed3 );

	// Houndeye
	CVAR_REGISTER( &sk_houndeye_health1 );
	CVAR_REGISTER( &sk_houndeye_health2 );
	CVAR_REGISTER( &sk_houndeye_health3 );

	CVAR_REGISTER( &sk_houndeye_dmg_blast1 );
	CVAR_REGISTER( &sk_houndeye_dmg_blast2 );
	CVAR_REGISTER( &sk_houndeye_dmg_blast3 );


	// ISlave
	CVAR_REGISTER( &sk_islave_health1 );
	CVAR_REGISTER( &sk_islave_health2 );
	CVAR_REGISTER( &sk_islave_health3 );

	CVAR_REGISTER( &sk_islave_dmg_claw1 );
	CVAR_REGISTER( &sk_islave_dmg_claw2 );
	CVAR_REGISTER( &sk_islave_dmg_claw3 );

	CVAR_REGISTER( &sk_islave_dmg_clawrake1	);
	CVAR_REGISTER( &sk_islave_dmg_clawrake2	);
	CVAR_REGISTER( &sk_islave_dmg_clawrake3	);

	CVAR_REGISTER( &sk_islave_dmg_zap1 );
	CVAR_REGISTER( &sk_islave_dmg_zap2 );
	CVAR_REGISTER( &sk_islave_dmg_zap3 );

	// Icthyosaur
	CVAR_REGISTER( &sk_ichthyosaur_health1 );
	CVAR_REGISTER( &sk_ichthyosaur_health2 );
	CVAR_REGISTER( &sk_ichthyosaur_health3 );

	CVAR_REGISTER( &sk_ichthyosaur_shake1 );
	CVAR_REGISTER( &sk_ichthyosaur_shake2 );
	CVAR_REGISTER( &sk_ichthyosaur_shake3 );

	// Leech
	CVAR_REGISTER( &sk_leech_health1 );
	CVAR_REGISTER( &sk_leech_health2 );
	CVAR_REGISTER( &sk_leech_health3 );

	CVAR_REGISTER( &sk_leech_dmg_bite1 );
	CVAR_REGISTER( &sk_leech_dmg_bite2 );
	CVAR_REGISTER( &sk_leech_dmg_bite3 );

	// Controller
	CVAR_REGISTER( &sk_controller_health1 );
	CVAR_REGISTER( &sk_controller_health2 );
	CVAR_REGISTER( &sk_controller_health3 );

	CVAR_REGISTER( &sk_controller_dmgzap1 );
	CVAR_REGISTER( &sk_controller_dmgzap2 );
	CVAR_REGISTER( &sk_controller_dmgzap3 );

	CVAR_REGISTER( &sk_controller_speedball1 );
	CVAR_REGISTER( &sk_controller_speedball2 );
	CVAR_REGISTER( &sk_controller_speedball3 );

	CVAR_REGISTER( &sk_controller_dmgball1 );
	CVAR_REGISTER( &sk_controller_dmgball2 );
	CVAR_REGISTER( &sk_controller_dmgball3 );

	// Nihilanth
	CVAR_REGISTER( &sk_nihilanth_health1 );
	CVAR_REGISTER( &sk_nihilanth_health2 );
	CVAR_REGISTER( &sk_nihilanth_health3 );

	CVAR_REGISTER( &sk_nihilanth_zap1 );
	CVAR_REGISTER( &sk_nihilanth_zap2 );
	CVAR_REGISTER( &sk_nihilanth_zap3 );

	// Scientist
	CVAR_REGISTER( &sk_scientist_health1 );
	CVAR_REGISTER( &sk_scientist_health2 );
	CVAR_REGISTER( &sk_scientist_health3 );

	// Snark
	CVAR_REGISTER( &sk_snark_health1 );
	CVAR_REGISTER( &sk_snark_health2 );
	CVAR_REGISTER( &sk_snark_health3 );

	CVAR_REGISTER( &sk_snark_dmg_bite1 );
	CVAR_REGISTER( &sk_snark_dmg_bite2 );
	CVAR_REGISTER( &sk_snark_dmg_bite3 );

	CVAR_REGISTER( &sk_snark_dmg_pop1 );
	CVAR_REGISTER( &sk_snark_dmg_pop2 );
	CVAR_REGISTER( &sk_snark_dmg_pop3 );

	// Zombie
	CVAR_REGISTER( &sk_zombie_health1 );
	CVAR_REGISTER( &sk_zombie_health2 );
	CVAR_REGISTER( &sk_zombie_health3 );

	CVAR_REGISTER( &sk_zombie_dmg_one_slash1 );
	CVAR_REGISTER( &sk_zombie_dmg_one_slash2 );
	CVAR_REGISTER( &sk_zombie_dmg_one_slash3 );

	CVAR_REGISTER( &sk_zombie_dmg_both_slash1 );
	CVAR_REGISTER( &sk_zombie_dmg_both_slash2 );
	CVAR_REGISTER( &sk_zombie_dmg_both_slash3 );

	// Turret
	CVAR_REGISTER( &sk_turret_health1 );
	CVAR_REGISTER( &sk_turret_health2 );
	CVAR_REGISTER( &sk_turret_health3 );


	// Mini turret
	CVAR_REGISTER( &sk_miniturret_health1 );
	CVAR_REGISTER( &sk_miniturret_health2 );
	CVAR_REGISTER( &sk_miniturret_health3 );

	// Sentry turret
	CVAR_REGISTER( &sk_sentry_health1 );
	CVAR_REGISTER( &sk_sentry_health2 );
	CVAR_REGISTER( &sk_sentry_health3 );

	//==================================================
	// PLAYER WEAPONS
	//==================================================

	// ========== MELEE AND ITEMS ==========
	// Knife whack (normal)
	CVAR_REGISTER( &sk_plr_knife1 );
	CVAR_REGISTER( &sk_plr_knife2 );
	CVAR_REGISTER( &sk_plr_knife3 );

	// Knife whack (stab)
	CVAR_REGISTER( &sk_plr_knife_stab1 );
	CVAR_REGISTER( &sk_plr_knife_stab2 );
	CVAR_REGISTER( &sk_plr_knife_stab3 );

	// Fists
	CVAR_REGISTER( &sk_plr_fists1 );
	CVAR_REGISTER( &sk_plr_fists2 );
	CVAR_REGISTER( &sk_plr_fists3 );

	// Semtex charge
	CVAR_REGISTER( &sk_plr_semtex1 );
	CVAR_REGISTER( &sk_plr_semtex2 );
	CVAR_REGISTER( &sk_plr_semtex3 );

	// ========== PISTOLS ==========
	// IMI Desert Eagle .50 Round
	CVAR_REGISTER( &sk_plr_deagle_bullet1 );
	CVAR_REGISTER( &sk_plr_deagle_bullet2 );
	CVAR_REGISTER( &sk_plr_deagle_bullet3 );

	// Colt M1911A2 45 ACP Round
	CVAR_REGISTER( &sk_plr_1911_bullet1 );
	CVAR_REGISTER( &sk_plr_1911_bullet2 );
	CVAR_REGISTER( &sk_plr_1911_bullet3 );

	// Beretta 92 FS 9mm Round
	CVAR_REGISTER( &sk_plr_ber92f_bullet1 );
	CVAR_REGISTER( &sk_plr_ber92f_bullet2 );
	CVAR_REGISTER( &sk_plr_ber92f_bullet3 );

	// Glock 18 9mm Round
	CVAR_REGISTER( &sk_plr_glock18_bullet1 );
	CVAR_REGISTER( &sk_plr_glock18_bullet2 );
	CVAR_REGISTER( &sk_plr_glock18_bullet3 );

	// Colt Anaconda (Python) .44 Round
	CVAR_REGISTER( &sk_plr_anaconda_bullet1 );
	CVAR_REGISTER( &sk_plr_anaconda_bullet2 );
	CVAR_REGISTER( &sk_plr_anaconda_bullet3 );

	// ========== SMGS ==========
	// H&K MP5 9mm Round
	CVAR_REGISTER( &sk_plr_mp5_bullet1 );
	CVAR_REGISTER( &sk_plr_mp5_bullet2 );
	CVAR_REGISTER( &sk_plr_mp5_bullet3 );

	// FN P90 .57 Round
	CVAR_REGISTER( &sk_plr_p90_bullet1 );
	CVAR_REGISTER( &sk_plr_p90_bullet2 );
	CVAR_REGISTER( &sk_plr_p90_bullet3 );

	// PP-19 Bizon 9mm Round
	CVAR_REGISTER( &sk_plr_bizon_bullet1 );
	CVAR_REGISTER( &sk_plr_bizon_bullet2 );
	CVAR_REGISTER( &sk_plr_bizon_bullet3 );

	// ========== RIFLES ==========
	// FAMAS G2 5.56 Round
	CVAR_REGISTER( &sk_plr_famas_bullet1 );
	CVAR_REGISTER( &sk_plr_famas_bullet2 );
	CVAR_REGISTER( &sk_plr_famas_bullet3 );

	// Colt M4A1 5.56 Round
	CVAR_REGISTER( &sk_plr_m4a1_bullet1 );
	CVAR_REGISTER( &sk_plr_m4a1_bullet2 );
	CVAR_REGISTER( &sk_plr_m4a1_bullet3 );

	// AK47 7.62 Round
	CVAR_REGISTER( &sk_plr_ak47_bullet1 );
	CVAR_REGISTER( &sk_plr_ak47_bullet2 );
	CVAR_REGISTER( &sk_plr_ak47_bullet3 );

	// M249 5.56 Round
	CVAR_REGISTER( &sk_plr_m249_bullet1 );
	CVAR_REGISTER( &sk_plr_m249_bullet2 );
	CVAR_REGISTER( &sk_plr_m249_bullet3 );

	// ========== SHOTGUNS ==========
	// SPAS 12 Round
	CVAR_REGISTER( &sk_plr_spas12_buckshot1 );
	CVAR_REGISTER( &sk_plr_spas12_buckshot2 );
	CVAR_REGISTER( &sk_plr_spas12_buckshot3 );

	// Less Lethal shotgun Round
	CVAR_REGISTER( &sk_plr_less_lethal_buckshot1 );
	CVAR_REGISTER( &sk_plr_less_lethal_buckshot2 );
	CVAR_REGISTER( &sk_plr_less_lethal_buckshot3 );

	// USAS-12 Round
	CVAR_REGISTER( &sk_plr_usas_buckshot1 );
	CVAR_REGISTER( &sk_plr_usas_buckshot2 );
	CVAR_REGISTER( &sk_plr_usas_buckshot3 );

	// ========== SNIPERS ==========
	// M14 7.62 Round
	CVAR_REGISTER( &sk_plr_m14_bullet1 );
	CVAR_REGISTER( &sk_plr_m14_bullet2 );
	CVAR_REGISTER( &sk_plr_m14_bullet3 );

	// L96A1 7.62 Round
	CVAR_REGISTER( &sk_plr_l96a1_bullet1 );
	CVAR_REGISTER( &sk_plr_l96a1_bullet2 );
	CVAR_REGISTER( &sk_plr_l96a1_bullet3 );

	// ========== EXPLOSIVES ==========
	// RPG-7
	CVAR_REGISTER( &sk_plr_rpg1 );
	CVAR_REGISTER( &sk_plr_rpg2 );
	CVAR_REGISTER( &sk_plr_rpg3 );

	// ========== HALF-LIFE BACKWARD COMPATIBILITY ==========
	// HL's M203 grenade
	CVAR_REGISTER( &sk_plr_9mmAR_grenade1 );
	CVAR_REGISTER( &sk_plr_9mmAR_grenade2 );
	CVAR_REGISTER( &sk_plr_9mmAR_grenade3 );

	// HL's Tripmine
	CVAR_REGISTER( &sk_plr_tripmine1 );
	CVAR_REGISTER( &sk_plr_tripmine2 );
	CVAR_REGISTER( &sk_plr_tripmine3 );

	//==================================================
	// MONSTERS WEAPONS
	//==================================================

	CVAR_REGISTER( &sk_12mm_bullet1 );
	CVAR_REGISTER( &sk_12mm_bullet2 );
	CVAR_REGISTER( &sk_12mm_bullet3 );

	CVAR_REGISTER( &sk_9mmAR_bullet1 );
	CVAR_REGISTER( &sk_9mmAR_bullet2 );
	CVAR_REGISTER( &sk_9mmAR_bullet3 );

	CVAR_REGISTER( &sk_9mm_bullet1 );
	CVAR_REGISTER( &sk_9mm_bullet2 );
	CVAR_REGISTER( &sk_9mm_bullet3 );

	CVAR_REGISTER( &sk_hornet_dmg1 );
	CVAR_REGISTER( &sk_hornet_dmg2 );
	CVAR_REGISTER( &sk_hornet_dmg3 );

	//==================================================
	// HEALTH AND ARMOR
	//==================================================

	// HL's HEV Recharger
	CVAR_REGISTER( &sk_suitcharger1 );
	CVAR_REGISTER( &sk_suitcharger2 );
	CVAR_REGISTER( &sk_suitcharger3 );

	// Kevlar vest
	CVAR_REGISTER( &sk_battery1 );
	CVAR_REGISTER( &sk_battery2 );
	CVAR_REGISTER( &sk_battery3 );

	// HL's Health Recharger
	CVAR_REGISTER( &sk_healthcharger1 );
	CVAR_REGISTER( &sk_healthcharger2 );
	CVAR_REGISTER( &sk_healthcharger3 );

	// Medkit
	CVAR_REGISTER( &sk_healthkit1 );
	CVAR_REGISTER( &sk_healthkit2 );
	CVAR_REGISTER( &sk_healthkit3 );

	// HL's Scientist syringe
	CVAR_REGISTER( &sk_scientist_heal1 );
	CVAR_REGISTER( &sk_scientist_heal2 );
	CVAR_REGISTER( &sk_scientist_heal3 );

	//==================================================
	// MONSTER DAMAGE MODIFIERS
	//==================================================

	CVAR_REGISTER( &sk_monster_head1 );
	CVAR_REGISTER( &sk_monster_head2 );
	CVAR_REGISTER( &sk_monster_head3 );

	CVAR_REGISTER( &sk_monster_chest1 );
	CVAR_REGISTER( &sk_monster_chest2 );
	CVAR_REGISTER( &sk_monster_chest3 );

	CVAR_REGISTER( &sk_monster_stomach1 );
	CVAR_REGISTER( &sk_monster_stomach2 );
	CVAR_REGISTER( &sk_monster_stomach3 );

	CVAR_REGISTER( &sk_monster_arm1 );
	CVAR_REGISTER( &sk_monster_arm2 );
	CVAR_REGISTER( &sk_monster_arm3 );

	CVAR_REGISTER( &sk_monster_leg1 );
	CVAR_REGISTER( &sk_monster_leg2 );
	CVAR_REGISTER( &sk_monster_leg3 );

	//==================================================
	// PLAYER DAMAGE MODIFIERS
	//==================================================

	CVAR_REGISTER( &sk_player_head1 );
	CVAR_REGISTER( &sk_player_head2 );
	CVAR_REGISTER( &sk_player_head3 );

	CVAR_REGISTER( &sk_player_chest1 );
	CVAR_REGISTER( &sk_player_chest2 );
	CVAR_REGISTER( &sk_player_chest3 );

	CVAR_REGISTER( &sk_player_stomach1 );
	CVAR_REGISTER( &sk_player_stomach2 );
	CVAR_REGISTER( &sk_player_stomach3 );

	CVAR_REGISTER( &sk_player_arm1 );
	CVAR_REGISTER( &sk_player_arm2 );
	CVAR_REGISTER( &sk_player_arm3 );

	CVAR_REGISTER( &sk_player_leg1 );
	CVAR_REGISTER( &sk_player_leg2 );
	CVAR_REGISTER( &sk_player_leg3 );
// END REGISTER CVARS FOR SKILL LEVEL STUFF

	// Shepard : Developers CVARs
#ifdef DEBUG
	CVAR_REGISTER( &am_infinite_stamina );
	CVAR_REGISTER( &am_wp_infinite_ammo );
	CVAR_REGISTER( &am_wp_debug );
	CVAR_REGISTER( &am_wp_spread_x_max );
	CVAR_REGISTER( &am_wp_spread_y_max );
	CVAR_REGISTER( &am_wp_spread_x_base );
	CVAR_REGISTER( &am_wp_spread_y_base );
	CVAR_REGISTER( &am_wp_spread_x_mod );
	CVAR_REGISTER( &am_wp_spread_y_mod );
	CVAR_REGISTER( &am_wp_spread_dir_change );
	CVAR_REGISTER( &am_wp_spread_drop );
	CVAR_REGISTER( &am_wp_shake_amount );
#endif

	SERVER_COMMAND("exec cfg/skill.cfg\n"); 
	SERVER_COMMAND("exec cfg/base.cfg\n");
}
