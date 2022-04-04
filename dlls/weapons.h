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
#ifndef WEAPONS_H
#define WEAPONS_H

#include "effects.h"

class CBasePlayer;
extern int gmsgWeapPickup;

void DeactivateSatchels( CBasePlayer *pOwner, bool bCheckOldEntityName = false );

// Contact Grenade / Timed grenade / Semtex Charge
class CGrenade : public CBaseMonster
{
public:
#ifndef CLIENT_DLL
	int		Save(CSave &save);
	int		Restore(CRestore &restore);
	static	TYPEDESCRIPTION m_SaveData[];
#endif

	void Spawn( void );
	BOOL m_bIsFlashbang;
	BOOL m_bIsSmoke;
	BOOL m_bIsSfera;
	BOOL m_bIsSemtex;

	typedef enum { SATCHEL_DETONATE = 0, SATCHEL_RELEASE } SATCHELCODE;
	static CGrenade *ShootTimedSmoke(entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float time);
	static CGrenade *ShootSmokeFragment(entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float time);
	static CGrenade *ShootFlashbang(entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float time);
	static CGrenade *FlashShootTimed(entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float time);
	void EXPORT FlashTumbleThink(void);

	void EXPORT Smoke(void);
	void Flash(TraceResult *pTrace, int bitsDamageType);

	void EXPORT SmokeTumbleThink(void);
	void EXPORT SmokeDetonate(void);
	void SmokeGren(TraceResult *pTrace);

	static CGrenade *ShootTimed( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float time );
	static CGrenade *ShootContact( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity );
	static CGrenade *ShootSatchelCharge( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity );
	static void UseSatchelCharges( entvars_t *pevOwner, SATCHELCODE code );

	void Explode( Vector vecSrc, Vector vecAim );
	void Explode( TraceResult *pTrace, int bitsDamageType );

	void EXPORT BounceTouch( CBaseEntity *pOther );
	void EXPORT SlideTouch( CBaseEntity *pOther );
	void EXPORT ExplodeTouch( CBaseEntity *pOther );
	void EXPORT DangerSoundThink( void );
	void EXPORT PreDetonate( void );
	void EXPORT Detonate( void );
	void EXPORT DetonateUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void EXPORT TumbleThink( void );

	virtual void BounceSound( void );
	virtual int	BloodColor( void ) { return DONT_BLEED; }
	virtual void Killed( entvars_t *pevAttacker, int iGib );

	BOOL m_fRegisteredSound;// whether or not this grenade has issued its DANGER sound to the world sound list yet.
};


// constant items
#define ITEM_HEALTHKIT		1
#define ITEM_ANTIDOTE		2
#define ITEM_SECURITY		3
#define ITEM_BATTERY		4
#define ITEM_EPIPEN			5
#define ITEM_GRENADE		6

#define WEAPON_NONE				0
// Melee and items
#define WEAPON_KNIFE			1
#define WEAPON_FLASHLIGHT		2
#define WEAPON_FISTS			3
#define WEAPON_PDA				4
#define WEAPON_SEMTEX			5
#define WEAPON_ROSE				6
// Pistols
#define WEAPON_DEAGLE			7
#define WEAPON_1911				8
#define WEAPON_BERETTA			9
#define WEAPON_GLOCK			10
#define WEAPON_ANACONDA			11
// SMGs
#define WEAPON_MP5				12
#define WEAPON_P90				13
#define WEAPON_BIZON			14
// Rifles
#define WEAPON_FAMAS			15
#define WEAPON_M4A1				16
#define WEAPON_AK47				17
#define WEAPON_M249				18
// Shotguns
#define WEAPON_SPAS12			19
#define WEAPON_LESSLETHAL		20
#define WEAPON_USAS				21
// Snipers
#define WEAPON_M14				22
#define WEAPON_L96A1			23
// Explosives
#define WEAPON_RPG				24

#define WEAPON_ALLWEAPONS		(~(1<<WEAPON_SUIT))

#define WEAPON_SUIT				31	// ?????

#define MAX_WEAPONS			32


#define MAX_NORMAL_BATTERY	100


// weapon weight factors (for auto-switching)   (-1 = noswitch)
#define KNIFE_WEIGHT		0
#define PISTOL_WEIGHT		10
#define SMG_WEIGHT			15
#define RIFLE_WEIGHT		20
#define EXPLOSIVE_WEIGHT	25

// weapon clip/carry ammo capacities
#define HEAL_MAX_CARRY			5
#define _9MM_MAX_CARRY			150
#define _50_MAX_CARRY			35
#define _44_MAX_CARRY			36
#define _57_MAX_CARRY			250
#define _556_MAX_CARRY			120
#define _762_MAX_CARRY			90
#define BUCKSHOT_MAX_CARRY		125
#define ROCKET_MAX_CARRY		5

// the maximum amount of ammo each weapon's clip can hold
#define WEAPON_NOCLIP			-1

// The amount of ammo given to a player by an ammo item.
#define AMMO_BUCKSHOTBOX_GIVE	12

// bullet types
typedef	enum
{
	BULLET_NONE = 0,
	// Melee
	BULLET_PLAYER_KNIFE,
	BULLET_PLAYER_FISTS,
	// Pistols
	BULLET_PLAYER_DEAGLE,
	BULLET_PLAYER_1911,
	BULLET_PLAYER_BERETTA,
	BULLET_PLAYER_GLOCK,
	BULLET_PLAYER_ANACONDA,
	// SMGs
	BULLET_PLAYER_MP5,
	BULLET_PLAYER_P90,
	BULLET_PLAYER_BIZON,
	// Rifles
	BULLET_PLAYER_FAMAS,
	BULLET_PLAYER_M4A1,
	BULLET_PLAYER_AK47,
	BULLET_PLAYER_M249,
	// Shotguns
	BULLET_PLAYER_SPAS12,
	BULLET_PLAYER_LESSLETHAL,
	BULLET_PLAYER_USAS,
	// Snipers
	BULLET_PLAYER_M14,
	BULLET_PLAYER_L96A1,
	// Monsters bullets
	BULLET_MONSTER_9MM,
	BULLET_MONSTER_MP5,
	BULLET_MONSTER_12MM,
} Bullet;

#define	MELEE_BODYHIT_VOLUME		128
#define	MELEE_WALLHIT_VOLUME		512

#define ITEM_FLAG_SELECTONEMPTY		1
#define ITEM_FLAG_NOAUTORELOAD		2
#define ITEM_FLAG_NOAUTOSWITCHEMPTY	4
#define ITEM_FLAG_LIMITINWORLD		8
#define ITEM_FLAG_EXHAUSTIBLE		16 // A player can totally exhaust their ammo supply and lose this weapon

#define WEAPON_IS_ONTARGET 0x40

typedef struct
{
	int		iSlot;
	int		iPosition;
	const char	*pszAmmo1;	// ammo 1 type
	int		iMaxAmmo1;		// max ammo 1
	const char	*pszAmmo2;	// ammo 2 type
	int		iMaxAmmo2;		// max ammo 2
	const char	*pszName;
	int		iMaxClip;
	int		iId;
	int		iFlags;
	int		iWeight;// this value used to determine this weapon's importance in autoselection.
} ItemInfo;

typedef struct
{
	const char *pszName;
	int iId;
} AmmoInfo;

// Items that the player has in their inventory that they can use
class CBasePlayerItem : public CBaseAnimating
{
public:
	virtual void SetObjectCollisionBox( void );

	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );

	static	TYPEDESCRIPTION m_SaveData[];

	virtual int AddToPlayer( CBasePlayer *pPlayer );	// return TRUE if the item you want the item added to the player inventory
	virtual int AddDuplicate( CBasePlayerItem *pItem ) { return FALSE; }	// return TRUE if you want your duplicate removed from world
	void EXPORT DestroyItem( void );
	void EXPORT DefaultTouch( CBaseEntity *pOther );	// default weapon touch
	void EXPORT FallThink ( void );// when an item is first spawned, this think is run to determine when the object has hit the ground.
	void EXPORT Materialize( void );// make a weapon visible and tangible
	void EXPORT AttemptToMaterialize( void );  // the weapon desires to become visible and tangible, if the game rules allow for it
	CBaseEntity* Respawn ( void );// copy a weapon
	void FallInit( void );
	void CheckRespawn( void );
	virtual int GetItemInfo(ItemInfo *p) { return 0; };	// returns 0 if struct not filled out
	virtual BOOL CanDeploy( void ) { return TRUE; };
	virtual BOOL Deploy( )								// returns is deploy was successful
		 { return TRUE; };

	virtual BOOL CanHolster( void ) { return TRUE; };// can this weapon be put away right now?
	virtual void Holster( int skiplocal = 0 );
	virtual void UpdateItemInfo( void ) { return; };

	virtual void ItemPreFrame( void )	{ return; }		// called each frame by the player PreThink
	virtual void ItemPostFrame( void ) { return; }		// called each frame by the player PostThink

	virtual void Drop( void );
	virtual void Kill( void );
	virtual void AttachToPlayer ( CBasePlayer *pPlayer );

	virtual int PrimaryAmmoIndex() { return -1; };
	virtual int SecondaryAmmoIndex() { return -1; };

	virtual int UpdateClientData( CBasePlayer *pPlayer ) { return 0; }

	virtual CBasePlayerItem *GetWeaponPtr( void ) { return NULL; };

	static ItemInfo ItemInfoArray[ MAX_WEAPONS ];
	static AmmoInfo AmmoInfoArray[ MAX_AMMO_SLOTS ];

	CBasePlayer	*m_pPlayer;
	CBasePlayerItem *m_pNext;
	int		m_iId;												// WEAPON_???

	virtual int iItemSlot( void ) { return 0; }			// return 0 to MAX_ITEMS_SLOTS, used in hud

	int			iItemPosition( void ) { return ItemInfoArray[ m_iId ].iPosition; }
	const char	*pszAmmo1( void )	{ return ItemInfoArray[ m_iId ].pszAmmo1; }
	int			iMaxAmmo1( void )	{ return ItemInfoArray[ m_iId ].iMaxAmmo1; }
	const char	*pszAmmo2( void )	{ return ItemInfoArray[ m_iId ].pszAmmo2; }
	int			iMaxAmmo2( void )	{ return ItemInfoArray[ m_iId ].iMaxAmmo2; }
	const char	*pszName( void )	{ return ItemInfoArray[ m_iId ].pszName; }
	int			iMaxClip( void )	{ return ItemInfoArray[ m_iId ].iMaxClip; }
	int			iWeight( void )		{ return ItemInfoArray[ m_iId ].iWeight; }
	int			iFlags( void )		{ return ItemInfoArray[ m_iId ].iFlags; }

	// int		m_iIdPrimary;										// Unique Id for primary ammo
	// int		m_iIdSecondary;										// Unique Id for secondary ammo
};

