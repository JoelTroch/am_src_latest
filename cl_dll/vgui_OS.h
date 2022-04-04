using namespace vgui;

#include "..\game_shared\vgui_loadtga.h"
#include<VGUI_TextEntry.h>
#include "..\game_shared\vgui_checkbutton2.h"


#define ALPHA_VISIBLE		0.0
#define ALPHA_INVISIBLE		255.0
#define BUTTON_SPEED		5

/*
#define KEYPAD_CMD_NONE				90
#define	KEYPAD_CLOSE_ALL			91
#define KEYPAD_CLEAR				92
*/


#define MIN_OS_BUTTON_ALPHA	64.0//127

#define ALPHA_VISIBLE		0.0
#define ALPHA_INVISIBLE		255.0
#define BUTTON_SPEED		5



#define WINDOW_HANDLE_H		20
#define WINDOW_MENU_H		20

#define WINDOW_COLOR_MENU	220, 220, 220
#define WINDOW_COLOR_BODY	240, 240, 240


#define MENU_ELEMENT_H 20
#define MENU_ELEMENT_W 120

class CMyMenuButton : public CommandButton
{
public:
	//	CMyMenuButton(const char* text, int x, int y) : Button(text, x, y)
	CMyMenuButton(const char* text, int x, int y, int wide, int tall, bool bNoHighlight = false) :CommandButton(text, x, y, wide, tall, false, false)
	{
		CSchemeManager *pSchemes = gViewPort->GetSchemeManager();
		SchemeHandle_t hTextScheme = pSchemes->getSchemeHandle("OS");
		Font *pDefFont = pSchemes->getFont(hTextScheme);

		setPaintBackgroundEnabled(true);

		Label* menu_label;
		menu_label = new Label("", 0, 0, 0, ALPHA_VISIBLE);

		//main label, shows a pretty title (not the full path)
		if (menu_label)
		{
			menu_label->setParent(this);

			menu_label->setFont(pDefFont);
			menu_label->setPaintBackgroundEnabled(false);
			menu_label->setContentFitted(true);

			menu_label->setFgColor(255, 255, 255, ALPHA_VISIBLE);

			menu_label->setText(text);
			menu_label->setPos(20, 0);

			menu_label->setVisible(true);//to prevent show the button at the corner on the very first frame
		}
	}

	void paintBackground();

	/*	void paintBackground()
	{
	if (isArmed())
	{
	drawSetColor(255, 0, 0, 150);
	drawFilledRect(0, 0, getWide(), getTall());

	drawSetColor(255, 0, 0, 150);
	drawOutlinedRect(0, 0, getWide()-1, getTall()-1);

	drawSetColor(255, 0, 0, 150);
	drawOutlinedRect(1, 1, getWide(), getTall());
	}
	else
	{
	drawSetColor(222, 222, 222, 150);
	drawFilledRect(0, 0, getWide(), getTall());

	drawSetColor(222, 222, 222, 150);
	drawOutlinedRect(0, 0, getWide()-1, getTall()-1);

	drawSetColor(222, 222, 222, 150);
	drawOutlinedRect(1, 1, getWide(), getTall());
	}
	}*/

	//	void internalCursorExited();

	void internalCursorExited()
	{
		setSelected(false);
		setArmed(false);//when mouse is over it, it sets stays armed forever, lets prevent that
	}
};

class COpSys;
//if there's a button, must be this pretty one
class CMyOSButton : public CommandButton
{
public:
	COpSys* pOS;

	BitmapTGA	*m_pTGA;
	int iAlpha;
	bool bReady;//determines whether the button is ready to click
	bool bSndSend;
	bool bFile;

