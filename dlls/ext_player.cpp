#include "extdll.h"
#include "util.h"

#include "cbase.h"
#include "player.h"
#include "trains.h"
#include "nodes.h"
#include "weapons.h"
#include "soundent.h"
#include "monsters.h"
#include "shake.h"
#include "decals.h"
#include "gamerules.h"
#include "game.h"
#include "pm_shared.h"
#include "hltv.h"

#include "lensFlareEnt.h"

int gmsgParticles = 0;
int gmsgAddRope = 0;
int gmsgSetBody = 0;
int gmsgSetSkin = 0;
int gmsgRadioIcon = 0;
int gmsgRecoil = 0;
int gmsgTbutton = 0;
int gmsgMenuWindow = 0;
int gmsgAllowWeaponSwitch = 0;

void CBasePlayer::MapReloaded(){
	if (gmsgSpecMsg != 0){
		if (_r != 0 && _g != 0 && _b != 0){
			MESSAGE_BEGIN(MSG_ONE, gmsgSpecMsg, NULL, pev);
			WRITE_BYTE(CL_CCORRECTION);
			WRITE_SHORT(_r);
			WRITE_SHORT(_g);
			WRITE_SHORT(_b);
			WRITE_SHORT(_s);
			WRITE_COORD(0.01);
			MESSAGE_END();
		}

		CBaseEntity *lightEnt = UTIL_FindEntityByClassname(0, "env_sun");
		if (lightEnt){
			CLensFlareEnt *lens = (CLensFlareEnt*)lightEnt;
			lens->sendMessage();
		}
		//ALERT(at_console, "!!!!SERVER!!!! Sent CC message\n");
	}
	else{
		//ALERT(at_console, "!!!!SERVER!!!! NO gmsgSpecMsg on CBasePlayer::Precache\n");
	}

	m_bReloaded = false;
}

void CBasePlayer::ResetPunch()
{
	pev->punchangle = Vector(0, 0, 0);
	pev->vuser3 = Vector(0, 0, 0);
}

// buz
void CBasePlayer::ViewPunch(float p, float y, float r)
{
	// vuser3 is punch speed
	pev->vuser3[0] -= p * 20;
	pev->vuser3[1] += y * 20;
	pev->vuser3[2] += r * 20;
}

void LoadAnotherMessages()
{
	gmsgMenuWindow = REG_USER_MSG("MenuWindow", -1);
	gmsgSpecMsg = REG_USER_MSG("SpecMsg", -1);
	gmsgParticles = REG_USER_MSG("Particles", -1);
	gmsgAddRope = REG_USER_MSG("AddRope", -1);
	gmsgSetBody = REG_USER_MSG("SetBody", 1);
	gmsgSetSkin = REG_USER_MSG("SetSkin", 1);
	gmsgRadioIcon = REG_USER_MSG("RadioIcon", -1); // buz
	gmsgRecoil = REG_USER_MSG("Recoil", -1);
	gmsgTbutton = REG_USER_MSG("Tbutton", -1);

	gmsgAllowWeaponSwitch = REG_USER_MSG( "AllowWepSw", 1 );
}

void CBasePlayer::UpdateEnvironment()
{
	//WARNING: THIS COULD BE HORRIBLE SLOW - UPDATE EACH X SECONDS
	CBaseEntity *pEntidad = NULL;
	while ((pEntidad = UTIL_FindEntityInSphere(pEntidad, pev->origin, 150)) != NULL)
	{
		if (FClassnameIs(pEntidad->pev, "func_door") || FClassnameIs(pEntidad->pev, "func_door_rotating"))
			b_PlayerIsNearOf = DOOR;
		else if (FClassnameIs(pEntidad->pev, "func_breakable"))
			b_PlayerIsNearOf = BREKABLE;
		else if (FClassnameIs(pEntidad->pev, "func_ladder"))
			b_PlayerIsNearOf = LADDER;
		else if (FClassnameIs(pEntidad->pev, "func_train"))
			b_PlayerIsNearOf = TRAIN;
		else if (FClassnameIs(pEntidad->pev, "func_plat"))
			b_PlayerIsNearOf = PLAT;
	}

}