typedef struct wep_params_s //iron?
{
	char szWeaponName[32];

	float			origin_x;
	float			origin_y;
	float			origin_z;

} wep_params_t;

// inventory items that
class CBasePlayerWeapon : public CBasePlayerItem
{
public:
	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );

	static	TYPEDESCRIPTION m_SaveData[];

	void SetTableSettingsPointer(void);
	wep_params_t	*m_pMySettings; // pointer to global spread table for this gun

	// generic weapon versions of CBasePlayerItem calls
	virtual int AddToPlayer( CBasePlayer *pPlayer );
	virtual int AddDuplicate( CBasePlayerItem *pItem );

	virtual int ExtractAmmo( CBasePlayerWeapon *pWeapon ); //{ return TRUE; };			// Return TRUE if you can add ammo to yourself when picked up
	virtual int ExtractClipAmmo( CBasePlayerWeapon *pWeapon );// { return TRUE; };			// Return TRUE if you can add ammo to yourself when picked up

	virtual int AddWeapon( void ) { ExtractAmmo( this ); return TRUE; };	// Return TRUE if you want to add yourself to the player

	// generic "shared" ammo handlers
	BOOL AddPrimaryAmmo( int iCount, char *szName, int iMaxClip, int iMaxCarry );
	BOOL AddSecondaryAmmo( int iCount, char *szName, int iMaxCarry );

//	virtual void UpdateItemInfo( void ) {};	// updates HUD state
	virtual void UpdateItemInfo(void);//sys
	int m_iPlayEmptySound;
	int m_fFireOnEmpty;		// True when the gun is empty and the player is still holding down the
							// attack key(s)
	virtual BOOL PlayEmptySound( void );
	virtual void ResetEmptySound( void );

	virtual void SendWeaponAnim( int iAnim, int skiplocal = 1, int body = 0 );  // skiplocal is 1 if client is predicting weapon animations

	virtual BOOL CanDeploy( void );
	virtual BOOL IsUseable( void );
	BOOL DefaultDeploy( char *szViewModel, char *szWeaponModel, int iAnim, char *szAnimExt, int skiplocal = 0, int body = 0 );
	int DefaultReload( int iClipSize, int iAnim, float fDelay, int body = 0 );
	
	virtual void ItemPreFrame(void);	// called each frame by the player PreThink
	virtual void ItemPostFrame( void );	// called each frame by the player PostThink
	// called by CBasePlayerWeapons ItemPostFrame()
	virtual void PrimaryAttack( void ) { return; }				// do "+ATTACK"
	virtual void SecondaryAttack( void ) { return; }			// do "+ATTACK2"
	virtual void Reload( void ) { return; }						// do "+RELOAD"
	virtual void WeaponIdle( void ) { return; }					// called when no buttons pressed
	virtual int UpdateClientData( CBasePlayer *pPlayer );		// sends hud info to client dll, if things have changed
	virtual void RetireWeapon( void );
	virtual BOOL ShouldWeaponIdle( void ) {return FALSE; };
	virtual void Holster( int skiplocal = 0 );
	virtual BOOL UseDecrement( void ) { return FALSE; };

	int	PrimaryAmmoIndex();
	int	SecondaryAmmoIndex();

	void PrintState( void );

	virtual CBasePlayerItem *GetWeaponPtr( void ) { return (CBasePlayerItem *)this; };
	float GetNextAttackDelay( float delay );

	float m_flPumpTime;
	int		m_fInSpecialReload;									// Are we in the middle of a reload for the shotguns
	float	m_flNextPrimaryAttack;								// soonest time ItemPostFrame will call PrimaryAttack
	float	m_flNextSecondaryAttack;							// soonest time ItemPostFrame will call SecondaryAttack
	float	m_flTimeWeaponIdle;									// soonest time ItemPostFrame will call WeaponIdle
	int		m_iPrimaryAmmoType;									// "primary" ammo index into players m_rgAmmo[]
	int		m_iSecondaryAmmoType;								// "secondary" ammo index into players m_rgAmmo[]
	int		m_iClip;											// number of shots left in the primary weapon clip, -1 it not used
	int		m_iClientClip;										// the last version of m_iClip sent to hud dll
	int		m_iClientWeaponState;								// the last version of the weapon state sent to hud dll (is current weapon, is on target)
	int		m_fInReload;										// Are we in the middle of a reload;

	int		m_iDefaultAmmo;// how much ammo you get when you pick up this weapon as placed by a level designer.

	// hle time creep vars
	float	m_flPrevPrimaryAttack;
	float	m_flLastFireTime;

	virtual int IsAmWeapon() { return FALSE; }
};

enum wpmods
{
	MOD_NOMODS = 1,
	MOD_REDDOT = 2,
	MOD_SILENCER = 4,
	MOD_SCOPE = 8,
	MOD_BURSTATTACK = 16,
	MOD_SEMIAUTO = 32,
};

enum wptype
{
	TYPE_WEAPON = 0,
	TYPE_ITEM_AMMO,
	TYPE_ITEM_INFINITE
};

class CBasePlayerAmmo : public CBaseEntity
{
public:
	virtual void Spawn( void );
	void EXPORT DefaultTouch( CBaseEntity *pOther ); // default weapon touch
	virtual BOOL AddAmmo( CBaseEntity *pOther ) { return TRUE; };

	CBaseEntity* Respawn( void );
	void EXPORT Materialize( void );
};


extern DLL_GLOBAL	short	g_sModelIndexLaser;// holds the index for the laser beam
extern DLL_GLOBAL	const char *g_pModelNameLaser;

extern DLL_GLOBAL	short	g_sModelIndexLaserDot;// holds the index for the laser beam dot
extern DLL_GLOBAL	short	g_sModelIndexFireball;// holds the index for the fireball
extern DLL_GLOBAL	short	g_sModelIndexSmoke;// holds the index for the smoke cloud
extern DLL_GLOBAL	short	g_sModelIndexWExplosion;// holds the index for the underwater explosion
extern DLL_GLOBAL	short	g_sModelIndexBubbles;// holds the index for the bubbles model
extern DLL_GLOBAL	short	g_sModelIndexBloodDrop;// holds the sprite index for blood drops
extern DLL_GLOBAL	short	g_sModelIndexBloodSpray;// holds the sprite index for blood spray (bigger)

extern void ClearMultiDamage(void);
extern void ApplyMultiDamage(entvars_t* pevInflictor, entvars_t* pevAttacker );
extern void AddMultiDamage( entvars_t *pevInflictor, CBaseEntity *pEntity, float flDamage, int bitsDamageType);

extern void DecalGunshot( TraceResult *pTrace, int iBulletType );
extern void SpawnBlood(Vector vecSpot, int bloodColor, float flDamage);
extern int DamageDecal( CBaseEntity *pEntity, int bitsDamageType );
extern void RadiusDamage( Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, float flRadius, int iClassIgnore, int bitsDamageType );

typedef struct
{
	CBaseEntity		*pEntity;
	float			amount;
	int				type;
} MULTIDAMAGE;

extern MULTIDAMAGE gMultiDamage;


#define LOUD_GUN_VOLUME			1000
#define NORMAL_GUN_VOLUME		600
#define QUIET_GUN_VOLUME		200

#define	BRIGHT_GUN_FLASH		512
#define NORMAL_GUN_FLASH		256
#define	DIM_GUN_FLASH			128

#define BIG_EXPLOSION_VOLUME	2048
#define NORMAL_EXPLOSION_VOLUME	1024
#define SMALL_EXPLOSION_VOLUME	512

#define	WEAPON_ACTIVITY_VOLUME	64

#define VECTOR_CONE_1DEGREES	Vector( 0.00873, 0.00873, 0.00873 )
#define VECTOR_CONE_2DEGREES	Vector( 0.01745, 0.01745, 0.01745 )
#define VECTOR_CONE_3DEGREES	Vector( 0.02618, 0.02618, 0.02618 )
#define VECTOR_CONE_4DEGREES	Vector( 0.03490, 0.03490, 0.03490 )
#define VECTOR_CONE_5DEGREES	Vector( 0.04362, 0.04362, 0.04362 )
#define VECTOR_CONE_6DEGREES	Vector( 0.05234, 0.05234, 0.05234 )
#define VECTOR_CONE_7DEGREES	Vector( 0.06105, 0.06105, 0.06105 )
#define VECTOR_CONE_8DEGREES	Vector( 0.06976, 0.06976, 0.06976 )
#define VECTOR_CONE_9DEGREES	Vector( 0.07846, 0.07846, 0.07846 )
#define VECTOR_CONE_10DEGREES	Vector( 0.08716, 0.08716, 0.08716 )
#define VECTOR_CONE_15DEGREES	Vector( 0.13053, 0.13053, 0.13053 )
#define VECTOR_CONE_20DEGREES	Vector( 0.17365, 0.17365, 0.17365 )

#define VECTOR_CONE_PERFECT		Vector( 0.0, 0.0, 0.0 )
#define VECTOR_CONE_BAD			Vector( 0.27365, 0.27365, 0.27365 )
#define VECTOR_CONE_TOOBAD		Vector( 0.47365, 0.47365, 0.47365 )

