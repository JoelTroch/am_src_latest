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
#include "shake.h"
#include "soundent.h"

TYPEDESCRIPTION CBaseAMWeapon::m_SaveData[] =
{
	DEFINE_FIELD( CBaseAMWeapon, installedMods, FIELD_INTEGER ),
	DEFINE_FIELD( CBaseAMWeapon, m_flNextReload, FIELD_TIME ),
	DEFINE_FIELD( CBaseAMWeapon, m_fInSpecialReload, FIELD_INTEGER )
};
IMPLEMENT_SAVERESTORE( CBaseAMWeapon, CBasePlayerWeapon );

float holdTime = 0;
bool holding = false;
void CBaseAMWeapon::SecondaryAttack( void )
{
	
 

	

	return;

	if (holding)
		return;

	if ( !AllowIronSight() || !m_pPlayer )
		return;
	ALERT(at_console, "SecondaryAttack\n");
	int fov = 90;
	if ( m_pPlayer->m_iFOV == 60 && (GetAvailableMods() & MOD_SCOPE) )
		fov = 45;
	else if (m_pPlayer->m_iFOV == 90)
	{
		fov = 60;
	}

	if ( ModInstalled( MOD_REDDOT ) || (GetAvailableMods() & MOD_SCOPE) )
	{
		UTIL_ScreenFade( m_pPlayer, Vector( 0, 0, 0 ), 1.0, 0.1, 255, FFADE_IN );
		m_pPlayer->pev->viewmodel = MAKE_STRING( (fov == 90) ? GetVModel() : GetScopeModel() );
		EMIT_SOUND_FMOD_3D( ENT( m_pPlayer->pev ), "weapons/zoom.wav", 512.0f, -1.0f, m_pPlayer->entindex() );
	}

	m_pPlayer->m_iFOV = fov;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3;
}

int CBaseAMWeapon::DefaultDeploy( int skiplocal, int body )
{
	ItemPreFrame();//SYS: this forces an update

	holdTime = 0;
	holding = FALSE;
	int r;
	if ( m_iClip == 0 )
		r = CBasePlayerWeapon::DefaultDeploy( GetVModel(), GetPModel(), DeployEmptyAnimation(), GetExtAnim(), skiplocal, body );
	else
	{
		int iAnim = DeployAnimation();
		if ( DeployVariations() > 0 )
			iAnim += RANDOM_LONG( 0, DeployVariations() - 1 );

		r = CBasePlayerWeapon::DefaultDeploy( GetVModel(), GetPModel(), iAnim, GetExtAnim(), skiplocal, body );
	}

	if ( r )
		bulletsFired = 0;

	float flTime = DeployTime();
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + flTime;
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = flTime;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + flTime + 0.5;
	EMIT_SOUND_FMOD_3D( ENT( m_pPlayer->pev ), GetDeploySound(), 512.0f, -1.0f, m_pPlayer->entindex() );
	return r;
}

int CBaseAMWeapon::DefaultReload( int clip, int anim, float time, int body )
{
	int r = CBasePlayerWeapon::DefaultReload( clip, anim, time, body );
	if ( r )
	{
		m_iClip = 0;
		bulletsFired = 0;
	}

	return r;
}

void CBaseAMWeapon::ItemPostFrame( void )
{
 
	if (m_pPlayer->pev->button & IN_ATTACK2)
	{
		holdTime += gpGlobals->frametime * 200;
		if (holdTime > 1)
		{
		//	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 9999;
		//	holding = TRUE;
			 



			m_pPlayer->m_iFOV = 60;

		//	ALERT(at_console, "holding iron\n");
		}
	}
	else
	{
		if (holdTime > 1)
		{

			m_pPlayer->m_iFOV = 90;
		//	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5;
		//	holding = FALSE;
		//	SecondaryAttack();
		 //	m_pPlayer->pev->button &= ~IN_ATTACK2;
			holdTime = 0;
	//		ALERT(at_console, "restored aim\n");
		}
		
	}
	CBasePlayerWeapon::ItemPostFrame();
//	ALERT(at_console, "holdtime %f - m_flNextSecondaryAttack %f\n", holdTime, m_flNextSecondaryAttack);


	if ( bulletsFired < 0.0f )
		bulletsFired = 0.0f;
	if ( bulletsFired > 30.0f )
		bulletsFired = 30.0f;

	if ( nextRecoilDecrease <= gpGlobals->time )
	{
#ifdef DEBUG
		if ( CVAR_GET_FLOAT( "am_wp_debug" ) == 1 )
			nextRecoilDecrease = gpGlobals->time + CVAR_GET_FLOAT( "am_wp_spread_drop" );
		else
#endif
			nextRecoilDecrease = gpGlobals->time + DropRecoilDelay();
	//	nextRecoilDecrease = gpGlobals->time + 0.01;

		bulletsFired--;
	}
//	ALERT(at_console, "bulletsFired %f  \n", bulletsFired);

}