	CMyOSButton(const char* text, int x, int y, int wide, int tall, bool bNoHighlight = false) :CommandButton(text, x, y, wide, tall, false, false)
	{
		setPos(0, 0);//in case a stupid forgot

		setPaintBackgroundEnabled(false);//to not show solid colours
		iAlpha = ALPHA_INVISIBLE;//to prevent alpha ping-pong when first started
		bReady = false;//meh, just to be sure
		bSndSend = 0;
	}
	~CMyOSButton()
	{
		if (m_pTGA)
			delete m_pTGA;
	}

	void paint();
	/*	{
	//this plays around with TGAs. Be sure there's one of those.
	if (!m_pTGA)
	return;

	m_pTGA->setColor( Color(255,255,255, iAlpha) );

	if ( bReady )
	{
	if (isArmed())
	{
	if( bSndSend == 0 )
	{

	bSndSend = 1;
	}

	if (iAlpha > ALPHA_VISIBLE)
	iAlpha -= (gHUD.m_flTimeDelta * 800);

	if (iAlpha < ALPHA_VISIBLE)
	iAlpha = ALPHA_VISIBLE;
	}
	else
	{
	if (iAlpha < MIN_OS_BUTTON_ALPHA)
	iAlpha += (gHUD.m_flTimeDelta * 800);

	if (iAlpha > MIN_OS_BUTTON_ALPHA)
	iAlpha = MIN_OS_BUTTON_ALPHA;
	}
	}
	else//button is just created, fade it slooooowly
	{
	if (iAlpha > MIN_OS_BUTTON_ALPHA)
	iAlpha -= (gHUD.m_flTimeDelta * 500);

	if (iAlpha < MIN_OS_BUTTON_ALPHA)
	iAlpha = MIN_OS_BUTTON_ALPHA;

	if (iAlpha == MIN_OS_BUTTON_ALPHA)
	bReady = true;//yay!
	}

	m_pTGA->doPaint(this);
	}*/

	void internalCursorExited()
	{
		setSelected(false);
		setArmed(false);//when mouse is over it, it sets stays armed forever, lets prevent that
		bSndSend = 0;
	}
};

class COSWindow : public DragNDropPanel, public ActionSignal///a dragable window - closes itself - displays app launchers (folder or files)
{
public:
	bool bIsCamera;