class CBaseAMWeapon : public CBasePlayerWeapon
{
public:
	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	static			TYPEDESCRIPTION m_SaveData[];

	BOOL DefaultDeploy( int skiplocal = 0, int body = 0 );
	int DefaultReload( int iClipSize, int iAnim, float fDelay, int body = 0 );

	virtual int GetBody();
	virtual int InstallMod( int mod );
	virtual void SetBody( int body );
	virtual void SetSkin( int skin );

	virtual BOOL Deploy();
	virtual void Reload();
	virtual int AddToPlayer( CBasePlayer *pPlayer );
	virtual int WeaponID()					{ return 0; }
	virtual void SecondaryAttack( void );	// do "+ATTACK2"
	virtual void ItemPostFrame( void );
	virtual void PrimaryAttack();
	virtual void Spawn();
	virtual void Holster( int skiplocal = 0 );

	int installedMods;
	int clientMods;

	virtual int GetAvailableMods()			{ return MOD_NOMODS; }
	virtual BOOL AllowIronSight()			{ return TRUE; }//need to return false if it's pistol.
	virtual int IsAmWeapon()				{ return TRUE; }
	virtual int WeaponType()				{ return TYPE_WEAPON; }
	virtual float DeployTime()				{ return 1.0; }
	virtual float DropRecoilDelay()			{ return 0.05; }//0.1
	virtual float AttackDelay()				{ return 0.0875; }
	virtual float ReloadTime()				{ return 1.5; }
	virtual float ReloadEmptyTime()			{ return ReloadTime(); }
	virtual int FireAnimation()				{ return 2; }
	virtual int FireEmptyAnimation()		{ return IdleEmptyAnimation(); }
	virtual int FireLastAnimation()			{ return FireAnimation(); }
	virtual int FireVariations()			{ return 0; }
	virtual int ReloadAnimation()			{ return 3; }
	virtual int ReloadEmptyAnimation()		{ return ReloadAnimation(); }
	virtual int ReloadVariations()			{ return 0; }
	virtual int DeployAnimation()			{ return 1; }
	virtual int DeployEmptyAnimation()		{ return DeployAnimation(); }
	virtual int DeployVariations()			{ return 0; }
	virtual int IdleAnimation()				{ return 0; }
	virtual int IdleEmptyAnimation()		{ return -1; }
	virtual int IdleVariations()			{ return 0; }
	virtual int IdleEmptyVariations()		{ return 0; }
	virtual Vector WeaponSpread()			{ return Vector( 0, 0, 0 ); }
	virtual float silencerSpreadModifier()	{ return 0.35; }
	virtual float ShakeAmount()				{ return 50; }

	virtual int BulletType()				{ return BULLET_PLAYER_MP5; }
	virtual const char *GetAmmoName()		{ return ""; }
	virtual int GetMaxCarry()				{ return 120; }

	virtual BOOL IsSilenced()				{ return (installedMods & MOD_SILENCER); }
	virtual BOOL ModInstalled( int mod )	{ return installedMods & mod; }

	virtual int PelletsToFire() 			{ return 8; }
	virtual BOOL FireAsShotgun() 			{ return FALSE; }
	virtual BOOL ReloadAsPumpShotgun()		{ return FALSE; }
	virtual float ReloadShotgunStartTime()	{ return 0.5; }
	virtual float ReloadShotgunEndTime()	{ return 1.0; }

	virtual char *GetDeploySound()			{ return ""; }
	virtual char *GetFireSound()			{ return ""; }
	virtual char *GetFire2Sound()			{ return GetFireSound(); }
	virtual char *GetFire3Sound()			{ return GetFireSound(); }
	virtual char *GetSilencedFireSound()	{ return GetFireSound(); }
	virtual char *GetSilencedFire2Sound()	{ return GetSilencedFireSound(); }
	virtual char *GetSilencedFire3Sound()	{ return GetSilencedFireSound(); }

	virtual char *GetVModel()				{ return "models/weapons/mp5/v_mp5.mdl"; }
	virtual char *GetWModel()				{ return "models/weapons/mp5/w_mp5.mdl"; }
	virtual char *GetPModel()				{ return "models/weapons/mp5/p_mp5.mdl"; }
	virtual char *GetExtAnim()				{ return "mp5"; }
	virtual char *GetScopeModel() 			{ return NULL; }
	virtual char *GetShellModel() 			{ return "models/weapons/shell_9mm.mdl"; }

	virtual void Precache();
	virtual int GetItemInfo( ItemInfo *p );
	virtual void WeaponIdle();
	virtual int GetMaxClipAmmo()			{ return 30; }
	virtual BOOL FireUnderWater()			{ return FALSE; }
	virtual BOOL SemiAutoModeOnly()			{ return FALSE; }
	virtual int GunVolume()					{ return NORMAL_GUN_VOLUME; }
	virtual int GunFlash()					{ return NORMAL_GUN_FLASH; }
	virtual	char *WeaponClassName()			{ return "weapon_none"; }
	virtual int iItemSlot()					{ return 8; }
	virtual int WeaponPosition()			{ return 0; }
	virtual int WeaponWeight()				{ return -1; }

	virtual float spreadXMax()				{ return 1.5; }
	virtual float spreadYMax()				{ return 3.4; }
	virtual float spreadXBase()				{ return 0.2; }
	virtual float spreadYBase()				{ return 0.4; }
	virtual float spreadXMod()				{ return 0.1; }
	virtual float spreadYMod()				{ return 0.15; }
	virtual int directionChange()			{ return 30; }
	virtual void punchScreen();
	void PlayIdleAnimation();

	float nextRecoilDecrease;
	float bulletsFired;//yes, it's float.
	int m_iShell;
	// For pump shotguns
	float m_flNextReload;
	int m_fInSpecialReload;
	unsigned short m_usWeaponShell;
};

//==================================================
// MELEE AND ITEMS
//==================================================

class CKnife : public CBaseAMWeapon
{
public:
	// Specific knife methods and attributes
	void EXPORT Smack( void );
	TraceResult m_trHit;

	// Overriden CBasePlayerWeapon methods
	void Precache( void );
	void PrimaryAttack( void );
	void SecondaryAttack( void );
	int iItemSlot( void )			{ return 1; }

	// Overriden CBaseAMWeapon methods
	float AttackDelay()				{ return 0.9; }
	int FireVariations()			{ return 1; }
	int ReloadAnimation()			{ return 4; } // Don't worry, it's not reloading, it's stabbing ^^

	int WeaponID()					{ return WEAPON_KNIFE; }
	char *WeaponClassName()			{ return "weapon_knife"; }

	int BulletType()				{ return BULLET_PLAYER_KNIFE; }
	int GetMaxCarry()				{ return -1; }
	int GetMaxClipAmmo()			{ return -1; }
	int WeaponWeight()				{ return KNIFE_WEIGHT; }

	char *GetVModel()				{ return "models/weapons/knife/v_knife.mdl"; }
	char *GetWModel()				{ return "models/weapons/knife/w_knife.mdl"; }
	char *GetPModel()				{ return "models/weapons/knife/p_knife.mdl"; }
	char *GetExtAnim()				{ return "crowbar"; }

	char *GetFireSound()			{ return "weapons/knife/hit_flesh-1.wav"; }
	char *GetFire2Sound()			{ return "weapons/knife/hit_flesh-2.wav"; }
	char *GetFire3Sound()			{ return "weapons/knife/hit_flesh-3.wav"; }
	char *GetSilencedFireSound()	{ return "weapons/knife/stab_hit_flesh-1.wav"; }
	char *GetSilencedFire2Sound()	{ return "weapons/knife/stab_hit_flesh-2.wav"; }
	char *GetDeploySound()			{ return "weapons/draw_knife.wav"; }
};

class CFlashlight : public CBaseAMWeapon
{
public:
	// Overriden CBasePlayerWeapon methods
	void PrimaryAttack( void );
	int iItemSlot( void )			{ return 1; }
	void Holster( int skiplocal = 0 );

	// Overriden CBaseAMWeapon methods
	BOOL AllowIronSight()			{ return FALSE; }
	int WeaponType()				{ return TYPE_ITEM_INFINITE; }
	float AttackDelay()				{ return 0.5; }
	int IdleVariations()			{ return 1; }
	int DeployAnimation()			{ return 2; }
	int FireAnimation()				{ return 3; }

	int WeaponID()					{ return WEAPON_FLASHLIGHT; }
	char *WeaponClassName()			{ return "weapon_flash"; }

	int GetMaxCarry()				{ return -1; }
	int GetMaxClipAmmo()			{ return -1; }
	int WeaponPosition()			{ return 1; }
	int WeaponWeight()				{ return KNIFE_WEIGHT; }

	char *GetVModel()				{ return "models/weapons/flash/v_flash.mdl"; }
	char *GetWModel()				{ return "models/weapons/flash/w_flash.mdl"; }
	char *GetPModel()				{ return "models/weapons/flash/p_flash.mdl"; }
	char *GetExtAnim()				{ return "grenade"; }

	char *GetDeploySound()			{ return "weapons/draw_pistol.wav"; }
};

class CPDA : public CBaseAMWeapon
{
public:
	// Specific PDA methods and attributes
	void EXPORT OpenOS( void );

	// Overriden CBasePlayerWeapon methods
	void PrimaryAttack( void );
	int iItemSlot( void )			{ return 1; }

	// Overriden CBaseAMWeapon methods
	BOOL AllowIronSight()			{ return FALSE; }
	int WeaponType()				{ return TYPE_ITEM_INFINITE; }
	float AttackDelay()				{ return 0.5; }

	int WeaponID()					{ return WEAPON_PDA; }
	char *WeaponClassName()			{ return "weapon_pda"; }

	int GetMaxCarry()				{ return -1; }
	int GetMaxClipAmmo()			{ return -1; }
	int WeaponPosition()			{ return 2; }
	int WeaponWeight()				{ return KNIFE_WEIGHT; }