void CBasePlayer::SetSpeed( float flNewSpeed )
{
	m_flPlayerSpeed = flNewSpeed;
}

void CBasePlayer::setColorCorrection(float r, float g, float b, float s, float t, bool fromEnt){
	if (!gmsgSpecMsg) return;

	MESSAGE_BEGIN(MSG_ONE, gmsgSpecMsg, NULL, pev);
	WRITE_BYTE(CL_CCORRECTION);
		WRITE_SHORT(r);
		WRITE_SHORT(g);
		WRITE_SHORT(b);
		WRITE_SHORT(s);
		WRITE_COORD(t);
	MESSAGE_END();

	if (fromEnt){
		_r = r;
		_g = g;
		_b = b;
		_s = s;
		_t = t;
	}
}

void CBasePlayer::postSpawn(){
	//fix up "No sprite set to glow"
	for (int i = 0; i < gpGlobals->maxEntities; i++)
	{
		CBaseEntity *ent = CBaseEntity::Instance(INDEXENT(i));
		if (!ent || !ent->pev)
			return;

		if (STRING(ent->pev->model)[0] == '*' && ent->pev->rendermode == kRenderGlow)
		{
			ent->pev->rendermode = kRenderNormal;
		}
	}

	//now update the sun.
	CBaseEntity *ent = UTIL_FindEntityByClassname(0, "env_sun");
	if (ent){
		CLensFlareEnt *lens = (CLensFlareEnt*)ent;
		lens->sendMessage();
	}

	//default automatic cubemap
	CPointEntity *pEntity = (CPointEntity *)UTIL_FindEntityByClassname(NULL, "env_cubemap");

	if (!pEntity)
	{
		CPointEntity *pCubemap = (CPointEntity *)CBaseEntity::Create("env_cubemap", pev->origin, pev->angles);
	}

	SERVER_COMMAND("developer 0\n");
}

//#define EPIPEN_HEAL_TIME 2.36f
#define EPIPEN_HEAL_TIME 4.06f

void CBasePlayer::EpipenPrepare( void )
{
	// We are already using an epipen or a grenade
	if ( m_flEpipenTime != -1.0f || m_flGrenadeTime != -1.0f )
		return;

	// We are dead or dying
	if ( pev->deadflag == DEAD_DEAD || pev->deadflag == DEAD_DEAD )
	{
		m_flEpipenTime = -1.0f;
		return;
	}

	// No more epipen
	if ( m_rgItems[ ITEM_EPIPEN ] <= 0 )
	{
		ClientPrint( pev, HUD_PRINTCENTER, "#Heal_NoMore" );
		return;
	}

	// Already at full health
	if ( pev->health >= pev->max_health )
	{
		ClientPrint( pev, HUD_PRINTCENTER, "#Heal_NoNeed" );
		return;
	}

	// Are we holding a weapon?
	if ( m_pActiveItem )
	{
		CBasePlayerWeapon *pWeaponPtr = (CBasePlayerWeapon *)m_pActiveItem;
		if ( pWeaponPtr )
		{
			// Are we performing an action with it (like firing/reloading)?
			if ( (pWeaponPtr->m_flNextPrimaryAttack > gpGlobals->time) || (pWeaponPtr->m_flNextSecondaryAttack > gpGlobals->time) ||
				 (pWeaponPtr->m_fInReload > 0) || (pWeaponPtr->m_fInSpecialReload > 0) )
				return;

			// Apply timings to the weapon
			pWeaponPtr->m_flNextPrimaryAttack = gpGlobals->time + EPIPEN_HEAL_TIME;
			pWeaponPtr->m_flNextSecondaryAttack = gpGlobals->time + EPIPEN_HEAL_TIME;
			pWeaponPtr->m_flTimeWeaponIdle = gpGlobals->time + EPIPEN_HEAL_TIME;
		}
	}

	// Change the model and play the sound
	ClientPrint( pev, HUD_PRINTCENTER, "#Heal_Fire" );
	EMIT_SOUND_FMOD_3D( ENT( pev ), "items/epipen_prepare.wav", 512.0f, -1.0f, entindex() );
	pev->viewmodel = MAKE_STRING( "models/v_epipen.mdl" );
	pev->weaponanim = 0;
	MESSAGE_BEGIN( MSG_ONE, SVC_WEAPONANIM, NULL, pev );
		WRITE_BYTE( 0 );
		WRITE_BYTE( 0 );
	MESSAGE_END();
	m_flEpipenTime = gpGlobals->time + EPIPEN_HEAL_TIME;
	pev->weaponmodel = MAKE_STRING( "models/p_epipen.mdl" );
	MESSAGE_BEGIN( MSG_ONE, gmsgAllowWeaponSwitch, NULL, pev );
		WRITE_BYTE( 0 );
	MESSAGE_END();
}

