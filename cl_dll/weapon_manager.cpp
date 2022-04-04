#include "hud.h"
#include "cl_util.h"
#include "const.h"

#include "weapon_manager.h"

#include "com_model.h"
#include "gl_texloader.h"

CWeaponManager gWpnMgr;
ammotype_t ammo_types[MAX_AMMOTYPES];
/*
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
*/
void CWeaponManager::AmmoInit()
{
	ammo_types[AMMO_9MM].name = "9mm";
	ammo_types[AMMO_50].name = "50";
	ammo_types[AMMO_45].name = "45";
	ammo_types[AMMO_44].name = "44";
	ammo_types[AMMO_57].name = "57";
	ammo_types[AMMO_556].name = "556";
	ammo_types[AMMO_762].name = "762";

	char texName[256];
	for (int i = 0; i < MAX_AMMOTYPES; i++)
	{
		ammo_types[i].ammo = 0;
		ammo_types[i].index = i;

		sprintf(texName, "gfx/ammo/%s.tga", ammo_types[i].name);

		if (!ammo_types[i].gl_textureid)
			ammo_types[i].gl_textureid = CreateTexture(texName, MIPS_YES);
	}
}

CWeaponManager::CWeaponManager()
{

}

CWeaponManager :: ~CWeaponManager()
{

}

void CWeaponManager::VidInit()
{
	AmmoInit();
}