	char *GetVModel()				{ return "models/weapons/pda/v_pda.mdl"; }
	char *GetWModel()				{ return "models/weapons/pda/w_pda.mdl"; }
	char *GetPModel()				{ return "models/null.mdl"; }
	char *GetExtAnim()				{ return "grenade"; }

	char *GetDeploySound()			{ return "weapons/draw_pistol.wav"; }
};

class CFists : public CBaseAMWeapon
{
public:
	// Overriden CBasePlayerWeapon methods
	void Precache( void );
	void PrimaryAttack( void );
	void SecondaryAttack( void );
	int iItemSlot( void )			{ return 1; }
	void WeaponIdle( void );
	void Holster( int skiplocal = 0 );

	// Overriden CBaseAMWeapon methods
	float AttackDelay()				{ return 0.46; }
	int FireVariations()			{ return 3; }
	int ReloadAnimation()			{ return 6; } // Don't worry, it's not reloading, it's covering ^^

	int WeaponID()					{ return WEAPON_FISTS; }
	char *WeaponClassName()			{ return "weapon_fists"; }

	int BulletType()				{ return BULLET_PLAYER_FISTS; }
	int GetMaxCarry()				{ return -1; }
	int GetMaxClipAmmo()			{ return -1; }
	int WeaponPosition()			{ return 3; }
	int WeaponWeight()				{ return KNIFE_WEIGHT; }

	char *GetVModel()				{ return "models/weapons/fists/v_fists.mdl"; }
	char *GetWModel()				{ return "models/null.mdl"; }
	char *GetPModel()				{ return "models/null.mdl"; }
	char *GetExtAnim()				{ return "crowbar"; }

	char *GetFireSound()			{ return "weapons/fists/hit-1.wav"; }
	char *GetFire2Sound()			{ return "weapons/fists/hit-2.wav"; }
	char *GetSilencedFireSound()	{ return "weapons/fists/miss-1.wav"; }
	char *GetSilencedFire2Sound()	{ return "weapons/fists/miss-2.wav"; }
	char *GetDeploySound()			{ return "weapons/draw_pistol.wav"; }
};

class CSemtex : public CBaseAMWeapon
{
public:
	// Save/restore methods
	int		Save( CSave &save );
	int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	// Overriden CBasePlayerWeapon methods
	BOOL CanDeploy( void );
	BOOL Deploy( void );
	void Precache( void );
	void PrimaryAttack( void );
	int iItemSlot( void )			{ return 1; }
	void WeaponIdle( void );
	void Holster( int skiplocal = 0 );

	// Overriden CBaseAMWeapon methods
	BOOL AllowIronSight()			{ return FALSE; }
	int WeaponType()				{ return TYPE_ITEM_AMMO; }
	float AttackDelay()				{ return 1.0; }
	int IdleVariations()			{ return 1; }
	int FireAnimation()				{ return 2; }
	int DeployAnimation()			{ return 2; }

	int WeaponID()					{ return WEAPON_SEMTEX; }
	char *WeaponClassName()			{ return "weapon_semtex"; }

	int GetMaxCarry()				{ return 5; }
	int GetMaxClipAmmo()			{ return -1; }
	int WeaponPosition()			{ return 4; }
	int WeaponWeight()				{ return KNIFE_WEIGHT; }
	const char *GetAmmoName()		{ return "semtex"; }

	char *GetVModel()				{ return "models/weapons/semtex/v_semtex.mdl"; }
	char *GetWModel()				{ return "models/weapons/semtex/w_semtex.mdl"; }
	char *GetPModel()				{ return "models/p_satchel.mdl"; }
	char *GetExtAnim()				{ return "trip"; }

	char *GetDeploySound()			{ return "weapons/draw_pistol.wav"; }
};

class CRose : public CBaseAMWeapon
{
public:
	// Overriden CBasePlayerWeapon methods
	void PrimaryAttack( void );
	int iItemSlot( void )			{ return 1; }

	// Overriden CBaseAMWeapon methods
	BOOL AllowIronSight()			{ return FALSE; }
	int WeaponType()				{ return TYPE_ITEM_INFINITE; }
	int DeployAnimation()			{ return 0; }
	int IdleAnimation()				{ return 1; }
//	int IdleVariations()			{ return 1; }
	int WeaponID()					{ return WEAPON_ROSE; }
	char *WeaponClassName()			{ return "weapon_rose"; }

	int GetMaxCarry()				{ return -1; }
	int GetMaxClipAmmo()			{ return -1; }
	int WeaponPosition()			{ return 5; }
	int WeaponWeight()				{ return KNIFE_WEIGHT; }
	const char *GetAmmoName()		{ return NULL; }

	char *GetVModel()				{ return "models/weapons/rose/v_rose.mdl"; }
	char *GetWModel()				{ return "models/null.mdl"; }
	char *GetPModel()				{ return "models/null.mdl"; }
	char *GetExtAnim()				{ return "crowbar"; }

	char *GetDeploySound()			{ return "weapons/draw_pistol.wav"; }
};

//==================================================
// PISTOLS
//==================================================

// Shepard TODO : Adjust spread
class CDeagle : public CBaseAMWeapon
{
public:
	// Overriden CBasePlayerWeapon methods
	int iItemSlot( void )			{ return 2; }

	// Overriden CBaseAMWeapon methods
	float ReloadTime()				{ return 2.3; }
	float ReloadEmptyTime()			{ return 2.6; }
	float AttackDelay()				{ return 0.2; }
	int IdleVariations()			{ return 1; }
	int IdleEmptyVariations()		{ return 1; }
	int IdleEmptyAnimation()		{ return 2; }
	int DeployAnimation()			{ return 4; }
	int DeployEmptyAnimation()		{ return 5; }
	int FireAnimation()				{ return 6; }
	int FireVariations()			{ return 1; }
	int FireEmptyAnimation()		{ return 9; }
	int FireLastAnimation()			{ return 8; }
	int ReloadAnimation()			{ return 10; }
	int ReloadEmptyAnimation()		{ return 11; }
	BOOL SemiAutoModeOnly()			{ return TRUE; }
	Vector WeaponSpread()			{ return VECTOR_CONE_PERFECT; }

	int WeaponID()					{ return WEAPON_DEAGLE; }
	char *WeaponClassName()			{ return "weapon_deagle"; }

	int BulletType()				{ return BULLET_PLAYER_DEAGLE; }
	int GetMaxCarry()				{ return _50_MAX_CARRY; }
	int GetMaxClipAmmo()			{ return 7; }
	int WeaponPosition()			{ return 0; }
	int WeaponWeight()				{ return PISTOL_WEIGHT; }
	const char *GetAmmoName()		{ return "50"; }

	float spreadXMax()				{ return 6.8; }
	float spreadYMax()				{ return 4.5; } // 4.2
	float spreadXBase()				{ return 1.55; }
	float spreadYBase()				{ return 1.3; } // 1.22
	float spreadXMod()				{ return 0.55; }
	float spreadYMod()				{ return 0.2; } // 0.14
	int directionChange()			{ return 10; }
	float DropRecoilDelay()			{ return 0.2; }

	char *GetVModel()				{ return "models/weapons/deagle/v_deagle.mdl"; }
	char *GetWModel()				{ return "models/weapons/deagle/w_deagle.mdl"; }
	char *GetPModel()				{ return "models/weapons/deagle/p_deagle.mdl"; }
	char *GetShellModel()			{ return "models/weapons/shell_50cal.mdl"; }
	char *GetExtAnim()				{ return "onehanded"; }

	char *GetFireSound()			{ return "weapons/deagle/fire.wav"; }
	char *GetDeploySound()			{ return "weapons/draw_pistol.wav"; }
};

// Shepard TODO : Adjust spread
class C1911 : public CBaseAMWeapon
{
public:
	// Overriden CBasePlayerWeapon methods
	int iItemSlot( void )			{ return 2; }

	// Overriden CBaseAMWeapon methods
	float ReloadTime()				{ return 2.5; }
	float AttackDelay()				{ return 0.15; }
	int IdleEmptyAnimation()		{ return 1; }
	int DeployAnimation()			{ return 2; }
	int DeployEmptyAnimation()		{ return 3; }
	int FireAnimation()				{ return 4; }
	int FireVariations()			{ return 1; }
	int FireLastAnimation()			{ return 6; }
	int ReloadAnimation()			{ return 7; }
	int ReloadEmptyAnimation()		{ return 8; }
	BOOL SemiAutoModeOnly()			{ return TRUE; }
	Vector WeaponSpread()			{ return VECTOR_CONE_PERFECT; }

	int WeaponID()					{ return WEAPON_1911; }
	char *WeaponClassName()			{ return "weapon_1911"; }

	int BulletType()				{ return BULLET_PLAYER_1911; }
	int GetMaxCarry()				{ return _9MM_MAX_CARRY; }
	int GetMaxClipAmmo()			{ return 7; }
	int WeaponPosition()			{ return 1; }
	int WeaponWeight()				{ return PISTOL_WEIGHT; }
	const char *GetAmmoName()		{ return "45"; }

	float spreadXMax()				{ return 1.8; }
	float spreadYMax()				{ return 1.2; }
	float spreadXBase()				{ return 0.55; }
	float spreadYBase()				{ return 0.5; }
	float spreadXMod()				{ return 0.55; }
	float spreadYMod()				{ return 0.14; }
	int directionChange()			{ return 10; }
	float DropRecoilDelay()			{ return 0.2; }

	char *GetVModel()				{ return "models/weapons/1911/v_1911.mdl"; }
	char *GetWModel()				{ return "models/weapons/1911/w_1911.mdl"; }
	char *GetPModel()				{ return "models/weapons/1911/p_1911.mdl"; }
	char *GetShellModel()			{ return "models/weapons/shell_9mm.mdl"; }
	char *GetExtAnim()				{ return "onehanded"; }

	char *GetFireSound()			{ return "weapons/1911/fire.wav"; }
	char *GetDeploySound()			{ return "weapons/draw_pistol.wav"; }
};

