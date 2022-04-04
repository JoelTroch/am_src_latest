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
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "gamerules.h"

// ==================================================
// IMI Desert Eagle .50 ammo (ammo_deagle / ammo_50cal)
// ==================================================

class CDeagleAmmo : public CBasePlayerAmmo
{
	void Spawn( void )
	{
		Precache();
		SET_MODEL( ENT( pev ), "models/weapons/deagle/w_deagleclip.mdl" );
		CBasePlayerAmmo::Spawn();
	}
	void Precache( void )
	{
		PRECACHE_MODEL( "models/weapons/deagle/w_deagleclip.mdl" );
		PRECACHE_SOUND( "items/9mmclip1.wav" );
	}
	BOOL AddAmmo( CBaseEntity *pOther )
	{
		if ( pOther->GiveAmmo( 7, "50", _50_MAX_CARRY ) != -1 )
		{
			EMIT_SOUND_FMOD_3D( ENT( pev ), "items/9mmclip1.wav", 512.0f, -1.0f, pOther->entindex() );
			return TRUE;
		}
		return FALSE;
	}
};
LINK_ENTITY_TO_CLASS( ammo_deagleclip, CDeagleAmmo );

// ==================================================
// Colt M1911A2 45 ACP ammo (ammo_45cal)
// ==================================================

class C1911Ammo : public CBasePlayerAmmo
{
	void Spawn( void )
	{
		Precache();
		SET_MODEL( ENT( pev ), "models/weapons/1911/w_1911clip.mdl" );
		CBasePlayerAmmo::Spawn();
	}
	void Precache( void )
	{
		PRECACHE_MODEL( "models/weapons/1911/w_1911clip.mdl" );
		PRECACHE_SOUND( "items/9mmclip1.wav" );
	}
	BOOL AddAmmo( CBaseEntity *pOther )
	{
		if ( pOther->GiveAmmo( 7, "45", _9MM_MAX_CARRY ) != -1 )
		{
			EMIT_SOUND_FMOD_3D( ENT( pev ), "items/9mmclip1.wav", 512.0f, -1.0f, pOther->entindex() );
			return TRUE;
		}
		return FALSE;
	}
};
LINK_ENTITY_TO_CLASS( ammo_1911clip, C1911Ammo );

// ==================================================
// Beretta 92 FS 9mm ammo [Mag] (ammo_ber92f)
// ==================================================

class CBerettaAmmo : public CBasePlayerAmmo
{
	void Spawn( void )
	{
		Precache();
		SET_MODEL( ENT( pev ), "models/weapons/ber92f/w_ber92fclip.mdl" );
		CBasePlayerAmmo::Spawn();
	}
	void Precache( void )
	{
		PRECACHE_MODEL( "models/weapons/ber92f/w_ber92fclip.mdl" );
		PRECACHE_SOUND( "items/9mmclip1.wav" );
	}
	BOOL AddAmmo( CBaseEntity *pOther )
	{
		if ( pOther->GiveAmmo( 15, "9mm", _9MM_MAX_CARRY ) != -1 )
		{
			EMIT_SOUND_FMOD_3D( ENT( pev ), "items/9mmclip1.wav", 512.0f, -1.0f, pOther->entindex() );
			return TRUE;
		}
		return FALSE;
	}
};
LINK_ENTITY_TO_CLASS( ammo_ber92fclip, CBerettaAmmo );

// ==================================================
// Glock ammo (ammo_glockclip / ammo_9mmclip)
// ==================================================

