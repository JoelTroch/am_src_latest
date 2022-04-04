#ifndef __WEAPON_MGR__
#define __WEAPON_MGR__

#define MAX_AMMOTYPES 16
typedef struct ammotype_t
{
	int index;
	const char *name;
	int ammo;

	int gl_textureid;
};

class CClientWeapon
{
public:
	int m_iId;

	//anims.
	int shot_anim;
	int deploy_anim;
	int reload_anim;
	int idle_anim;

	int ammoType;

	int m_iClip;
	int m_iAmmoInBack;

	float flCycle;
	float flReloadTime;

	float nextAttack;

	bool isSemi;
};

class CWeaponManager
{
public:
	CWeaponManager();
	~CWeaponManager();
	CClientWeapon *head;
	CClientWeapon *curWeapon();

	void VidInit();
	void AmmoInit();

	void AddNewWeapon(CClientWeapon *wpn);
	void ParseWeapon(char *weapon);
	void ParseFile(char *fileName);

	void SetAmmoInBack(int ammo, int type);
};

extern CWeaponManager gWpnMgr;
extern ammotype_t ammo_types[MAX_AMMOTYPES];

enum ammotypes_e
{
	AMMO_NONE = 0,
	AMMO_9MM,
	AMMO_50,
	AMMO_45,
	AMMO_44,
	AMMO_57,
	AMMO_556,
	AMMO_762,

};
#endif