// Shepard TODO : Adjust spread
class CBeretta : public CBaseAMWeapon
{
public:
	// Overriden CBasePlayerWeapon methods
	int iItemSlot( void )			{ return 2; }

	// Overriden CBaseAMWeapon methods
	float ReloadTime()				{ return 2.5; }
	float AttackDelay()				{ return 0.15; }
	int IdleVariations()			{ return 1; }
	int IdleEmptyAnimation()		{ return 2; }
	int IdleEmptyVariations()		{ return 1; }
	int DeployAnimation()			{ return 4; }
	int DeployEmptyAnimation()		{ return 5; }
	int FireAnimation()				{ return 6; }
	int FireVariations()			{ return 1; }
	int FireLastAnimation()			{ return 8; }
	int ReloadAnimation()			{ return 9; }
	int ReloadEmptyAnimation()		{ return 10; }
	BOOL SemiAutoModeOnly()			{ return TRUE; }
	Vector WeaponSpread()			{ return VECTOR_CONE_3DEGREES; }

	int WeaponID()					{ return WEAPON_BERETTA; }
	char *WeaponClassName()			{ return "weapon_ber92f"; }

	int BulletType()				{ return BULLET_PLAYER_BERETTA; }
	int GetMaxCarry()				{ return _9MM_MAX_CARRY; }
	int GetMaxClipAmmo()			{ return 15; }
	int WeaponPosition()			{ return 2; }
	int WeaponWeight()				{ return PISTOL_WEIGHT; }
	int GetAvailableMods()			{ return MOD_NOMODS | MOD_SILENCER; }
	const char *GetAmmoName()		{ return "9mm"; }

	float spreadXMax()				{ return 1.8; }
	float spreadYMax()				{ return 1.2; }
	float spreadXBase()				{ return 0.55; }
	float spreadYBase()				{ return 0.22; }
	float spreadXMod()				{ return 0.55; }
	float spreadYMod()				{ return 0.14; }
	int directionChange()			{ return 10; }
	float DropRecoilDelay()			{ return 0.2; }

	char *GetVModel()				{ return "models/weapons/ber92f/v_ber92f.mdl"; }
	char *GetWModel()				{ return "models/weapons/ber92f/w_ber92f.mdl"; }
	char *GetPModel()				{ return "models/weapons/ber92f/p_ber92f.mdl"; }
	char *GetShellModel()			{ return "models/weapons/shell_9mm.mdl"; }
	char *GetExtAnim()				{ return "onehanded"; }

	char *GetFireSound()			{ return "weapons/ber92f/fire.wav"; }
	char *GetSilencedFireSound()	{ return "weapons/ber92f/fire_sil.wav"; }
	char *GetDeploySound()			{ return "weapons/draw_pistol.wav"; }
};

// Shepard TODO : Adjust spread
class CGlock : public CBaseAMWeapon
{
public:
	// Overriden CBasePlayerWeapon methods
	int iItemSlot( void )			{ return 2; }

	// Overriden CBaseAMWeapon methods
	float ReloadTime()				{ return 2.1; }
	float ReloadEmptyTime()			{ return 2.3; }
	float AttackDelay()				{ return 0.08; }
	int IdleEmptyAnimation()		{ return 3; }
	int IdleVariations()			{ return 2; }
	int DeployAnimation()			{ return 4; }
	int DeployEmptyAnimation()		{ return 5; }
	int FireAnimation()				{ return 6; }
	int FireLastAnimation()			{ return 9; }
	int ReloadAnimation()			{ return 10; }
	int ReloadEmptyAnimation()		{ return 11; }
	Vector WeaponSpread()			{ return VECTOR_CONE_3DEGREES; }

	int WeaponID()					{ return WEAPON_GLOCK; }
	char *WeaponClassName()			{ return "weapon_glock18"; }

	int BulletType()				{ return BULLET_PLAYER_GLOCK; }
	int GetMaxCarry()				{ return _9MM_MAX_CARRY; }
	int GetMaxClipAmmo()			{ return 20; }
	int WeaponPosition()			{ return 3; }
	int WeaponWeight()				{ return PISTOL_WEIGHT; }
	int GetAvailableMods()			{ return MOD_NOMODS | MOD_SEMIAUTO; }
	const char *GetAmmoName()		{ return "9mm"; }

	float spreadXMax()				{ return 1.8; }
	float spreadYMax()				{ return 1.2; }
	float spreadXBase()				{ return 0.55; }
	float spreadYBase()				{ return 0.22; }
	float spreadXMod()				{ return 0.55; }
	float spreadYMod()				{ return 0.14; }
	int directionChange()			{ return 20; }
	float DropRecoilDelay()			{ return 0.2; }

	char *GetVModel()				{ return "models/weapons/glock18/v_glock18.mdl"; }
	char *GetWModel()				{ return "models/weapons/glock18/w_glock18.mdl"; }
	char *GetPModel()				{ return "models/weapons/glock18/p_glock18.mdl"; }
	char *GetShellModel()			{ return "models/weapons/shell_9mm.mdl"; }
	char *GetExtAnim()				{ return "onehanded"; }

	char *GetFireSound()			{ return "weapons/glock18/fire.wav"; }
	char *GetDeploySound()			{ return "weapons/draw_pistol.wav"; }
};

// Shepard TODO : Adjust spread, add scope (maybe)
class CAnaconda : public CBaseAMWeapon
{
public:
	// Overriden CBasePlayerWeapon methods
	int iItemSlot( void )			{ return 2; }

	// Overriden CBaseAMWeapon methods
	float ReloadTime()				{ return 2.0; }
	float AttackDelay()				{ return 1.0; }
	int FireVariations()			{ return 1; }
	int ReloadAnimation()			{ return 4; }
	BOOL SemiAutoModeOnly()			{ return TRUE; }
	Vector WeaponSpread()			{ return VECTOR_CONE_3DEGREES; }

	int WeaponID()					{ return WEAPON_ANACONDA; }
	char *WeaponClassName()			{ return "weapon_anaconda"; }

	int BulletType()				{ return BULLET_PLAYER_ANACONDA; }
	int GetMaxCarry()				{ return _44_MAX_CARRY; }
	int GetMaxClipAmmo()			{ return 6; }
	int WeaponPosition()			{ return 4; }
	int WeaponWeight()				{ return PISTOL_WEIGHT; }
	const char *GetAmmoName()		{ return "44"; }

	float spreadXMax()				{ return 6.8; }
	float spreadYMax()				{ return 6.0; } // 4.2
	float spreadXBase()				{ return 1.55; }
	float spreadYBase()				{ return 1.55; } // 1.22
	float spreadXMod()				{ return 0.55; }
	float spreadYMod()				{ return 0.2; } // 0.14
	int directionChange()			{ return 10; }
	float DropRecoilDelay()			{ return 0.2; }

	char *GetVModel()				{ return "models/weapons/357/v_357.mdl"; }
	char *GetWModel()				{ return "models/weapons/357/w_357.mdl"; }
	char *GetPModel()				{ return "models/weapons/357/p_357.mdl"; }
	char *GetShellModel()			{ return "models/weapons/shell_50cal.mdl"; }
	char *GetExtAnim()				{ return "onehanded"; }

	char *GetFireSound()			{ return "weapons/357/fire.wav"; }
	char *GetDeploySound()			{ return "weapons/draw_pistol.wav"; }
};

//==================================================
// SMGS
//==================================================

class CMP5 : public CBaseAMWeapon
{
public:
	// Overriden CBasePlayerWeapon methods
	int iItemSlot( void )			{ return 3; }

	// Overriden CBaseAMWeapon methods
	float DeployTime()				{ return 1.2; }
	float ReloadTime()				{ return 3.4; }
	float AttackDelay()				{ return 0.0875; }
	int IdleVariations()			{ return 1; }
	int DeployAnimation()			{ return 2; }
	int FireAnimation()				{ return 3; }
	int FireEmptyAnimation()		{ return 7; }
	int FireVariations()			{ return 1; }
	int ReloadAnimation()			{ return 8; }
	Vector WeaponSpread()			{ return VECTOR_CONE_3DEGREES; }

	int WeaponID()					{ return WEAPON_MP5; }
	char *WeaponClassName()			{ return "weapon_mp5"; }

	int GetMaxCarry()				{ return _9MM_MAX_CARRY; }
	int WeaponWeight()				{ return SMG_WEIGHT; }
	int GetAvailableMods()			{ return MOD_NOMODS | MOD_SILENCER | MOD_BURSTATTACK | MOD_SEMIAUTO; }
	const char *GetAmmoName()		{ return "9mm"; }

	float spreadXMax()				{ return 1.5; }
	float spreadYMax()				{ return 3.4; }
	float spreadXBase()				{ return 0.07; }
	float spreadYBase()				{ return 0.15; }
	float spreadXMod()				{ return 0.01; }
	float spreadYMod()				{ return 0.15; }
	int directionChange()			{ return 10; }
	float DropRecoilDelay()			{ return 0.2; }

	char *GetVModel()				{ return "models/weapons/mp5/v_mp5.mdl"; }
	char *GetWModel()				{ return "models/weapons/mp5/w_mp5.mdl"; }
	char *GetPModel()				{ return "models/weapons/mp5/p_mp5.mdl"; }
	char *GetShellModel()			{ return "models/weapons/shell_9mm.mdl"; }
	char *GetExtAnim()				{ return "mp5"; }

	char *GetFireSound()			{ return "weapons/mp5/fire-1.wav"; }
	char *GetFire2Sound()			{ return "weapons/mp5/fire-2.wav"; }
	char *GetFire3Sound()			{ return "weapons/mp5/fire-3.wav"; }
	char *GetSilencedFireSound()	{ return "weapons/mp5/fire_sil-1.wav"; }
	char *GetSilencedFire2Sound()	{ return "weapons/mp5/fire_sil-2.wav"; }
	char *GetSilencedFire3Sound()	{ return "weapons/mp5/fire_sil-3.wav"; }
	char *GetDeploySound()			{ return "weapons/draw_smg.wav"; }
};