BOOL CBaseAMWeapon::Deploy( void )
{
	return DefaultDeploy();
}

void CBaseAMWeapon::Reload( void )
{
	// Don't reload until recoil is done
	if ( m_flNextPrimaryAttack > UTIL_WeaponTimeBase() )
		return;

	if ( ReloadAsPumpShotgun() )
	{
		// No more ammo or no more buckshots to load ? Then don't
		if ( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 || m_iClip == GetMaxClipAmmo() )
			return;

		// Start the reload
		if ( m_fInSpecialReload == 0 )
		{
			SendWeaponAnim( ReloadAnimation() );
			m_fInSpecialReload = 1;
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + ReloadShotgunStartTime();
		}
		else if ( m_fInSpecialReload == 1 ) // I'm loading a buckshot...
		{
			// Wait for the weapon to move to the side
			if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
				return;

			SendWeaponAnim( ReloadAnimation() + 1 );
			m_fInSpecialReload = 2;
			m_flNextReload = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + ReloadTime();
		}
		else // Loaded !
		{
			m_iClip++;
			m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
			m_fInSpecialReload = 1;
		}
	}
	else
	{
		// Red dot/scope mod is installed and we are using it, return back to normal
		if ( (ModInstalled( MOD_REDDOT ) || (GetAvailableMods() & MOD_SCOPE)) && m_pPlayer->m_iFOV != 90 && m_iClip != GetMaxClipAmmo() &&
			m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] )
		{
			UTIL_ScreenFade( m_pPlayer, Vector( 0, 0, 0 ), 1.0, 0.1, 255, FFADE_IN );
			m_pPlayer->pev->viewmodel = MAKE_STRING( GetVModel() );
			m_pPlayer->m_iFOV = 90;
			EMIT_SOUND_FMOD_3D( ENT( m_pPlayer->pev ), "weapons/zoom.wav", 512.0f, -1.0f, m_pPlayer->entindex() );
		}

		if ( m_iClip == 0 )
			DefaultReload( GetMaxClipAmmo(), ReloadEmptyAnimation(), ReloadEmptyTime() );
		else
			DefaultReload( GetMaxClipAmmo(), ReloadAnimation(), ReloadTime() );
	}
}

int CBaseAMWeapon::AddToPlayer( CBasePlayer *pPlayer )
{
	if ( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev );
			WRITE_BYTE( WeaponID() );
		MESSAGE_END();
		// If we are picking up the Glock 18, then install the semi-auto mode as default
		if ( WeaponID() == WEAPON_GLOCK )
			InstallMod( MOD_SEMIAUTO );

		return TRUE;
	}
	return FALSE;
}

