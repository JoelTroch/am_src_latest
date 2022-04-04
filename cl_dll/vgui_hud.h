// ======================================
// Paranoia vgui hud header file
// written by BUzer.
// ======================================

#ifndef _VGUIHUD_H
#define _VGUIHUD_H
using namespace vgui;

#define MAX_ICONS 4// sys

#define NUM_WEAPON_ICONS 32

//#define MAX_RAINDROPS 2// sys
#define MAX_RAINDROP_TYPES 20//the different images

#define MAX_BLOOD_TYPES 3//5 the different images -edit lets use 3

class ImageHolder;

struct sDropContainer
{
	CImageLabel *m_pRainDropImage[MAX_RAINDROP_TYPES];
	float fRainDropAlpha;
};

struct sBloodContainer
{
	CImageLabel *m_pBloodImage[MAX_BLOOD_TYPES];
	float fBloodAlpha;
};

class CHud2 : public Panel
{
public:
    CHud2();
	~CHud2();
	void Initialize();
	
	void UpdateHealth (int newHealth);
	void UpdateArmor (int newArmor);
	void UpdateSlowMotion (int newSlowMotion);
	void UpdateLevel (int newLevel);

	void ResetOverlays();

//	void Think();
	virtual void solve();

//	float m_fHUDAlpha;
	int iLevelNotified;
	bool allowed_to_print_msg;
	int iTextLife;

protected:
	virtual void paintBackground(); // per-frame calculations and checks
	virtual void paint();

protected:
	sDropContainer Raindrops[MAX_RAINDROP_TYPES];
	sBloodContainer Bloods[MAX_BLOOD_TYPES];

	// Wargon: »конка юза.
	CImageLabel *m_pUsageIcon;

//	CImageLabel *pBuddyIcons[MAX_ICONS];

	CWeaponsSelPanel		*m_pHudWeaponsSelPanel;

	float m_fUsageUpdateTime;

	// painkiller icon
//	CImageLabel	*m_pMedkitsIcon;
//	Label		*m_pMedkitsCount;
//	int		m_pMedkitsOldNum;
//	float	m_fMedkitUpdateTime;

	// health and armor bars
	ImageHolder *m_pBitmapHealthFull;
	ImageHolder *m_pBitmapHealthEmpty;
	ImageHolder *m_pBitmapHealthFlash;
/*
	ImageHolder *m_pBitmapArmorFull;
	ImageHolder *m_pBitmapArmorEmpty;
	ImageHolder *m_pBitmapArmorFlash;
	*/

	ImageHolder *m_pBitmapSlowMotionFull;
	ImageHolder *m_pBitmapSlowMotionEmpty;
	ImageHolder *m_pBitmapSlowMotionFlash;

	ImageHolder *m_pBitmapLevelFull;
	ImageHolder *m_pBitmapLevelEmpty;

/*	ImageHolder *m_pHealthIcon;
	ImageHolder *m_pArmorIcon;
	ImageHolder *m_pSlowMotionIcon;*/

	int m_iHealthBarWidth, m_iHealthBarHeight;
	int m_iHealthBarXpos, m_iHealthBarYpos;
//	int m_iArmorBarWidth, m_iArmorBarHeight;
//	int m_iArmorBarXpos, m_iArmorBarYpos;

	int m_iSlowMotionBarWidth, m_iSlowMotionBarHeight;
	int m_iSlowMotionBarXpos, m_iSlowMotionBarYpos;

	int m_iLevelBarWidth, m_iLevelBarHeight;
	int m_iLevelBarXpos, m_iLevelBarYpos;

	BitmapTGA* m_pWeaponIconsArray[NUM_WEAPON_ICONS];
	BitmapTGA* FindAmmoImageForWeapon(const char *wpn);

	Font		*m_pFontSystem;
	Font		*m_pFontDigits;
//	Font		*m_pFontHUDBars;
	Font		*m_pFontHUD_BC;

	int health, armor, slowmotion, level;
	int oldhealth, oldarmor, oldslowmotion, oldlevel;
	float m_fHealthUpdateTime, m_fArmorUpdateTime, m_fSlowMotionUpdateTime, m_fLevelUpdateTime;
};

void Hud2Init();

#endif // _VGUIHUD_H