class CP90 : public CBaseAMWeapon
{
public:
	// Overriden CBasePlayerWeapon methods
	int iItemSlot( void ) { return 3; }

	// Overriden CBaseAMWeapon methods
	float ReloadTime()				{ return 3.4; }
	float AttackDelay()				{ return 0.06; }
	int IdleVariations()			{ return 1; }
	int DeployAnimation()			{ return 2; }
	int FireAnimation()				{ return 3; }
	int FireEmptyAnimation()		{ return 5; }
	int FireVariations()			{ return 1; }
	int ReloadAnimation()			{ return 6; }
	int ReloadEmptyAnimation()		{ return 7; }
	Vector WeaponSpread()			{ return VECTOR_CONE_3DEGREES; }

	int WeaponID()					{ return WEAPON_P90; }
	char *WeaponClassName()			{ return "weapon_p90"; }

	int BulletType()				{ return BULLET_PLAYER_P90; }
	int GetMaxCarry()				{ return _57_MAX_CARRY; }
	int GetMaxClipAmmo()			{ return 50; }
	int WeaponPosition()			{ return 1; }
	int WeaponWeight()				{ return SMG_WEIGHT; }
	int GetAvailableMods()			{ return MOD_NOMODS | MOD_SILENCER | MOD_SEMIAUTO; }
	const char *GetAmmoName()		{ return "57"; }

	float spreadXMax()				{ return 1.5; }
	float spreadYMax()				{ return 3.4; }
	float spreadXBase()				{ return 0.07; }
	float spreadYBase()				{ return 0.15; }
	float spreadXMod()				{ return 0.01; }
	float spreadYMod()				{ return 0.2; }
	int directionChange()			{ return 10; }
	float DropRecoilDelay()			{ return 0.2; }

	char *GetVModel()				{ return "models/weapons/p90/v_p90.mdl"; }
	char *GetWModel()				{ return "models/weapons/p90/w_p90.mdl"; }
	char *GetPModel()				{ return "models/weapons/p90/p_p90.mdl"; }
	char *GetShellModel()			{ return "models/weapons/shell_9mm.mdl"; }
	char *GetExtAnim()				{ return "mp5"; }

	char *GetFireSound()			{ return "weapons/p90/fire.wav"; }
	char *GetSilencedFireSound()	{ return "weapons/p90/fire_sil.wav"; }
	char *GetDeploySound()			{ return "weapons/draw_smg.wav"; }
};

class CBizon : public CBaseAMWeapon
{
public:
	// Overriden CBasePlayerWeapon methods
	int iItemSlot( void )			{ return 3; }

	// Overriden CBaseAMWeapon methods
	float ReloadTime()				{ return 3.4; }
	float AttackDelay()				{ return 0.05; }
	int FireVariations()			{ return 2; }
	int FireEmptyAnimation()		{ return 5; }
	int ReloadAnimation()			{ return 6; }
	Vector WeaponSpread()			{ return VECTOR_CONE_3DEGREES; }

	int WeaponID()					{ return WEAPON_BIZON; }
	char *WeaponClassName()			{ return "weapon_bizon"; }

	int BulletType()				{ return BULLET_PLAYER_BIZON; }
	int GetMaxCarry()				{ return _9MM_MAX_CARRY; }
	int GetMaxClipAmmo()			{ return 64; }
	int WeaponPosition()			{ return 2; }
	int WeaponWeight()				{ return SMG_WEIGHT; }
	int GetAvailableMods()			{ return MOD_NOMODS; }
	const char *GetAmmoName()		{ return "9mm"; }

	float spreadXMax()				{ return 1.5; }
	float spreadYMax()				{ return 3.4; }
	float spreadXBase()				{ return 0.07; }
	float spreadYBase()				{ return 0.15; }
	float spreadXMod()				{ return 0.01; }
	float spreadYMod()				{ return 0.15; }
	int directionChange()			{ return 10; }
	float DropRecoilDelay()			{ return 0.2; }

	char *GetVModel()				{ return "models/weapons/bizon/v_bizon.mdl"; }
	char *GetWModel()				{ return "models/weapons/bizon/w_bizon.mdl"; }
	char *GetPModel()				{ return "models/weapons/bizon/p_bizon.mdl"; }
	char *GetShellModel()			{ return "models/weapons/shell_9mm.mdl"; }
	char *GetExtAnim()				{ return "mp5"; }

	char *GetFireSound()			{ return "weapons/bizon/fire.wav"; }
	char *GetDeploySound()			{ return "weapons/draw_smg.wav"; }
};

//==================================================
// RIFLES
//==================================================

class CFAMAS : public CBaseAMWeapon
{
public:
	// Overriden CBasePlayerWeapon methods
	int iItemSlot( void )			{ return 4; }

	// Overriden CBaseAMWeapon methods
	float ReloadTime()				{ return 2.6; }
	float ReloadEmptyTime()			{ return 3.2; }
	float AttackDelay()				{ return 0.09; }
	int IdleVariations()			{ return 1; }
	int DeployAnimation()			{ return 2; }
	int FireAnimation()				{ return 3; }
	int FireEmptyAnimation()		{ return 7; }
	int FireVariations()			{ return 1; }
	int ReloadAnimation()			{ return 8; }
	int ReloadEmptyAnimation()		{ return 9; }
	Vector WeaponSpread()			{ return VECTOR_CONE_PERFECT; }

	int WeaponID()					{ return WEAPON_FAMAS; }
	char *WeaponClassName()			{ return "weapon_famas"; }

	int BulletType()				{ return BULLET_PLAYER_FAMAS; }
	int GetMaxCarry()				{ return _556_MAX_CARRY; }
	int GetMaxClipAmmo()			{ return 25; }
	int WeaponPosition()			{ return 2; }
	int WeaponWeight()				{ return RIFLE_WEIGHT; }
	int GetAvailableMods()			{ return MOD_NOMODS | MOD_REDDOT | MOD_SILENCER | MOD_BURSTATTACK | MOD_SEMIAUTO; }
	const char *GetAmmoName()		{ return "556"; }

	float spreadXMax()				{ return 7.75; }
	float spreadYMax()				{ return 3.75; }
	float spreadXBase()				{ return 0.3; }
	float spreadYBase()				{ return 0.15; }
	float spreadXMod()				{ return 0.05; }
	float spreadYMod()				{ return 0.035; }
	int directionChange()			{ return 10; }
	float DropRecoilDelay()			{ return 0.5; }

	char *GetVModel()				{ return "models/weapons/famas/v_famas.mdl"; }
	char *GetWModel()				{ return "models/weapons/famas/w_famas.mdl"; }
	char *GetPModel()				{ return "models/weapons/famas/p_famas.mdl"; }
	char *GetShellModel()			{ return "models/weapons/shell_556.mdl"; }
	char *GetExtAnim()				{ return "mp5"; }
	char *GetScopeModel()			{ return "models/weapons/famas/v_famas_scope.mdl"; }

	char *GetFireSound()			{ return "weapons/famas/fire-1.wav"; }
	char *GetFire2Sound()			{ return "weapons/famas/fire-2.wav"; }
	char *GetFire3Sound()			{ return "weapons/famas/fire-3.wav"; }
	char *GetSilencedFireSound()	{ return "weapons/famas/fire_sil-1.wav"; }
	char *GetSilencedFire2Sound()	{ return "weapons/famas/fire_sil-2.wav"; }
	char *GetSilencedFire3Sound()	{ return "weapons/famas/fire_sil-3.wav"; }
	char *GetDeploySound()			{ return "weapons/draw_rifle.wav"; }
};

class CM4A1 : public CBaseAMWeapon
{
public:
	// Overriden CBasePlayerWeapon methods
	int iItemSlot( void )			{ return 4; }

	// Overriden CBaseAMWeapon methods
	float DeployTime()				{ return 1.25; }
	float ReloadTime()				{ return 3.4; }
	float AttackDelay()				{ return 0.09; }
	int FireVariations()			{ return 2; }
	int FireEmptyAnimation()		{ return 5; }
	int ReloadAnimation()			{ return 6; }
	int ReloadEmptyAnimation()		{ return 7; }
	Vector WeaponSpread()			{ return VECTOR_CONE_PERFECT; }

	int WeaponID()					{ return WEAPON_M4A1; }
	char *WeaponClassName()			{ return "weapon_m4a1"; }

	int BulletType()				{ return BULLET_PLAYER_M4A1; }
	int GetMaxCarry()				{ return _556_MAX_CARRY; }
	int GetMaxClipAmmo()			{ return 30; }
	int WeaponPosition()			{ return 1; }
	int WeaponWeight()				{ return RIFLE_WEIGHT; }
	int GetAvailableMods()			{ return MOD_NOMODS | MOD_SILENCER | MOD_SEMIAUTO; }
	const char *GetAmmoName()		{ return "556"; }

	float spreadXMax()				{ return 7.75; }
	float spreadYMax()				{ return 3.0; }
	float spreadXBase()				{ return 0.3; }
	float spreadYBase()				{ return 0.15; }
	float spreadXMod()				{ return 0.05; }
	float spreadYMod()				{ return 0.1; }
	int directionChange()			{ return 10; }
	float DropRecoilDelay()			{ return 0.5; }

	char *GetVModel()				{ return "models/weapons/m4a1/v_m4a1.mdl"; }
	char *GetWModel()				{ return "models/weapons/m4a1/w_m4a1.mdl"; }
	char *GetPModel()				{ return "models/weapons/m4a1/p_m4a1.mdl"; }
	char *GetShellModel()			{ return "models/weapons/shell_556.mdl"; }
	char *GetExtAnim()				{ return "mp5"; }

	char *GetFireSound()			{ return "weapons/m4a1/fire.wav"; }
	char *GetSilencedFireSound()	{ return "weapons/m4a1/fire_sil.wav"; }
	char *GetDeploySound()			{ return "weapons/draw_rifle.wav"; }
};