void CBaseAMWeapon::Precache( void )
{
	// P, V, W models and deploy sound, this is common for all weapons/items
	PRECACHE_MODEL( GetVModel() );
	PRECACHE_MODEL( GetWModel() );
	PRECACHE_MODEL( GetPModel() );
	PRECACHE_SOUND( GetDeploySound() );
	m_iShell = PRECACHE_MODEL( GetShellModel() );
	m_usWeaponShell = PRECACHE_EVENT( 1, "events/weapon_shell.sc" );

	// Weapon/item has a red dot/scope model
	if ( GetAvailableMods() != MOD_NOMODS && ((GetAvailableMods() & MOD_REDDOT) || (GetAvailableMods() & MOD_SCOPE)) && GetScopeModel() != NULL )
	{
		PRECACHE_MODEL( GetScopeModel() );
		PRECACHE_SOUND( "weapons/zoom.wav" );
	}

	// Weapon/item has a "firing" sound
	if ( GetFireSound() != "" )
		PRECACHE_SOUND( GetFireSound() );

	// Weapon/item can be silenced, precache it's sound
	if ( GetAvailableMods() != MOD_NOMODS && (GetAvailableMods() & MOD_SILENCER) && GetFireSound() != GetSilencedFireSound() )
		PRECACHE_SOUND( GetSilencedFireSound() );

	// Weapon/item can have a burst attack, precache it's 2 shots sound
	if ( GetAvailableMods() != MOD_NOMODS && (GetAvailableMods() & MOD_BURSTATTACK) && GetFire2Sound() != GetFireSound() )
		PRECACHE_SOUND( GetFire2Sound() );

	// Weapon/item can have a burst attack, precache it's 3 shots sound
	if ( GetAvailableMods() != MOD_NOMODS && (GetAvailableMods() & MOD_BURSTATTACK) && GetFire3Sound() != GetFireSound() )
		PRECACHE_SOUND( GetFire3Sound() );

	// Weapon/item can be silenced and have a burst attack, precache it's 2 silenced shots sound
	if ( GetAvailableMods() != MOD_NOMODS && (GetAvailableMods() & MOD_BURSTATTACK) && (GetAvailableMods() & MOD_SILENCER) && GetSilencedFire2Sound() != GetSilencedFireSound() )
		PRECACHE_SOUND( GetSilencedFire2Sound() );

	// Weapon/item can be silenced and have a burst attack, precache it's 3 silenced shots sound
	if ( GetAvailableMods() != MOD_NOMODS && (GetAvailableMods() & MOD_BURSTATTACK) && (GetAvailableMods() & MOD_SILENCER) && GetSilencedFire3Sound() != GetSilencedFireSound() )
		PRECACHE_SOUND( GetSilencedFire3Sound() );
}

void CBaseAMWeapon::PrimaryAttack( void )
{
	if ( SemiAutoModeOnly() || ModInstalled( MOD_SEMIAUTO ) )
	{
		if ( !(m_pPlayer->m_afButtonPressed & IN_ATTACK) )
			return;
	}

	if ( ReloadAsPumpShotgun() && m_fInSpecialReload != 0 ) // For pump shotguns, this makes a proper "stop reloading" logic
	{
		SendWeaponAnim( ReloadAnimation() + 2 );
		m_fInReload = 0;
		m_fInSpecialReload = 0;
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + ReloadShotgunEndTime();
		return;
	}

	if ( m_pPlayer->pev->waterlevel == 3 && !FireUnderWater() )
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15;
		return;
	}

	if ( m_iClip <= 0 )
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5; // 0.15
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
		SendWeaponAnim( FireEmptyAnimation() );
		ResetEmptySound();
		return;
	}

	int iBulletsToFire = 1;
	if ( FireAsShotgun() ) // We are using a shotgun, so fire all the pellets but just remove only one buckshot
	{
		iBulletsToFire = PelletsToFire();
#ifdef DEBUG
		if ( CVAR_GET_FLOAT( "am_wp_infinite_ammo" ) != 1 )
#endif
			m_iClip--;
	}
	else // We are not using a shotgun, normal behavior
	{
		if ( ModInstalled( MOD_BURSTATTACK ) && m_iClip > 2 )
			iBulletsToFire = 3;
		else if ( ModInstalled( MOD_BURSTATTACK ) && m_iClip == 2 )
			iBulletsToFire = 2;

#ifdef DEBUG
		if ( CVAR_GET_FLOAT( "am_wp_infinite_ammo" ) != 1 )
#endif
			m_iClip -= iBulletsToFire;
	}

	// AI stuff
	m_pPlayer->m_iWeaponVolume = IsSilenced() ? QUIET_GUN_VOLUME : GunVolume();
	m_pPlayer->m_iWeaponFlash = IsSilenced() ? DIM_GUN_FLASH : GunFlash();

	bulletsFired += iBulletsToFire;

	if ( !ModInstalled( MOD_SILENCER ) )
		m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecAiming = m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );
	Vector vecDir;

	Vector spread = WeaponSpread();
	if ( IsSilenced() )
	{
		spread.x *= silencerSpreadModifier();
		spread.y *= silencerSpreadModifier();
		spread.z *= silencerSpreadModifier();
	}

	EMIT_SOUND_FMOD_3D(ENT(m_pPlayer->pev), "weapons/bass.wav", 512.0f, -1.0f, m_pPlayer->entindex(), 1.0f + UTIL_SharedRandomFloat(m_pPlayer->random_seed, -0.2f, 0.5f));