	COSWindow(char* title, char* path, int x, int y, int w, int h, int win_id) : DragNDropPanel(x, y, w, h)
	{
		bIsCamera = false;

		strcpy(szWndTitle, title);

		CSchemeManager *pSchemes = gViewPort->GetSchemeManager();
		SchemeHandle_t hTextScheme = pSchemes->getSchemeHandle("OS_large");
		Font *pDefFont = pSchemes->getFont(hTextScheme);

		SchemeHandle_t hTextScheme_OS = pSchemes->getSchemeHandle("OS");
		Font *pOSFontSmall = pSchemes->getFont(hTextScheme_OS);

		setBgColor(0, 0, 255, ALPHA_INVISIBLE);//to prevent the first frame flash

		setBorder(new LineBorder(1, Color(0, 0, 0, 0)));

		window_label = new Label(title, 20, -5, 0, ALPHA_VISIBLE);

		//main label, shows a pretty title (not the full path)
		if (window_label)
		{
			window_label->setParent(this);
			window_label->setFont(pDefFont);
			window_label->setPaintBackgroundEnabled(false);
			window_label->setContentFitted(true);

			window_label->setFgColor(255, 255, 255, ALPHA_VISIBLE);

			window_label->setText(title);

			window_label->setVisible(true);//to prevent show the button at the corner on the very first frame
		}

		window_button_close = new CMyOSButton("", 0, 0, 0, ALPHA_VISIBLE);

		int close_button_w, close_button_h;

		if (window_button_close)
		{
			window_button_close->setParent(this);

			window_button_close->m_pTGA = vgui_LoadTGA("scripts/OS/cross.tga");
			window_button_close->setImage(window_button_close->m_pTGA);

			window_button_close->m_pTGA->getSize(close_button_w, close_button_h);
			window_button_close->setSize(close_button_w, close_button_h);


			window_button_close->addActionSignal(this);//closes the window

			window_button_close->setPos(w - close_button_w, WINDOW_HANDLE_H / 2 - close_button_h / 2);
		}

		//icon folder
		window_button_icon = new CImageLabel("", 0, 0, 0, ALPHA_VISIBLE);

		if (window_button_icon)
		{
			window_button_icon->setParent(this);

			window_button_icon->m_pTGA = vgui_LoadTGA("scripts/OS/window_icon.tga");
			window_button_icon->setImage(window_button_icon->m_pTGA);

			window_button_icon->m_pTGA->getSize(close_button_w, close_button_h);
			window_button_icon->setSize(close_button_w, close_button_h);

			window_button_icon->setPos(0, WINDOW_HANDLE_H / 2 - close_button_h / 2);
		}


		//menu
		window_panel_menu = new Panel(0, 0, 0, ALPHA_VISIBLE);

		if (window_panel_menu)
		{
			window_panel_menu->setParent(this);

			window_panel_menu->setSize(w, h - WINDOW_MENU_H);
			window_panel_menu->setPos(0, WINDOW_MENU_H);

			window_panel_menu->setBgColor(WINDOW_COLOR_MENU, ALPHA_VISIBLE);

			window_panel_menu->setBorder(new LineBorder(1, Color(0, 0, 0, 0)));
		}


		//window's body
		window_panel_body = new Panel(0, 0, 0, ALPHA_VISIBLE);

		if (window_panel_body)
		{
			window_panel_body->setParent(this);

			window_panel_body->setSize(w, h - WINDOW_HANDLE_H * 2);
			window_panel_body->setPos(0, WINDOW_HANDLE_H * 2);

			window_panel_body->setBgColor(WINDOW_COLOR_BODY, ALPHA_VISIBLE);

			window_panel_body->setBorder(new LineBorder(1, Color(0, 0, 0, 0)));
		}


		//label that shows current directory
		window_label_directory = new Label(title, 20, 0, 0, ALPHA_VISIBLE);

		if (window_label_directory)
		{
			window_label_directory->setParent(window_panel_menu);
			window_label_directory->setFont(pOSFontSmall);
			window_label_directory->setPaintBackgroundEnabled(false);
			window_label_directory->setContentFitted(true);

			window_label_directory->setFgColor(255, 255, 255, ALPHA_VISIBLE);
		}



		//TODO: CHANGE THIS BUTTON TO "UNDO" OR SOMETHING LIKE
		window_button_up = new CMyOSButton("", 0, 0, 0, ALPHA_VISIBLE);

		if (window_button_up)
		{
			int tga_size_x, tga_size_y;

			window_button_up->setParent(window_panel_menu);

			window_button_up->m_pTGA = vgui_LoadTGA("scripts/OS/back.tga");
			window_button_up->setImage(window_button_up->m_pTGA);

			window_button_up->m_pTGA->getSize(tga_size_x, tga_size_y);
			window_button_up->setSize(tga_size_x, tga_size_y);

			window_button_up->setPos(0, WINDOW_MENU_H / 2 - tga_size_x / 2);
		}


		int tga_size_x, tga_size_y;

		image_os_logo_black = new CImageLabel("", 0, 0);

		if (image_os_logo_black)
		{
			image_os_logo_black->setParent(this);

			char image[64];
			sprintf(image, "scripts/OS/logo_black.tga");

			image_os_logo_black->m_pTGA = vgui_LoadTGA(image);

			image_os_logo_black->setImage(image_os_logo_black->m_pTGA);

			image_os_logo_black->m_pTGA->getSize(tga_size_x, tga_size_y);
			image_os_logo_black->setSize(tga_size_x, tga_size_y);

			image_os_logo_black->setPos(this->getWide() / 2 - tga_size_x / 2, this->getTall() / 2 - tga_size_y / 2 - WINDOW_HANDLE_H * 2);

			image_os_logo_black->setVisible(false);//dont show yet
		}

		fAlpha = ALPHA_INVISIBLE;
	}