class CAK47 : public CBaseAMWeapon
{
public:
	// Overriden CBasePlayerWeapon methods
	int iItemSlot( void )			{ return 4; }

	// Overriden CBaseAMWeapon methods
	float DeployTime()				{ return 1.25; }
	float ReloadTime()				{ return 3.0; }
	float AttackDelay()				{ return 0.09; }
	int IdleVariations()			{ return 1; }
	int DeployAnimation()			{ return 2; }
	int FireAnimation()				{ return 3; }
	int FireEmptyAnimation()		{ return 5; }
	int FireVariations()			{ return 1; }
	int ReloadAnimation()			{ return 6; }
	int ReloadEmptyAnimation()		{ return 7; }
	Vector WeaponSpread()			{ return VECTOR_CONE_PERFECT; }

	int WeaponID()					{ return WEAPON_AK47; }
	char *WeaponClassName()			{ return "weapon_ak47"; }

	int BulletType()				{ return BULLET_PLAYER_AK47; }
	int GetMaxCarry()				{ return _762_MAX_CARRY; }
	int GetMaxClipAmmo()			{ return 30; }
	int WeaponPosition()			{ return 0; }
	int WeaponWeight()				{ return RIFLE_WEIGHT; }
	int GetAvailableMods()			{ return MOD_NOMODS | MOD_REDDOT | MOD_SILENCER | MOD_SEMIAUTO; }
	const char *GetAmmoName()		{ return "762"; }

	float spreadXMax()				{ return 7.75; }
	float spreadYMax()				{ return 3.5; }
	float spreadXBase()				{ return 0.2; }
	float spreadYBase()				{ return 0.7; }
	float spreadXMod()				{ return 0.05; }
	float spreadYMod()				{ return 0.2; }
	int directionChange()			{ return 10; }
	float DropRecoilDelay()			{ return 0.5; }

	char *GetVModel()				{ return "models/weapons/ak47/v_ak47.mdl"; }
	char *GetWModel()				{ return "models/weapons/ak47/w_ak47.mdl"; }
	char *GetPModel()				{ return "models/weapons/ak47/p_ak47.mdl"; }
	char *GetShellModel()			{ return "models/weapons/shell_762.mdl"; }
	char *GetExtAnim()				{ return "mp5"; }
	char *GetScopeModel()			{ return "models/weapons/ak47/v_ak47_scope.mdl"; }

	char *GetFireSound()			{ return "weapons/ak47/fire.wav"; }
	char *GetSilencedFireSound()	{ return "weapons/ak47/fire_sil.wav"; }
	char *GetDeploySound()			{ return "weapons/draw_rifle.wav"; }
};

class CM249 : public CBaseAMWeapon
{
public:
	// Specific M249 methods and attributes
	void EXPORT ReloadSetBelt( void );

	// Overriden CBasePlayerWeapon methods
	BOOL Deploy( void );
	void PrimaryAttack( void );
	void Reload( void );
	void WeaponIdle( void );
	int iItemSlot( void )			{ return 4; }

	// Overriden CBaseAMWeapon methods
	float DeployTime()				{ return 1.25; }
	float ReloadTime()				{ return 8.4; }
	float AttackDelay()				{ return 0.1; }
	int DeployEmptyAnimation()		{ return 2; }
	int FireAnimation()				{ return 3; }
	int FireEmptyAnimation()		{ return 4; }
	int ReloadAnimation()			{ return 5; }
	int ReloadEmptyAnimation()		{ return 6; }
	Vector WeaponSpread()			{ return VECTOR_CONE_3DEGREES; }

	int WeaponID()					{ return WEAPON_M249; }
	char *WeaponClassName()			{ return "weapon_m249"; }

	int BulletType()				{ return BULLET_PLAYER_M249; }
	int GetMaxCarry()				{ return _556_MAX_CARRY; }
	int GetMaxClipAmmo()			{ return 100; }
	int WeaponPosition()			{ return 3; }
	int WeaponWeight()				{ return RIFLE_WEIGHT; }
	int GetAvailableMods()			{ return MOD_NOMODS; }
	const char *GetAmmoName()		{ return "556"; }

	float spreadXMax()				{ return 7.75; }
	float spreadYMax()				{ return 3.75; }
	float spreadXBase()				{ return 0.3; }
	float spreadYBase()				{ return 0.4; }
	float spreadXMod()				{ return 0.05; }
	float spreadYMod()				{ return 0.035; }
	int directionChange()			{ return 20; }
	float DropRecoilDelay()			{ return 0.5; }

	char *GetVModel()				{ return "models/weapons/m249/v_m249.mdl"; }
	char *GetWModel()				{ return "models/weapons/m249/w_m249.mdl"; }
	char *GetPModel()				{ return "models/weapons/m249/p_m249.mdl"; }
	char *GetShellModel()			{ return "models/weapons/shell_556.mdl"; }
	char *GetExtAnim()				{ return "mp5"; }

	char *GetFireSound()			{ return "weapons/m249/fire.wav"; }
	char *GetDeploySound()			{ return "weapons/draw_lmg.wav"; }
};

//==================================================
// SHOTGUNS
//==================================================

class CSPAS12 : public CBaseAMWeapon
{
public:
	// Overriden CBasePlayerWeapon methods
	int iItemSlot( void )			{ return 5; }

	// Overriden CBaseAMWeapon methods
	float ReloadTime()				{ return 0.8; }
	float AttackDelay()				{ return 1.15; }
	int DeployAnimation()			{ return 2; }
	int FireAnimation()				{ return 3; }
	int FireVariations() 			{ return 5; }
	int ReloadAnimation()			{ return 9; }
	BOOL SemiAutoModeOnly()			{ return TRUE; }
	Vector WeaponSpread()			{ return VECTOR_CONE_10DEGREES; }

	int WeaponID()					{ return WEAPON_SPAS12; }
	char *WeaponClassName()			{ return "weapon_spas12"; }

	int BulletType()				{ return BULLET_PLAYER_SPAS12; }
	int GetMaxCarry()				{ return BUCKSHOT_MAX_CARRY; }
	int GetMaxClipAmmo()			{ return 8; }
	int WeaponWeight()				{ return RIFLE_WEIGHT; }
	int GetAvailableMods()			{ return MOD_NOMODS; }
	const char *GetAmmoName()		{ return "buckshot"; }

	BOOL FireAsShotgun()			{ return TRUE; }
	BOOL ReloadAsPumpShotgun()		{ return TRUE; }

	float spreadXMax()				{ return 7.75; }
	float spreadYMax()				{ return 4.75; }
	float spreadXBase()				{ return 0.3; }
	float spreadYBase()				{ return 1.5; }
	float spreadXMod()				{ return 0.05; }
	float spreadYMod()				{ return 0.5; }
	int directionChange()			{ return 20; }
	float DropRecoilDelay()			{ return 0.75; }

	char *GetVModel()				{ return "models/weapons/spas12/v_spas12.mdl"; }
	char *GetWModel()				{ return "models/weapons/spas12/w_spas12.mdl"; }
	char *GetPModel()				{ return "models/weapons/spas12/p_spas12.mdl"; }
	char *GetShellModel()			{ return "models/weapons/shell_buckshot.mdl"; }
	char *GetExtAnim()				{ return "shotgun"; }

	char *GetFireSound()			{ return "weapons/spas12/fire.wav"; }
	char *GetDeploySound()			{ return "weapons/draw_shotgun.wav"; }
};

class CLessLethal : public CBaseAMWeapon
{
public:
	// Overriden CBasePlayerWeapon methods
	int iItemSlot( void )			{ return 5; }

	// Overriden CBaseAMWeapon methods
	float ReloadTime()				{ return 0.8; }
	float AttackDelay()				{ return 1.15; }
	int DeployAnimation()			{ return 2; }
	int FireAnimation()				{ return 3; }
	int FireVariations() 			{ return 5; }
	int ReloadAnimation()			{ return 9; }
	BOOL SemiAutoModeOnly()			{ return TRUE; }
	Vector WeaponSpread()			{ return VECTOR_CONE_10DEGREES; }

	int WeaponID()					{ return WEAPON_LESSLETHAL; }
	char *WeaponClassName()			{ return "weapon_shotgun_non_lethal"; }

	int BulletType()				{ return BULLET_PLAYER_LESSLETHAL; }
	int GetMaxCarry()				{ return BUCKSHOT_MAX_CARRY; }
	int GetMaxClipAmmo()			{ return 8; }
	int WeaponPosition() 			{ return 1; }
	int WeaponWeight()				{ return RIFLE_WEIGHT; }
	int GetAvailableMods()			{ return MOD_NOMODS; }
	const char *GetAmmoName()		{ return "buckshot"; }

	BOOL FireAsShotgun()			{ return TRUE; }
	BOOL ReloadAsPumpShotgun()		{ return TRUE; }

	float spreadXMax()				{ return 7.75; }
	float spreadYMax()				{ return 3.75; }
	float spreadXBase()				{ return 0.3; }
	float spreadYBase()				{ return 0.15; }
	float spreadXMod()				{ return 0.05; }
	float spreadYMod()				{ return 0.035; }
	int directionChange()			{ return 20; }
	float DropRecoilDelay()			{ return 0.5; }

	char *GetVModel()				{ return "models/weapons/lesslethal/v_lesslethal.mdl"; }
	char *GetWModel()				{ return "models/weapons/lesslethal/w_lesslethal.mdl"; }
	char *GetPModel()				{ return "models/weapons/lesslethal/p_lesslethal.mdl"; }
	char *GetShellModel()			{ return "models/weapons/shell_buckshot.mdl"; }
	char *GetExtAnim()				{ return "shotgun"; }

	char *GetFireSound()			{ return "weapons/lesslethal/fire.wav"; }
	char *GetDeploySound()			{ return "weapons/draw_shotgun.wav"; }
};

class CUSAS : public CBaseAMWeapon
{
public:
	// Overriden CBasePlayerWeapon methods
	int iItemSlot( void )			{ return 5; }