/*	switch (RANDOM_LONG(0, 1))
	{
	case 0: 	EMIT_SOUND_FMOD_3D(ENT(m_pPlayer->pev), "weapons/bass1.wav", 512.0f, -1.0f, m_pPlayer->entindex(), 1.0f + UTIL_SharedRandomFloat(m_pPlayer->random_seed, -0.2f, 0.5f)); break;
	case 1: 	EMIT_SOUND_FMOD_3D(ENT(m_pPlayer->pev), "weapons/bass2.wav", 512.0f, -1.0f, m_pPlayer->entindex(), 1.0f + UTIL_SharedRandomFloat(m_pPlayer->random_seed, -0.2f, 0.5f)); break;
	}*/


	punchScreen();
#ifdef DEBUG
	float flShakeAmount = (CVAR_GET_FLOAT( "am_wp_debug" ) == 1) ? CVAR_GET_FLOAT( "am_wp_shake_amount" ) : ShakeAmount();
	UTIL_ScreenShake( m_pPlayer->pev->origin, 2, flShakeAmount, 0.3, 1.0 );
#else
	UTIL_ScreenShake( m_pPlayer->pev->origin, 2, ShakeAmount(), 0.3, 1.0 );
#endif
	if (m_pPlayer->m_iFOV == 60 && WeaponID() != WEAPON_SPAS12 &&  WeaponID() != WEAPON_LESSLETHAL)
		vecDir = m_pPlayer->FireBulletsPlayer(iBulletsToFire, vecSrc, vecAiming, Vector(0, 0, 0), 8192, BulletType(), 2, 0, m_pPlayer->pev, m_pPlayer->random_seed);
	else
		vecDir = m_pPlayer->FireBulletsPlayer(iBulletsToFire, vecSrc, vecAiming, WeaponSpread(), 8192, BulletType(), 2, 0, m_pPlayer->pev, m_pPlayer->random_seed);

	int iAnim = (m_iClip == 0) ? FireLastAnimation() : FireAnimation() + RANDOM_LONG( 0, (FireVariations() - 1) );
	if ( iBulletsToFire > 1 && !FireAsShotgun() )
		iAnim = FireAnimation() + FireVariations() + (iBulletsToFire - 1);
	else if ( m_pPlayer->m_iFOV != 90 && (ModInstalled( MOD_REDDOT ) || (GetAvailableMods() & MOD_SCOPE)) )
		iAnim = 1;

	SendWeaponAnim(iAnim);

	char *cSoundToUse = GetFireSound();
	if ( IsSilenced() )
	{
		switch ( iBulletsToFire )
		{
		case 1: cSoundToUse = GetSilencedFireSound(); break;
		case 2: cSoundToUse = GetSilencedFire2Sound(); break;
		case 3: cSoundToUse = GetSilencedFire3Sound(); break;
		}
	}
	else
	{
		switch ( iBulletsToFire )
		{
		case 2: cSoundToUse = GetFire2Sound(); break;
		case 3: cSoundToUse = GetFire3Sound(); break;
		}

		CSoundEnt::InsertSound(bits_SOUND_COMBAT, Classify(), pev->origin, 2048, 1);
	}

	// Only the Colt Anaconda .357 don't eject shells when firing
	if ( WeaponID() != WEAPON_ANACONDA )
		PLAYBACK_EVENT_FULL( 0, m_pPlayer->edict(), m_usWeaponShell, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, 0.0, 0.0, WeaponID(), 0, 0, 0 );

	float flPitch = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 0.8f, 1.5f );
	if ( CVAR_GET_FLOAT( "host_framerate" ) != 0 )
		flPitch = 0.4f;

	EMIT_SOUND_FMOD_3D( ENT( m_pPlayer->pev ), cSoundToUse, IsSilenced() ? QUIET_GUN_VOLUME : GunVolume(), IsSilenced() ? (QUIET_GUN_VOLUME * 1.25) : (GunVolume() * 1.5), m_pPlayer->entindex(), flPitch );

	CBaseEntity *pWeaponTailZone = NULL;
	while ( (pWeaponTailZone = UTIL_FindEntityInSphere( pWeaponTailZone, m_pPlayer->pev->origin, 1.0f )) != NULL )
	{
		if ( FClassnameIs( pWeaponTailZone->pev, "func_weapon_tail_zone" ) )
		{
			switch ( RANDOM_LONG( 0, 3 ) )
			{
			case 0:
				EMIT_SOUND_FMOD_3D( ENT( m_pPlayer->pev ), "weapons/tail-1.wav", IsSilenced() ? QUIET_GUN_VOLUME : GunVolume(), IsSilenced() ? (QUIET_GUN_VOLUME * 1.25) : (GunVolume() * 1.5), m_pPlayer->entindex(), flPitch );
				break;
			case 1:
				EMIT_SOUND_FMOD_3D( ENT( m_pPlayer->pev ), "weapons/tail-2.wav", IsSilenced() ? QUIET_GUN_VOLUME : GunVolume(), IsSilenced() ? (QUIET_GUN_VOLUME * 1.25) : (GunVolume() * 1.5), m_pPlayer->entindex(), flPitch );
				break;
			case 2:
				EMIT_SOUND_FMOD_3D( ENT( m_pPlayer->pev ), "weapons/tail-3.wav", IsSilenced() ? QUIET_GUN_VOLUME : GunVolume(), IsSilenced() ? (QUIET_GUN_VOLUME * 1.25) : (GunVolume() * 1.5), m_pPlayer->entindex(), flPitch );
				break;
			case 3:
				EMIT_SOUND_FMOD_3D( ENT( m_pPlayer->pev ), "weapons/tail-4.wav", IsSilenced() ? QUIET_GUN_VOLUME : GunVolume(), IsSilenced() ? (QUIET_GUN_VOLUME * 1.25) : (GunVolume() * 1.5), m_pPlayer->entindex(), flPitch );
				break;
			}
			break;
		}
	}

	float flAttackDelay = (ModInstalled( MOD_BURSTATTACK )) ? (AttackDelay() * 3) + 0.25 : AttackDelay();
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + flAttackDelay;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
}