void CBasePlayer::EpipenThink( void )
{
	// Not the time yet to give the healing
	if ( (gpGlobals->time < m_flEpipenTime) || (m_flEpipenTime == -1.0f) )
		return;

	// We are dead or dying
	if ( pev->deadflag == DEAD_DEAD || pev->deadflag == DEAD_DEAD )
	{
		m_flEpipenTime = -1.0f;
		return;
	}

	// Are we holding a weapon?
	if ( m_pActiveItem )
	{
		CBasePlayerWeapon *pWeaponPtr = (CBasePlayerWeapon *)m_pActiveItem;
		if ( pWeaponPtr )
		{
			// Deploy it again
			pWeaponPtr->Deploy();
		}
	}

	// Remove a syringe from the inventory and play the sound
	TakeHealth( gSkillData.healthkitCapacity * 2, DMG_GENERIC );
	EMIT_SOUND_FMOD_3D( ENT( pev ), "items/epipen_heal.wav", 512.0f, -1.0f, entindex() );
	m_rgItems[ ITEM_EPIPEN ]--;
	ALERT( at_console, "=== You have %d / 5 Epipen syringes left ===\n", m_rgItems[ ITEM_EPIPEN ] ); // FIXME: Replace this by HUD
	MESSAGE_BEGIN( MSG_ONE, gmsgAllowWeaponSwitch, NULL, pev );
		WRITE_BYTE( 1 );
	MESSAGE_END();
	m_flEpipenTime = -1.0f;
}

#define GRENADE_PRIME_TIME 1.5f

void CBasePlayer::GrenadePrepare( void )
{
	// We are already using an epipen or a grenade
	if ( m_flEpipenTime != -1.0f || m_flGrenadeTime != -1.0f )
		return;

	// We are dead or dying
	if ( pev->deadflag == DEAD_DEAD || pev->deadflag == DEAD_DEAD )
	{
		m_flGrenadeTime = -1.0f;
		return;
	}

	// No more grenades
	if ( m_rgItems[ ITEM_GRENADE ] <= 0 )
	{
		ClientPrint( pev, HUD_PRINTCENTER, "#Grenade_NoMore" );
		return;
	}

	// Are we holding a weapon?
	if ( m_pActiveItem )
	{
		CBasePlayerWeapon *pWeaponPtr = (CBasePlayerWeapon *)m_pActiveItem;
		if ( pWeaponPtr )
		{
			// Are we performing an action with it (like firing/reloading)?
			if ( (pWeaponPtr->m_flNextPrimaryAttack > gpGlobals->time) || (pWeaponPtr->m_flNextSecondaryAttack > gpGlobals->time) ||
				 (pWeaponPtr->m_fInReload > 0) || (pWeaponPtr->m_fInSpecialReload > 0) )
				return;

			// Apply timings to the weapon
			pWeaponPtr->m_flNextPrimaryAttack = gpGlobals->time + GRENADE_PRIME_TIME;
			pWeaponPtr->m_flNextSecondaryAttack = gpGlobals->time + GRENADE_PRIME_TIME;
			pWeaponPtr->m_flTimeWeaponIdle = gpGlobals->time + GRENADE_PRIME_TIME;
		}
	}

	// Change the model and play the sound
	pev->viewmodel = MAKE_STRING( "models/v_hegrenade.mdl" );
	pev->weaponanim = 0;
	MESSAGE_BEGIN( MSG_ONE, SVC_WEAPONANIM, NULL, pev );
		WRITE_BYTE( 0 );
		WRITE_BYTE( 0 );
	MESSAGE_END();
	m_flGrenadeTime = gpGlobals->time + GRENADE_PRIME_TIME;
	pev->weaponmodel = MAKE_STRING( "models/p_hegrenade.mdl" );
	MESSAGE_BEGIN( MSG_ONE, gmsgAllowWeaponSwitch, NULL, pev );
		WRITE_BYTE( 0 );
	MESSAGE_END();
}

