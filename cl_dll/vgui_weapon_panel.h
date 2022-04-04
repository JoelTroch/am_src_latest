using namespace vgui;

#include "..\game_shared\vgui_loadtga.h"

//255 non-visible, 0 fully visible

#ifndef M_PI
#define M_PI		3.14159265358979323846	// matches value in gcc v2 math.h
#endif

#define MIN_BUTTON_ALPHA	127.0//127
#define ALPHA_VISIBLE		0.0
#define ALPHA_INVISIBLE		255.0
#define BUTTON_SPEED		5

#define POP_UP_TIME			200

#define MAX_MENU_BUTTONS	8

//this must be shared!
#define	MOD_SCOPE			1
#define MOD_SILENCER		2
#define MOD_REDDOT			3
#define MOD_EXTENDED_MAG	4
#define MOD_GL				5
#define MOD_LASER			6
#define MOD_FIREMODE		7

//i'd like to use bit flags but I need to be 100% this is going to work server/client wise
#define MOD_NOT_AVAILABLE	0
#define MOD_ON				1
#define MOD_OFF				2

//the handle
struct oModHandle
{
	int iOrder;//determines the priority
	int iValue;//if on, off, not available
};

//the modifiers

struct oModifiers
{
	oModHandle sScope;
	oModHandle sSilencer;
	oModHandle sRedDot;
	oModHandle sExtendedMag;
	oModHandle sGL;
	oModHandle sLaser;
	oModHandle sFireMode;
};

//==================================
// CMyButton
//==================================
class CMySpecialButton : public CommandButton
{
public:
	BitmapTGA	*m_pTGA;
	int iAlpha;
	bool bReady;//determines whether the button is ready to click
	oModifiers myModType;//like pretty much before, but used internally
	bool bSndSend;

	float desired_x;
	float desired_y;

//	int iHelper;//a pop up label

	CMySpecialButton( const char* text,int x,int y,int wide,int tall, bool bNoHighlight = false):CommandButton(text,x,y,wide,tall, false, false)
	{
		setPos( 0, 0 );//in case a stupid forgot

	//	setContentFitted(true);//to fit the button to the current tga size - EDIT: DOESN'T WORK PROPERLY, USE GETSIZE INSTEAD

		setPaintBackgroundEnabled(false);//to not show solid colours
		iAlpha = ALPHA_INVISIBLE;//to prevent alpha ping-pong when first started
		bReady = false;//meh, just to be sure
		bSndSend = 0;
//		iHelper = 0;
	}
	~CMySpecialButton() 
	{
		if( m_pTGA )
		delete m_pTGA;
	}

	void paint()
	{
		//this plays around with TGAs. Be sure there's one of those.
		if (!m_pTGA)
			return;

		if ( bReady )
		{
			if (isArmed())
			{
				if( bSndSend == 0 )
				{
					PlaySound("common/wpn_moveselect.wav", 1);
					bSndSend = 1;
				}

				if (iAlpha > ALPHA_VISIBLE)
					iAlpha -= (gHUD.m_flTimeDelta * 1000);

				if (iAlpha < ALPHA_VISIBLE)
					iAlpha = ALPHA_VISIBLE;

			/*	if (iHelper < POP_UP_TIME)
					iHelper += gHUD.m_flTimeDelta * 133;

				if (iHelper > POP_UP_TIME)
					iHelper = POP_UP_TIME;*/
			//	setBorder(new LineBorder(1, Color(HUD_COLOR, 0)));//DEBUG PURPOSES
			}//
			else
			{
				if (iAlpha < MIN_BUTTON_ALPHA)
					iAlpha += (gHUD.m_flTimeDelta * 500);

				if (iAlpha > MIN_BUTTON_ALPHA)
					iAlpha = MIN_BUTTON_ALPHA;

//				iHelper = -1;

			//	setBorder(NULL);//DEBUG PURPOSES
			}
		}
		else//button is just created, fade it slooooowly
		{
			if (iAlpha > MIN_BUTTON_ALPHA)
				iAlpha -= (gHUD.m_flTimeDelta * 300);

			if (iAlpha < MIN_BUTTON_ALPHA)
				iAlpha = MIN_BUTTON_ALPHA;

			if (iAlpha == MIN_BUTTON_ALPHA)
				bReady = true;//yay!
		}

		m_pTGA->doPaint(this);		
	}

	void internalCursorExited()
	{
		setSelected(false);
		setArmed(false);//when mouse is over it, it sets stays armed forever, lets prevent that
		bSndSend = 0;
	}
};


class CWeaponMenuPanel : public Panel, public ActionSignal//, public ICheckButton2Handler
{
public:
	CWeaponMenuPanel();
	~CWeaponMenuPanel();

	void actionPerformed(Panel* panel);
	int KeyInput(int down, int keynum, const char *pszCurrentBinding);

	void paint();
	void Init();

	oModifiers sWeaponModifiers;

	int iNumButtons;

private:
	void CloseWindow(void);

//	CMyButton* mbutton;
//	CMyButton* mbutton_TEST;
	CMySpecialButton* mbutton_close;//button for close the window
	
	Label* label;//common label for all buttons
//	Label* helper;//a pop up helper - not used

	CMySpecialButton* MenuButtons[MAX_BUTTONS];
};