void CBaseAMWeapon::Spawn( void )
{
	pev->classname = MAKE_STRING( WeaponClassName() ); // hack to allow for old names
	Precache();
	SET_MODEL( ENT( pev ), GetWModel() );
	m_iId = WeaponID();

	if ( WeaponType() == TYPE_ITEM_AMMO )
		m_iDefaultAmmo = 1;
	else
	{
		if (pev->armorvalue)
		{
			if ((pev->armorvalue > GetMaxClipAmmo()) && (pev->armorvalue < 0))
				pev->armorvalue = GetMaxClipAmmo();

			m_iDefaultAmmo = pev->armorvalue;
		}
		else m_iDefaultAmmo = GetMaxClipAmmo();
	}

	FallInit();// get ready to fall down.
}

int CBaseAMWeapon::GetItemInfo( ItemInfo *p )
{
	p->pszName = STRING( pev->classname );
	p->pszAmmo1 = GetAmmoName();
	p->iMaxAmmo1 = GetMaxCarry();
	p->iMaxClip = GetMaxClipAmmo();
	p->iSlot = iItemSlot() - 1;
	p->iPosition = WeaponPosition();
	if ( WeaponType() == TYPE_ITEM_AMMO )
		p->iFlags = ITEM_FLAG_LIMITINWORLD | ITEM_FLAG_EXHAUSTIBLE | ITEM_FLAG_SELECTONEMPTY;
	else if ( WeaponType() == TYPE_ITEM_INFINITE )
		p->iFlags = ITEM_FLAG_SELECTONEMPTY;
	else
		p->iFlags = ITEM_FLAG_NOAUTORELOAD;

	p->iId = m_iId = WeaponID();
	p->iWeight = WeaponWeight();
	return 1;
}