class CGlockAmmo : public CBasePlayerAmmo
{
	void Spawn( void )
	{
		Precache();
		SET_MODEL( ENT( pev ), "models/weapons/glock18/w_glock18clip.mdl" );
		CBasePlayerAmmo::Spawn();
	}
	void Precache( void )
	{
		PRECACHE_MODEL( "models/weapons/glock18/w_glock18clip.mdl" );
		PRECACHE_SOUND( "items/9mmclip1.wav" );
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{
		if ( pOther->GiveAmmo( 17, "9mm", _9MM_MAX_CARRY ) != -1 )
		{
			EMIT_SOUND_FMOD_3D( ENT( pev ), "items/9mmclip1.wav", 512.0f, -1.0f, pOther->entindex() );
			return TRUE;
		}
		return FALSE;
	}
};
LINK_ENTITY_TO_CLASS( ammo_glock18clip, CGlockAmmo );
LINK_ENTITY_TO_CLASS( ammo_9mmclip, CGlockAmmo ); // Half-Life backward compatibility
LINK_ENTITY_TO_CLASS( ammo_glockclip, CGlockAmmo ); // Half-Life backward compatibility

// ==================================================
// Colt Anaconda (Python) .44 ammo [Box] (ammo_357)
// ==================================================

class CAnacondaAmmo : public CBasePlayerAmmo
{
	void Spawn( void )
	{
		Precache();
		SET_MODEL( ENT( pev ), "models/w_357ammobox.mdl" );
		CBasePlayerAmmo::Spawn();
	}
	void Precache( void )
	{
		PRECACHE_MODEL( "models/w_357ammobox.mdl" );
		PRECACHE_SOUND( "items/9mmclip1.wav" );
	}
	BOOL AddAmmo( CBaseEntity *pOther )
	{
		if ( pOther->GiveAmmo( 6, "44", _44_MAX_CARRY ) != -1 )
		{
			EMIT_SOUND_FMOD_3D( ENT( pev ), "items/9mmclip1.wav", 512.0f, -1.0f, pOther->entindex() );
			return TRUE;
		}
		return FALSE;
	}
};
LINK_ENTITY_TO_CLASS( ammo_44, CAnacondaAmmo );
LINK_ENTITY_TO_CLASS( ammo_357, CAnacondaAmmo ); // Half-Life backward compatibility

// ==================================================
// H&K MP5 9mm ammo [Mag] (ammo_9mmclip / ammo_9mmAR)
// ==================================================

class CMP5Ammo : public CBasePlayerAmmo
{
	void Spawn( void )
	{
		Precache();
		SET_MODEL( ENT( pev ), "models/weapons/mp5/w_mp5clip.mdl" );
		CBasePlayerAmmo::Spawn();
	}
	void Precache( void )
	{
		PRECACHE_MODEL( "models/weapons/mp5/w_mp5clip.mdl" );
		PRECACHE_SOUND( "items/9mmclip1.wav" );
	}
	BOOL AddAmmo( CBaseEntity *pOther )
	{
		if ( pOther->GiveAmmo( 30, "9mm", _9MM_MAX_CARRY ) != -1 )
		{
			EMIT_SOUND_FMOD_3D( ENT( pev ), "items/9mmclip1.wav", 512.0f, -1.0f, pOther->entindex() );
			return TRUE;
		}
		return FALSE;
	}
};
LINK_ENTITY_TO_CLASS( ammo_mp5clip, CMP5Ammo ); // Half-Life backward compatibility
LINK_ENTITY_TO_CLASS( ammo_9mmAR, CMP5Ammo ); // Half-Life backward compatibility

// ==================================================
// 9mm ammo [Box] (ammo_9mmbox)
// ==================================================

class C9mmAmmoBox : public CBasePlayerAmmo
{
	void Spawn( void )
	{
		Precache();
		SET_MODEL( ENT( pev ), "models/w_chainammo.mdl" );
		CBasePlayerAmmo::Spawn();
	}
	void Precache( void )
	{
		PRECACHE_MODEL( "models/w_chainammo.mdl" );
		PRECACHE_SOUND( "items/9mmclip1.wav" );
	}
	BOOL AddAmmo( CBaseEntity *pOther )
	{
		if ( pOther->GiveAmmo( _9MM_MAX_CARRY, "9mm", _9MM_MAX_CARRY ) != -1 )
		{
			EMIT_SOUND_FMOD_3D( ENT( pev ), "items/9mmclip1.wav", 512.0f, -1.0f, pOther->entindex() );
			return TRUE;
		}
		return FALSE;
	}
};
LINK_ENTITY_TO_CLASS( ammo_9mmbox, C9mmAmmoBox ); // Half-Life backward compatibility

// ==================================================
// FN P90 .57 ammo [Mag] (ammo_p90clip)
// ==================================================

class CP90Ammo : public CBasePlayerAmmo
{
	void Spawn( void )
	{
		Precache();
		SET_MODEL( ENT( pev ), "models/weapons/p90/w_p90clip.mdl" );
		CBasePlayerAmmo::Spawn();
	}
	void Precache( void )
	{
		PRECACHE_MODEL( "models/weapons/p90/w_p90clip.mdl" );
		PRECACHE_SOUND( "items/9mmclip1.wav" );
	}
	BOOL AddAmmo( CBaseEntity *pOther )
	{
		if ( pOther->GiveAmmo( 50, "57", _57_MAX_CARRY ) != -1 )
		{
			EMIT_SOUND_FMOD_3D( ENT( pev ), "items/9mmclip1.wav", 512.0f, -1.0f, pOther->entindex() );
			return TRUE;
		}
		return FALSE;
	}
};
LINK_ENTITY_TO_CLASS( ammo_p90clip, CP90Ammo );
LINK_ENTITY_TO_CLASS( ammo_ARgrenades, CP90Ammo ); // Half-Life backward compatibiltiy

// ==================================================
// FAMAS G2 5.56 ammo [Mag] (ammo_556)
// ==================================================

class CFAMASAmmo : public CBasePlayerAmmo
{
	void Spawn( void )
	{
		Precache();
		SET_MODEL( ENT( pev ), "models/weapons/famas/w_556ammo.mdl" );
		CBasePlayerAmmo::Spawn();
	}
	void Precache( void )
	{
		PRECACHE_MODEL( "models/weapons/famas/w_556ammo.mdl" );
		PRECACHE_SOUND( "items/9mmclip1.wav" );
	}
	BOOL AddAmmo( CBaseEntity *pOther )
	{
		if ( pOther->GiveAmmo( 25, "556", _556_MAX_CARRY ) != -1 )
		{
			EMIT_SOUND_FMOD_3D( ENT( pev ), "items/9mmclip1.wav", 512.0f, -1.0f, pOther->entindex() );
			return TRUE;
		}
		return FALSE;
	}
};
LINK_ENTITY_TO_CLASS( ammo_famasclip, CFAMASAmmo );
LINK_ENTITY_TO_CLASS( ammo_gaussclip, CFAMASAmmo ); // Half-Life backward compatibility

// ==================================================
// Colt M4A1 5.56 ammo [Mag] (ammo_m4a1clip)
// ==================================================

class CM4A1Ammo : public CBasePlayerAmmo
{
	void Spawn( void )
	{
		Precache();
		SET_MODEL( ENT( pev ), "models/weapons/m4a1/w_m4a1clip.mdl" );
		CBasePlayerAmmo::Spawn();
	}
	void Precache( void )
	{
		PRECACHE_MODEL( "models/weapons/m4a1/w_m4a1clip.mdl" );
		PRECACHE_SOUND( "items/9mmclip1.wav" );
	}
	BOOL AddAmmo( CBaseEntity *pOther )
	{
		if ( pOther->GiveAmmo( 30, "556", _556_MAX_CARRY ) != -1 )
		{
			EMIT_SOUND_FMOD_3D( ENT( pev ), "items/9mmclip1.wav", 512.0f, -1.0f, pOther->entindex() );
			return TRUE;
		}
		return FALSE;
	}
};
LINK_ENTITY_TO_CLASS( ammo_m4a1clip, CM4A1Ammo );

// ==================================================
// AK47 7.62 ammo [Mag] (ammo_ak47clip)
// ==================================================

class CAK47Ammo : public CBasePlayerAmmo
{
	void Spawn( void )
	{
		Precache();
		SET_MODEL( ENT( pev ), "models/weapons/ak47/w_ak47clip.mdl" );
		CBasePlayerAmmo::Spawn();
	}
	void Precache( void )
	{
		PRECACHE_MODEL( "models/weapons/ak47/w_ak47clip.mdl" );
		PRECACHE_SOUND( "items/9mmclip1.wav" );
	}
	BOOL AddAmmo( CBaseEntity *pOther )
	{
		if ( pOther->GiveAmmo( 30, "762", _762_MAX_CARRY ) != -1 )
		{
			EMIT_SOUND_FMOD_3D( ENT( pev ), "items/9mmclip1.wav", 512.0f, -1.0f, pOther->entindex() );
			return TRUE;
		}
		return FALSE;
	}
};
LINK_ENTITY_TO_CLASS( ammo_ak47clip, CAK47Ammo );
LINK_ENTITY_TO_CLASS( ammo_egonclip, CAK47Ammo ); // Half-Life backward compatibility


// ==================================================
// M249 5.56 ammo [Mag] (ammo_m249clip)
// ==================================================

class CM249Ammo : public CBasePlayerAmmo
{
	void Spawn( void )
	{
		Precache();
		SET_MODEL( ENT( pev ), "models/weapons/m249/w_m249clip.mdl" );
		CBasePlayerAmmo::Spawn();
	}
	void Precache( void )
	{
		PRECACHE_MODEL( "models/weapons/m249/w_m249clip.mdl" );
		PRECACHE_SOUND( "items/9mmclip1.wav" );
	}
	BOOL AddAmmo( CBaseEntity *pOther )
	{
		if ( pOther->GiveAmmo( 100, "556", _556_MAX_CARRY ) != -1 )
		{
			EMIT_SOUND_FMOD_3D( ENT( pev ), "items/9mmclip1.wav", 512.0f, -1.0f, pOther->entindex() );
			return TRUE;
		}
		return FALSE;
	}
};
LINK_ENTITY_TO_CLASS( ammo_m249clip, CM249Ammo );

// ==================================================
// SPAS 12 Gauge ammo [Box] (ammo_buckshot)
// ==================================================

class CSPAS12Ammo : public CBasePlayerAmmo
{
	void Spawn( void )
	{
		Precache();
		SET_MODEL( ENT( pev ), "models/weapons/w_buckshotbox.mdl" );
		CBasePlayerAmmo::Spawn();
	}
	void Precache( void )
	{
		PRECACHE_MODEL( "models/weapons/w_buckshotbox.mdl" );
		PRECACHE_SOUND( "items/9mmclip1.wav" );
	}
	BOOL AddAmmo( CBaseEntity *pOther )
	{
		if ( pOther->GiveAmmo( AMMO_BUCKSHOTBOX_GIVE, "buckshot", BUCKSHOT_MAX_CARRY ) != -1 )
		{
			EMIT_SOUND_FMOD_3D( ENT( pev ), "items/9mmclip1.wav", 512.0f, -1.0f, pOther->entindex() );
			return TRUE;
		}
		return FALSE;
	}
};
LINK_ENTITY_TO_CLASS( ammo_buckshot, CSPAS12Ammo ); // Half-Life backward compatibility

// ==================================================
// M14 7.62 ammo [Mag] (ammo_m14clip)
// ==================================================

class CM14Ammo : public CBasePlayerAmmo
{
	void Spawn( void )
	{
		Precache();
		SET_MODEL( ENT( pev ), "models/weapons/m14/w_m14clip.mdl" );
		CBasePlayerAmmo::Spawn();
	}
	void Precache( void )
	{
		PRECACHE_MODEL( "models/weapons/m14/w_m14clip.mdl" );
		PRECACHE_SOUND( "items/9mmclip1.wav" );
	}
	BOOL AddAmmo( CBaseEntity *pOther )
	{
		if ( pOther->GiveAmmo( 5, "762", _762_MAX_CARRY ) != -1 )
		{
			EMIT_SOUND_FMOD_3D( ENT( pev ), "items/9mmclip1.wav", 512.0f, -1.0f, pOther->entindex() );
			return TRUE;
		}
		return FALSE;
	}
};
LINK_ENTITY_TO_CLASS( ammo_m14clip, CM14Ammo );
LINK_ENTITY_TO_CLASS( ammo_crossbow, CM14Ammo ); // Half-Life backward compatibility

// ==================================================
// L96A1 7.62 ammo [Mag] (ammo_l96a1clip)
// ==================================================

class CL96A1Ammo : public CBasePlayerAmmo
{
	void Spawn( void )
	{
		Precache();
		SET_MODEL( ENT( pev ), "models/weapons/l96a1/w_l96a1clip.mdl" );
		CBasePlayerAmmo::Spawn();
	}
	void Precache( void )
	{
		PRECACHE_MODEL( "models/weapons/l96a1/w_l96a1clip.mdl" );
		PRECACHE_SOUND( "items/9mmclip1.wav" );
	}
	BOOL AddAmmo( CBaseEntity *pOther )
	{
		if ( pOther->GiveAmmo( 5, "762", _762_MAX_CARRY ) != -1 )
		{
			EMIT_SOUND_FMOD_3D( ENT( pev ), "items/9mmclip1.wav", 512.0f, -1.0f, pOther->entindex() );
			return TRUE;
		}
		return FALSE;
	}
};
LINK_ENTITY_TO_CLASS( ammo_l96a1clip, CL96A1Ammo );

// ==================================================
// RPG-7 ammo [Missile] (ammo_rpgclip)
// ==================================================

class CRPGAmmo : public CBasePlayerAmmo
{
	void Spawn( void )
	{
		Precache();
		SET_MODEL( ENT( pev ), "models/weapons/rpg/w_rpgammo.mdl" );
		CBasePlayerAmmo::Spawn();
	}
	void Precache( void )
	{
		PRECACHE_MODEL( "models/weapons/rpg/w_rpgammo.mdl" );
		PRECACHE_SOUND( "items/9mmclip1.wav" );
	}
	BOOL AddAmmo( CBaseEntity *pOther )
	{
		if ( pOther->GiveAmmo( 1, "rockets", ROCKET_MAX_CARRY ) != -1 )
		{
			EMIT_SOUND_FMOD_3D( ENT( pev ), "items/9mmclip1.wav", 512.0f, -1.0f, pOther->entindex() );
			return TRUE;
		}
		return FALSE;
	}
};
LINK_ENTITY_TO_CLASS( ammo_rpgclip, CRPGAmmo ); // Half-Life backward compatibility