	// Overriden CBaseAMWeapon methods
	float ReloadTime()				{ return 3.0; }
	float AttackDelay()				{ return 0.15; }
	Vector WeaponSpread()			{ return VECTOR_CONE_10DEGREES; }

	int WeaponID()					{ return WEAPON_USAS; }
	char *WeaponClassName()			{ return "weapon_usas"; }

	int BulletType()				{ return BULLET_PLAYER_USAS; }
	int GetMaxCarry()				{ return BUCKSHOT_MAX_CARRY; }
	int GetMaxClipAmmo()			{ return 20; }
	int WeaponPosition()			{ return 2; }
	int WeaponWeight()				{ return RIFLE_WEIGHT; }
	int GetAvailableMods()			{ return MOD_NOMODS; }
	const char *GetAmmoName()		{ return "buckshot"; }

	BOOL FireAsShotgun()			{ return TRUE; }

	float spreadXMax()				{ return 7.75; }
	float spreadYMax()				{ return 4.75; }
	float spreadXBase()				{ return 0.3; }
	float spreadYBase()				{ return 2.15; }
	float spreadXMod()				{ return 0.05; }
	float spreadYMod()				{ return 0.035; }
	int directionChange()			{ return 20; }
	float DropRecoilDelay()			{ return 0.5; }

	char *GetVModel()				{ return "models/weapons/usas/v_usas.mdl"; }
	char *GetWModel()				{ return "models/weapons/usas/w_usas.mdl"; }
	char *GetPModel()				{ return "models/weapons/usas/p_usas.mdl"; }
	char *GetShellModel()			{ return "models/weapons/shell_buckshot.mdl"; }
	char *GetExtAnim()				{ return "shotgun"; }

	char *GetFireSound()			{ return "weapons/usas/fire.wav"; }
	char *GetDeploySound()			{ return "weapons/draw_shotgun.wav"; }
};

//==================================================
// SNIPERS
//==================================================

class CM14 : public CBaseAMWeapon
{
public:
	// Overriden CBasePlayerWeapon methods
	int iItemSlot( void )			{ return 6; }

	// Overriden CBaseAMWeapon methods
	float DeployTime()				{ return 1.25; }
	float ReloadTime()				{ return 3.5; }
	float AttackDelay()				{ return 1.0; }
	BOOL SemiAutoModeOnly()			{ return TRUE; }
	Vector WeaponSpread()			{ return VECTOR_CONE_1DEGREES; }

	int WeaponID()					{ return WEAPON_M14; }
	char *WeaponClassName()			{ return "weapon_m14"; }

	int BulletType()				{ return BULLET_PLAYER_M14; }
	int GetMaxCarry()				{ return _762_MAX_CARRY; }
	int GetMaxClipAmmo()			{ return 5; }
	int WeaponWeight()				{ return RIFLE_WEIGHT; }
	int GetAvailableMods()			{ return MOD_NOMODS | MOD_SCOPE; }
	const char *GetAmmoName()		{ return "762"; }

	float spreadXMax()				{ return 7.75; }
	float spreadYMax()				{ return 3.75; }
	float spreadXBase()				{ return 0.3; }
	float spreadYBase()				{ return 2.15; }
	float spreadXMod()				{ return 0.05; }
	float spreadYMod()				{ return 0.035; }
	int directionChange()			{ return 20; }
	float DropRecoilDelay()			{ return 0.5; }

	char *GetVModel()				{ return "models/weapons/m14/v_m14.mdl"; }
	char *GetWModel()				{ return "models/weapons/m14/w_m14.mdl"; }
	char *GetPModel()				{ return "models/weapons/m14/p_m14.mdl"; }
	char *GetShellModel()			{ return "models/weapons/shell_762.mdl"; }
	char *GetExtAnim()				{ return "mp5"; }
	char *GetScopeModel()			{ return "models/weapons/m14/v_m14_scope.mdl"; }

	char *GetFireSound()			{ return "weapons/m14/fire.wav"; }
	char *GetDeploySound()			{ return "weapons/draw_rifle.wav"; }
};

class CL96A1 : public CBaseAMWeapon
{
public:
	// Overriden CBasePlayerWeapon methods
	int iItemSlot( void )			{ return 6; }

	// Overriden CBaseAMWeapon methods
	float DeployTime()				{ return 1.35; }
	float ReloadTime()				{ return 2.7; }
	float AttackDelay()				{ return 1.9; }
	BOOL SemiAutoModeOnly()			{ return TRUE; }
	Vector WeaponSpread()			{ return VECTOR_CONE_1DEGREES; }

	int WeaponID()					{ return WEAPON_L96A1; }
	char *WeaponClassName()			{ return "weapon_l96a1"; }

	int BulletType()				{ return BULLET_PLAYER_L96A1; }
	int GetMaxCarry()				{ return _762_MAX_CARRY; }
	int GetMaxClipAmmo()			{ return 5; }
	int WeaponPosition()			{ return 1; }
	int WeaponWeight()				{ return RIFLE_WEIGHT; }
	int GetAvailableMods()			{ return MOD_NOMODS | MOD_SCOPE; }
	const char *GetAmmoName()		{ return "762"; }

	float spreadXMax()				{ return 7.75; }
	float spreadYMax()				{ return 3.75; }
	float spreadXBase()				{ return 0.3; }
	float spreadYBase()				{ return 0.15; }
	float spreadXMod()				{ return 0.05; }
	float spreadYMod()				{ return 0.035; }
	int directionChange()			{ return 20; }
	float DropRecoilDelay()			{ return 0.5; }

	char *GetVModel()				{ return "models/weapons/l96a1/v_l96a1.mdl"; }
	char *GetWModel()				{ return "models/weapons/l96a1/w_l96a1.mdl"; }
	char *GetPModel()				{ return "models/weapons/l96a1/p_l96a1.mdl"; }
	char *GetShellModel()			{ return "models/weapons/shell_762.mdl"; }
	char *GetExtAnim()				{ return "mp5"; }
	char *GetScopeModel()			{ return "models/weapons/l96a1/v_l96a1_scope.mdl"; }

	char *GetFireSound()			{ return "weapons/l96a1/fire.wav"; }
	char *GetDeploySound()			{ return "weapons/draw_rifle.wav"; }
};

//==================================================
// EXPLOSIVES
//==================================================

class CRPG : public CBaseAMWeapon
{
public:
	// Overriden CBasePlayerWeapon methods
	BOOL Deploy( void );
	void Precache( void );
	void PrimaryAttack( void );
	void Reload( void );
	void WeaponIdle( void );
	int iItemSlot( void )			{ return 7; }

	// Overriden CBaseAMWeapon methods
	float DeployTime()				{ return 1.25; }
	float ReloadTime()				{ return 6.0; }
	float AttackDelay()				{ return 1.5; }
	BOOL SemiAutoModeOnly()			{ return TRUE; }

	int WeaponID()					{ return WEAPON_RPG; }
	char *WeaponClassName()			{ return "weapon_rpg"; }

	int GetMaxCarry()				{ return ROCKET_MAX_CARRY; }
	int GetMaxClipAmmo()			{ return 1; }
	int WeaponWeight()				{ return EXPLOSIVE_WEIGHT; }
	int GetAvailableMods()			{ return MOD_NOMODS; }
	const char *GetAmmoName()		{ return "rockets"; }

	char *GetVModel()				{ return "models/weapons/rpg/v_rpg.mdl"; }
	char *GetWModel()				{ return "models/weapons/rpg/w_rpg.mdl"; }
	char *GetPModel()				{ return "models/weapons/rpg/p_rpg.mdl"; }
	char *GetExtAnim()				{ return "rpg"; }

	char *GetFireSound()			{ return "weapons/rpg/fire.wav"; }
	char *GetDeploySound()			{ return "weapons/draw_rifle.wav"; }
};

class CRPGRocket : public CGrenade
{
public:
	int	Save( CSave &save );
	int	Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];

	void Spawn( void );
	void Precache( void );
	void EXPORT IgniteThink( void );
	void EXPORT RocketThink( void );
	void EXPORT RocketTouch( CBaseEntity *pOther );
	static CRPGRocket *CreateRPGRocket( Vector vecOrigin, Vector vecAngles, CBaseEntity *pOwner, CRPG *pLauncher );

	int m_iTrail;
	float m_flIgniteTime;
};

//=========================================================
// CWeaponBox - a single entity that can store weapons
// and ammo.
//=========================================================
class CWeaponBox : public CBaseEntity
{
	void Precache( void );
	void Spawn( void );
	void Touch( CBaseEntity *pOther );
	void KeyValue( KeyValueData *pkvd );
	BOOL IsEmpty( void );
	int  GiveAmmo( int iCount, char *szName, int iMax, int *pIndex = NULL );
	void SetObjectCollisionBox( void );

public:
	void EXPORT Kill ( void );
	int		Save( CSave &save );
	int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	BOOL HasWeapon( CBasePlayerItem *pCheckItem );
	BOOL PackWeapon( CBasePlayerItem *pWeapon );
	BOOL PackAmmo( int iszName, int iCount );

	CBasePlayerItem	*m_rgpPlayerItems[MAX_ITEM_TYPES];// one slot for each

	int m_rgiszAmmo[MAX_AMMO_SLOTS];// ammo names
	int	m_rgAmmo[MAX_AMMO_SLOTS];// ammo quantities

	int m_cAmmoTypes;// how many ammo types packed into this box (if packed by a level designer)
};

#ifdef CLIENT_DLL
bool bIsMultiplayer ( void );
void LoadVModel ( char *szViewModel, CBasePlayer *m_pPlayer );
#endif

class CLaserSpot : public CBaseEntity
{
	void Spawn( void );
	void Precache( void );

	int	ObjectCaps( void ) { return FCAP_DONT_SAVE; }

public:
	void Suspend( float flSuspendTime );
	void EXPORT Revive( void );

	static CLaserSpot *CreateSpot( void );
};

#endif // WEAPONS_H