void CBaseAMWeapon::WeaponIdle( void )
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );
	if ( ReloadAsPumpShotgun() )
	{
		if ( m_flTimeWeaponIdle < UTIL_WeaponTimeBase() )
		{
			if ( m_fInSpecialReload != 0 ) // Reloading !
			{
				if ( m_iClip != GetMaxClipAmmo() && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] ) // Insert another buckshot if we can
					Reload();
				else // Otherwise, stop here
				{
					SendWeaponAnim( ReloadAnimation() + 2 );
					m_fInSpecialReload = 0;
					m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + ReloadShotgunEndTime();
				}
			}
			else // Not reloading ? Go idle !
			{
				PlayIdleAnimation();
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
			}
		}
	}
	else
	{
		if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
			return;

		if (m_pPlayer->m_iFOV != 60)
		PlayIdleAnimation();

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
	}
}

void CBaseAMWeapon::PlayIdleAnimation( void )
{
	int iAnim = IdleAnimation();
	if ( m_iClip == 0 )
	{
		iAnim = IdleEmptyAnimation();
		if ( IdleEmptyVariations() != 0 && IdleEmptyAnimation() != -1 )
			iAnim += RANDOM_LONG( 0, IdleEmptyVariations() );
	}
	else
	{
		if ( IdleVariations() != 0 )
			iAnim += RANDOM_LONG( 0, IdleVariations() );
	}
	if ( iAnim != -1 )
		SendWeaponAnim( iAnim );
}

void CBaseAMWeapon::Holster( int skiplocal )
{
	m_fInReload = FALSE;
	m_fInSpecialReload = 0; // The HL's Shotgun reloading trick won't work this time ^^

	// Red dot/scope mod is installed and we are using it, return back to normal
	if ( (ModInstalled( MOD_REDDOT ) || (GetAvailableMods() & MOD_SCOPE)) && m_pPlayer->m_iFOV != 90 )
	{
		UTIL_ScreenFade( m_pPlayer, Vector( 0, 0, 0 ), 1.0, 0.1, 255, FFADE_IN );
		m_pPlayer->m_iFOV = 90;
		EMIT_SOUND_FMOD_3D( ENT( m_pPlayer->pev ), "weapons/zoom.wav", 512.0f, -1.0f, m_pPlayer->entindex() );
	}

	if ( WeaponType() == TYPE_ITEM_AMMO )
	{
		if ( !m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] )
		{
			m_pPlayer->pev->weapons &= ~(1 << m_iId);
			SetThink( &CBasePlayerWeapon::DestroyItem );
			pev->nextthink = gpGlobals->time + 0.1;
		}
	}
}

int CBaseAMWeapon::GetBody( void )
{
	if ( installedMods == MOD_NOMODS )
		return 0;
	if ( ModInstalled( MOD_SILENCER ) && !ModInstalled( MOD_REDDOT ) )
		return 1;
	if ( !ModInstalled( MOD_SILENCER ) && ModInstalled( MOD_REDDOT ) )
		return 2;
	if ( ModInstalled( MOD_SILENCER ) && ModInstalled( MOD_REDDOT ) )
		return 3;
}

int CBaseAMWeapon :: InstallMod( int mod )
{
	// Avoid memory leak here
	if ( m_pPlayer == NULL )
		return installedMods;

	// Don't install a mod if we are firing/reloading or whatever
	if ( m_fInReload || m_flNextPrimaryAttack > UTIL_WeaponTimeBase() || m_flNextSecondaryAttack > UTIL_WeaponTimeBase() || m_pPlayer->m_flNextAttack > UTIL_WeaponTimeBase() )
		return installedMods;

	if ( !( GetAvailableMods() & mod ) )
		return installedMods;

	// Don't have burst and semi modes at the same time, just replace the newer one by the older one
	if ( (GetAvailableMods() & MOD_BURSTATTACK) && (GetAvailableMods() & MOD_SEMIAUTO) && ModInstalled( MOD_BURSTATTACK ) && mod == MOD_SEMIAUTO )
		installedMods ^= MOD_BURSTATTACK;
	else if ( (GetAvailableMods() & MOD_BURSTATTACK) && (GetAvailableMods() & MOD_SEMIAUTO) && ModInstalled( MOD_SEMIAUTO ) && mod == MOD_BURSTATTACK )
		installedMods ^= MOD_SEMIAUTO;

	// Don't install the reddot mod if we are aiming
	if ( m_pPlayer->m_iFOV != 90 && (mod & MOD_REDDOT) )
		return installedMods;

	installedMods ^= mod;
	int bd = GetBody();
	SetBody( bd );
	ALERT( at_console, "InstallBody %i and mod %i\n", bd, mod );
	return installedMods;
}