void CBasePlayer::GrenadeThink( void )
{
	// Not the time yet to give the healing
	if ( (gpGlobals->time < m_flGrenadeTime) || (m_flGrenadeTime == -1.0f) )
		return;

	// We are dead or dying
	if ( pev->deadflag == DEAD_DEAD || pev->deadflag == DEAD_DEAD )
	{
		m_flGrenadeTime = -1.0f;
		return;
	}

	// Are we holding a weapon?
	if ( m_pActiveItem )
	{
		CBasePlayerWeapon *pWeaponPtr = (CBasePlayerWeapon *)m_pActiveItem;
		if ( pWeaponPtr )
		{
			// Deploy it again
			pWeaponPtr->Deploy();
		}
	}

	// Remove a grenade from the inventory and throw it
	Vector vecAngThrow = pev->v_angle + pev->punchangle;
	if ( vecAngThrow.x < 0 )
		vecAngThrow.x = -10 + vecAngThrow.x * ( (90 - 10) / 90.0 );
	else
		vecAngThrow.x = -10 + vecAngThrow.x * ( (90 + 10) / 90.0 );

	float flVelocity = (90 - vecAngThrow.x) * 4;
	if ( flVelocity > 500 )
		flVelocity = 500;

	UTIL_MakeVectors( vecAngThrow );
	Vector vecSrc = pev->origin + pev->view_ofs + gpGlobals->v_forward * 10;
	Vector vecThrow = gpGlobals->v_forward * flVelocity + pev->velocity;
	CGrenade::ShootTimed( pev, vecSrc, vecThrow, 3.0f );

	m_rgItems[ ITEM_GRENADE ]--;
	ALERT( at_console, "=== You have %d / 5 grenades left ===\n", m_rgItems[ ITEM_GRENADE ]); // FIXME: Replace this by HUD
	MESSAGE_BEGIN( MSG_ONE, gmsgAllowWeaponSwitch, NULL, pev );
		WRITE_BYTE( 1 );
	MESSAGE_END();
	m_flGrenadeTime = -1.0f;
}

//==================================================
//==================================================
//==================================================

class CPlayerFreeze : public CBaseDelay
{
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
};

void CPlayerFreeze::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( !pActivator->IsPlayer() )
		return;

	CBaseEntity *pPlayer = CBaseEntity::Instance( pActivator->pev );
	if ( pPlayer && pPlayer->pev->flags & FL_CLIENT )
	{
		if ( pPlayer->pev->flags & FL_FROZEN )
			((CBasePlayer *)pPlayer)->EnableControl( TRUE ); // Unfreeze him
		else
			((CBasePlayer *)pPlayer)->EnableControl( FALSE ); // Freeze him
	}
}
LINK_ENTITY_TO_CLASS( trigger_playerfreeze, CPlayerFreeze );