	void actionPerformed(Panel* panel);
	void CloseWindow(void);
	void SetSize(){ return; };

	void paint();
	void Init();
	void doCreateLOGInButton();
	void doDeniedButtons();
	void doInfoWindow(char * sz_msg);
	void doStadisticsWindow();

	COpSys* pOS;

	char szWndTitle[64];
	float fAlpha;

private:

	CMyOSButton* window_button_close;//button for close the window
	CMyOSButton* window_button_up;//goes back
	CMyOSButton* window_button_logon;
	CMyOSButton* window_button_error;

	CImageLabel* image_os_logo_black;
	CImageLabel* window_button_icon;//button for close the window

	Label* window_label;
	Label* window_label_directory;
	Panel* window_panel_body;
	Panel* window_panel_menu;
};

class COpSys : public Panel, public ICheckButton2Handler//desktop
{
public:
	COpSys();
	~COpSys();

	int KeyInput(int down, int keynum, const char *pszCurrentBinding);
	void ActionSignal(int cmd);

	void OSOpenAPP(char* full_path_to_app);

	void paint();
	bool Init();

	void OSCreateWindow(char* title, char* path, int w, int h, int win_id);
	void OSCreateAPP(int x, int y, char* icon_name, char* icon_label, char* full_path, Panel* pParent);
	void OSCreateContents(char* path, Panel* pParent);
	void OSSetBaseDirectory(char * path);

	void OSAPPTextViewer(char* full_path_to_file);
	void OSAPPImageViewer(char* full_path_to_file);

	void OSAPPLogIN();

	void OSMenuShow(CMyOSButton* button);
	void OSMenuHide();

	int iOpenedWindows;

	char szCurrentOSPath[256];
	char szOldOSPath[256];

	char szPCName[256];

	char szLogUser[64];
	char szLogPassword[64];
	int iFinalCode;

	char szDirectoryBase[512];

	char pc_id[64];
	char pc_settings[64];
	char wallpaper_image[64];
	int desktop_color[3];
	int label_color[3];
	int window_default_size[2];

	bool bArabic;
	bool bInitFailed;

	void StateChanged(CCheckButton2 *pButton){};
	CCheckButton2* pSwitch;

	float fDelay;

	bool bLogged;
	bool bCameraActive;

	//	bool bMenuActive;

private:
	void CloseWindow(void);

	CImageLabel *	my_wallpaper;
	CImageLabel *	my_image_logo_white;
	Label*	my_label_current_path;
	Label*	my_label_time;
	CMyOSButton * my_button_power;

	int		m_iMaxScrollValue;

	TextEntry	* entry_user;
	TextEntry	* entry_pass;

	Label*	my_label_username;
	Label*	my_label_password;
	COSWindow* my_window_login;
	COSWindow* my_window_camera;

	Panel* my_panel_desktop;

	CMyMenuButton* menu_button_cut;
	CMyMenuButton* menu_button_copy;
	CMyMenuButton* menu_button_paste;
	CMyMenuButton* menu_button_delete;

	float fAlpha;
	//	int iTimeMenuShow;
};

class COpSysHandler_Command : public ActionSignal
{

private:
	COpSys * m_pFather;
	int		 m_cmd;

public:
	COpSysHandler_Command(COpSys * panel, int cmd)
	{
		m_pFather = panel;
		m_cmd = cmd;
	}

	virtual void actionPerformed(Panel * panel)
	{
		m_pFather->ActionSignal(m_cmd);
	}
};

class command_open : public ActionSignal
{

private:
	COpSys * m_pFather;
	char folder[256];

public:
	command_open(COpSys * panel, char* path)
	{
		m_pFather = panel;

		strcpy(folder, path);
	}

	virtual void actionPerformed(Panel * panel)
	{
		m_pFather->OSOpenAPP(folder);
	}
};