void CBaseAMWeapon::SetBody( int body )
{
	pev->body = body;
	MESSAGE_BEGIN( MSG_ONE, gmsgSetBody, NULL, m_pPlayer->pev );
		WRITE_BYTE( pev->body );
	MESSAGE_END();
}

void CBaseAMWeapon::SetSkin( int skin )
{
	pev->skin = skin;
	MESSAGE_BEGIN( MSG_ONE, gmsgSetSkin, NULL, m_pPlayer->pev );
		WRITE_BYTE( pev->skin );
	MESSAGE_END();
}

extern int gmsgRecoil;

void CBaseAMWeapon::punchScreen(void)
{
	Vector punch;
#ifdef DEBUG
	if ( CVAR_GET_FLOAT( "am_wp_debug" ) == 1 )
	{
		float flSpreadXBase = CVAR_GET_FLOAT( "am_wp_spread_x_base" );
		float flSpreadXMod = CVAR_GET_FLOAT( "am_wp_spread_x_mod" );
		float flSpreadXMax = CVAR_GET_FLOAT( "am_wp_spread_x_max" );
		float flSpreadYBase = CVAR_GET_FLOAT( "am_wp_spread_y_base" );
		float flSpreadYMod = CVAR_GET_FLOAT( "am_wp_spread_y_mod" );
		float flSpreadYMax = CVAR_GET_FLOAT( "am_wp_spread_y_max" );
		int iDirectionChange = (int)CVAR_GET_FLOAT( "am_wp_spread_dir_change" );

		punch.x = flSpreadXBase + flSpreadXMod * bulletsFired;
		punch.y = flSpreadYBase + flSpreadYMod * bulletsFired;
		if ( RANDOM_LONG( 0, 100 ) < iDirectionChange )
			punch.x = -punch.x;

		if ( punch.x > flSpreadXMax )
			punch.x = flSpreadXMax;
		if ( punch.x < -flSpreadXMax )
			punch.x = -flSpreadXMax;

		if ( punch.y > flSpreadYMax )
			punch.y = flSpreadYMax;
	}
	else
#endif
	{
		//	 m_pPlayer->pev->punchangle.x = -2;


		punch.x = spreadXBase() + spreadXMod()/* * bulletsFired*/;
		punch.y = spreadYBase() + spreadYMod()/* * bulletsFired*/;
		//	if ( RANDOM_LONG( 0, 100 ) < 50 )
		//		punch.x = -punch.x;

		if (punch.x > spreadXMax())
			punch.x = spreadXMax();
		if (punch.x < -spreadXMax())
			punch.x = -spreadXMax();

		if (punch.y > spreadYMax())
			punch.y = spreadYMax();

		if (IsSilenced() || m_pPlayer->m_iFOV == 60)
		{
			punch.x *= silencerSpreadModifier();
			punch.y *= silencerSpreadModifier();
		}
	}


	MESSAGE_BEGIN(MSG_ONE, gmsgRecoil, NULL, m_pPlayer->pev);
	WRITE_BYTE(punch.y * 4);
	MESSAGE_END();

	//	m_pPlayer->ViewPunch(
	//	UTIL_SharedRandomFloat( m_pPlayer->random_seed, 0, 2 ),0,0);
	if (m_pPlayer)
		if (m_pPlayer->m_iFOV >= 87)
		{
		
	 m_pPlayer->ViewPunch(punch.y, RANDOM_FLOAT(-punch.x, punch.x), 0.0);
}
	else
		m_pPlayer->ViewPunch(RANDOM_FLOAT(-punch.y, punch.y), RANDOM_FLOAT(-punch.x, punch.x), 0.0);


}
