#include "hud.h"
#include "cl_util.h"
#include "vgui_TeamFortressViewport.h"
#include "vgui_OS.h"
#include "VGUI_LineBorder.h"
#include "VGUI_TextImage.h"
#include "keydefs.h"
#include "triangleapi.h"

#include "r_studioint.h"
#include "com_model.h"

#include <io.h>
#include<VGUI_LoweredBorder.h>
#include<VGUI_EtchedBorder.h>

#include <stdio.h>

#include "string.h" 
#include <fstream>
#include <iostream>
#include "fmod_engine.h"

using namespace std;

char szWindowTitle[256];
bool bMenuActive = false;
int iTimeMenuShow;

extern int FindNextTag(char* &ptext);
extern int GetTagName(char* &ptext, char* bufTagName);
extern int GetTagParameter(char* &ptext, char* bufParamName, char* bufParamValue);
extern void ParseColor(char* ptext, int &r, int &g, int &b, int &a);


char* extract_folder_name(char *src)//removes full path
{
	int num_slashes = 0;
	int slashes = 0;
	int	iChars = 0;

	int str = strlen(src);
		gEngfuncs.Con_Printf ( "strlen - %i\n", str );

	if( src[str-1] == '/' )//last character is a slash, then its a path
		gEngfuncs.Con_Printf ( "extract_folder_name FOLDER - %s\n", src );
	else
	{
		gEngfuncs.Con_Printf ( "extract_folder_name NOT A FOLDER - %s\n", src );
	}

	//count how many slashes the path has
	for (int i = 0; i<strlen(src); i++)
	{
		if( src[i] == '/' )
		num_slashes++;
	}

	//look for the one before the last flash
	for (int p = 0; p<strlen(src); p++)
	{
		if( src[p] == '/' )
		{
			slashes++;

			if(slashes == num_slashes-1)
			{
				iChars = p;
			}
		}
	}

	//that's the window's name, copy it but ignore first '/' and last '/'
 	for ( int j = iChars; j<strlen(src)-1; j++)
	{
		szWindowTitle[j-iChars-1] = src[j];
	}
	
	//print end of string - end
	szWindowTitle[strlen( src ) - iChars-2] = '\0';

	gEngfuncs.Con_Printf("szWindowTitle %s\n", szWindowTitle );

	//window title is cooked up and ready for use!
	return szWindowTitle;
}


class CMyOSSlider : public Slider
{
public:
	CMyOSSlider(int x,int y,int wide,int tall,bool vertical) : Slider(x,y,wide,tall,vertical){};
	
	void paintBackground( void )
	{
		int wide,tall,nobx,noby;
		getPaintSize(wide,tall);
		getNobPos(nobx,noby);

		//background
//		drawSetColor(HUD_COLOR, 150);
		drawSetColor(0, 0, 0, 150);
		drawFilledRect( 0,0,wide,tall );

		// nob
//		drawSetColor(HUD_COLOR, 0);
		drawSetColor(0, 0, 0, 150);
		drawFilledRect( 0,nobx,wide,noby );

		drawSetColor(0, 0, 0, 150);
		drawOutlinedRect( 0,nobx,wide-1,noby-1 );

		drawSetColor(255, 255, 255, 150);
		drawOutlinedRect( 1,nobx+1,wide,noby );

		drawSetColor(0, 0, 0, 0);
		drawOutlinedRect( 0,nobx,wide,noby );
	}
};


class CMyOSScrollbutton : public Button
{
public:
	CMyOSScrollbutton(int up, int x, int y) : Button("", x, y, 16, 16)
	{
		if (up)
			setImage(vgui_LoadTGA("gfx/vgui/arrowup.tga"));
		else
			setImage(vgui_LoadTGA("gfx/vgui/arrowdown.tga")); 

		setPaintEnabled(true);
		setPaintBackgroundEnabled(true);
	}

	void paintBackground()
	{
		if (isSelected())
		{
	//		drawSetColor(HUD_COLOR_PRESSED, 0);
			drawSetColor(255, 255, 255, 150);
			drawFilledRect(0, 0, getWide(), getTall());

			drawSetColor(255, 255, 255, 150);
			drawOutlinedRect(0, 0, getWide()-1, getTall()-1);

			drawSetColor(0, 0, 0, 150);
			drawOutlinedRect(1, 1, getWide(), getTall());
		}
		else
		{
//			drawSetColor(HUD_COLOR, 0);
			drawSetColor(0, 0, 0, 150);
			drawFilledRect(0, 0, getWide(), getTall());

			drawSetColor(0, 0, 0, 150);
			drawOutlinedRect(0, 0, getWide()-1, getTall()-1);

			drawSetColor(255, 255, 255, 150);
			drawOutlinedRect(1, 1, getWide(), getTall());
        }

		drawSetColor(0, 0, 0, 0);
		drawOutlinedRect(0, 0, getWide(), getTall());
	}

	void internalCursorExited()
	{
		setSelected(false);
	}
};


class CMyOSScrollPanel : public ScrollPanel
{
public:
	CMyOSScrollPanel(const char* imgname, int x,int y,int wide,int tall) : ScrollPanel(x, y, wide, tall)
	{
		ScrollBar *pScrollBar = getVerticalScrollBar();
		pScrollBar->setButton( new CMyOSScrollbutton( 1, 0,0 ), 0 );
		pScrollBar->setButton( new CMyOSScrollbutton( 0, 0,0 ), 1 );
		pScrollBar->setSlider( new CMyOSSlider(0,wide-1,wide,(tall-(wide*2))+2,true) ); 
		pScrollBar->setPaintBorderEnabled(false);
		pScrollBar->setPaintBackgroundEnabled(false);
		pScrollBar->setPaintEnabled(false);
			
		setPaintBackgroundEnabled(false);
		setPaintEnabled(false);			
	}
};
/*
void CMyMenuButton::internalCursorExited()
{
	setSelected(false);
	setArmed(false);
}
*/

void CMyMenuButton:: paintBackground()
{
	if (isArmed())
	{
		drawSetColor(255, 0, 0, 150);
		drawFilledRect(0, 0, getWide(), getTall());

		drawSetColor(255, 0, 0, 150);
		drawOutlinedRect(0, 0, getWide()-1, getTall()-1);

		drawSetColor(255, 0, 0, 150);
		drawOutlinedRect(1, 1, getWide(), getTall());

		iTimeMenuShow = 200;
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
}

void CMyOSButton::paint()
{
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

		/*	if( pOS )
			if( iAlpha == ALPHA_VISIBLE )
			{
				pOS->OSMenuShow( this );
			}*/
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
}







void COSWindow::actionPerformed(Panel* panel)
{
	CloseWindow();
}

void COSWindow::CloseWindow()
{
	if( bIsCamera )
	pOS->OSOpenAPP( pOS->szCurrentOSPath );//gross hack

	setVisible(false);
	pOS->iOpenedWindows--;


	PlaySound("scripts/OS/closed.wav", 1);
}

void COSWindow::paint()
{
	//prevent the window being dragged far beyond limits
	int x, y;

	getPos( x, y );

	if( x < 0 )
	x = 0;

	if( y < 0 )
	y = 0;

	if( x + pOS->window_default_size[0] > ScreenWidth )
	x = pOS->window_default_size[0] + WINDOW_HANDLE_H;

	if( y + pOS->window_default_size[1]> ScreenHeight )
	y = pOS->window_default_size[1] - WINDOW_HANDLE_H;

	setPos( x, y );


	if (fAlpha > 0.0)
	fAlpha -= (gHUD.m_flTimeDelta * 500);

	if (fAlpha < 0.0)
	fAlpha = 0.0;

	//useful check to not show more than a window once
	//EDIT: more than one window can be displayed nicely but I don't like the Z order!
	//SET THIS ON THE AUTOEXEC!
	/*
	if( pOS->iOpenedWindows == 2 )
	{
		setVisible(false);//bai bai!
		pOS->iOpenedWindows--;
	}
*/

	if( bIsCamera )
	{
		setBgColor(0, 0, 0, 255);

		window_panel_menu->setPaintBackgroundEnabled(true);
		window_panel_body->setPaintBackgroundEnabled(true);

		window_panel_menu->setBgColor( WINDOW_COLOR_MENU, 255 );
		window_panel_body->setBgColor( WINDOW_COLOR_BODY, 255 );
		
		if( window_button_up )
		window_button_up->setVisible( false );	
		
		if( window_label_directory )
		window_label_directory->setVisible( false );
	}
	else
	{
		if(hasFocus())
		setBgColor(0, 0, 0, fAlpha);
		else
		setBgColor(50, 50, 50, fAlpha);

		window_panel_menu->setBgColor( WINDOW_COLOR_MENU, fAlpha );
		window_panel_body->setBgColor( WINDOW_COLOR_BODY, fAlpha );
	}

		//label that shows current directory


/*
	if( window_button_close )//INCREDIBLE GROSS HACK - ALPHA WONT CHANGE IF BUTTON AINT VISIBLE, CHANGE!!
	{
		if(hasFocus())
		setBgColor(0, 0, 0, window_button_close->iAlpha);
		else
		setBgColor(50, 50, 50, window_button_close->iAlpha);
	}
*/
	if( image_os_logo_black )
		image_os_logo_black->m_pTGA->setColor( Color(255,255,255, fAlpha) );
}



void COSWindow::doDeniedButtons()
{	
	CSchemeManager *pSchemes = gViewPort->GetSchemeManager();
	SchemeHandle_t hTextScheme = pSchemes->getSchemeHandle( "OS_large" );
	Font *pDefFont = pSchemes->getFont( hTextScheme );

	window_button_error	= new CMyOSButton("",  0,  0, 0, ALPHA_VISIBLE);

	if( window_button_error )
	{
		int tga_size_x, tga_size_y;

		window_button_error->setParent(this);
		
		window_button_error->m_pTGA = vgui_LoadTGA("scripts/OS/logon.tga");
		window_button_error->setImage(window_button_error->m_pTGA);

 		window_button_error->m_pTGA->getSize( tga_size_x, tga_size_y );
		window_button_error->setSize( tga_size_x, tga_size_y );
				
//		window_button_error->setPos( this->getWide()/2 - tga_size_x/2,	this->getTall()/2 - tga_size_y/2 + WINDOW_HANDLE_H*2);//TODO: possitionate this button accordingly
		window_button_error->setPos( this->getWide()/2 - tga_size_x/2, this->getTall()/2 - tga_size_y/2 + this->getTall()/4);
	
		window_button_error->addActionSignal(this);//closes the window
	}

	Label * label				= new Label("", 30, 0, 0, 0);

	if( label )
	{
		label->setParent(this);
		label->setFont(pDefFont);
		label->setContentFitted(true);//to fit the button to the current characters size

		label->setPaintBackgroundEnabled(false);
		label->setFgColor(0, 0, 0, ALPHA_VISIBLE);//this is useless because will be overidden
		
		label->setText( "Whoops! Sorry, but this is protected!" );

		int itext_width, itext_height;

		label->getSize( itext_width, itext_height );
	//	label->setPos( this->getWide()/2 - itext_width/2, this->getTall()/2 - itext_height/2 + this->getTall()/4);
		label->setPos( this->getWide()/2 - itext_width/2,	this->getTall()/2 - itext_height/2 + WINDOW_HANDLE_H*2);

		label->setVisible(true);//to prevent show the button at the corner on the very first frame
	}

	image_os_logo_black->setVisible( true );

	//hide stuff we don't need
	if( window_button_close )
	window_button_close->setVisible( false );

	if( window_button_up )
	window_button_up->setVisible( false );	
	
	if( window_label_directory )
	window_label_directory->setVisible( false );
}



void COSWindow::doStadisticsWindow()
{	
	CSchemeManager *pSchemes = gViewPort->GetSchemeManager();
	SchemeHandle_t hTextScheme = pSchemes->getSchemeHandle( "OS_large" );
	Font *pDefFont = pSchemes->getFont( hTextScheme );

	Label * label				= new Label("", 30, 0, 0, 0);

	if( label )
	{
		label->setParent(this);
		label->setFont(pDefFont);
		label->setContentFitted(true);

		label->setPaintBackgroundEnabled(false);
		label->setFgColor(0, 0, 0, ALPHA_VISIBLE);
				
		char szLevelText[256];		

		int iCurrentLevel = CVAR_GET_FLOAT("xp_points") / 100;
		int i_filled_level = CVAR_GET_FLOAT("xp_points") - (iCurrentLevel * 100);

					
		sprintf(szLevelText, "Level : %i\nExperience : %i/100\nTotal Kills : %i\nHeadshots : %i\nKnifed : %i\nUnlocked Pages : %i", 
			iCurrentLevel, 
			i_filled_level, 
			int(CVAR_GET_FLOAT("score_killed")),
			int(CVAR_GET_FLOAT("score_head")),
			int(CVAR_GET_FLOAT("score_knifed")),
			int(CVAR_GET_FLOAT("score_pages"))
			);

		label->setText( szLevelText );

		label->setPos(0,	100);
	}


	if( window_button_up )
	window_button_up->setVisible( false );	
	
	if( window_label_directory )
	window_label_directory->setVisible( false );
}

void COSWindow::doInfoWindow( char * sz_msg )
{	
	CSchemeManager *pSchemes = gViewPort->GetSchemeManager();
	SchemeHandle_t hTextScheme = pSchemes->getSchemeHandle( "OS_large" );
	Font *pDefFont = pSchemes->getFont( hTextScheme );

/*
_
	int butX, butY;
	Button* button = new Button("     OK     ", 0, 0);
	button->setParent(this);
	button->addActionSignal(this);

	button->getSize(butX, butY);
	butX = (this->getWide() - butX) / 2;
	butY = this->getTall() - butY - YRES(10);
	button->setPos( butX, butY );

//	button->setBorder(new LoweredBorder());
//	button->setBorder(new EtchedBorder());
	button->setBorder(new LineBorder(1, Color(255, 0, 0, ALPHA_VISIBLE)));
*/
	Label * label				= new Label("", 30, 0, 0, 0);

	if( label )
	{
		label->setParent(this);
		label->setFont(pDefFont);
		label->setContentFitted(true);//to fit the button to the current characters size

		label->setPaintBackgroundEnabled(false);
		label->setFgColor(0, 0, 0, ALPHA_VISIBLE);//this is useless because will be overidden
		
		label->setText( CHudTextMessage::BufferedLocaliseTextString( sz_msg ) );

		int itext_width, itext_height;

		label->getSize( itext_width, itext_height );
	//	label->setPos( this->getWide()/2 - itext_width/2, this->getTall()/2 - itext_height/2 + this->getTall()/4);
		label->setPos( this->getWide()/2 - itext_width/2,	this->getTall()/2 - itext_height/2 + WINDOW_HANDLE_H*2);

		label->setVisible(true);//to prevent show the button at the corner on the very first frame
	}

	//hide stuff we don't need
//	if( window_button_close )
//	window_button_close->setVisible( false );

	if( window_button_up )
	window_button_up->setVisible( false );	
	
	if( window_label_directory )
	window_label_directory->setVisible( false );
}

void COSWindow::doCreateLOGInButton()
{
	window_button_logon	= new CMyOSButton("",  0,  0, 0, ALPHA_VISIBLE);

	if( window_button_logon )
	{
		int tga_size_x, tga_size_y;

		window_button_logon->setParent(this);
		
		window_button_logon->m_pTGA = vgui_LoadTGA("scripts/OS/logon.tga");
		window_button_logon->setImage(window_button_logon->m_pTGA);

 		window_button_logon->m_pTGA->getSize( tga_size_x, tga_size_y );
		window_button_logon->setSize( tga_size_x, tga_size_y );
				
		window_button_logon->setPos( this->getWide()/2 - tga_size_x/2,	this->getTall()/2 - tga_size_y/2 + WINDOW_HANDLE_H*2);//TODO: possitionate this button accordingly

		window_button_logon->addActionSignal( new command_open( pOS, "logon" ) );//we will check this string later
	}

	image_os_logo_black->setVisible( true );

	//hide stuff we don't need
	if( window_button_close )
	window_button_close->setVisible( false );

	if( window_button_up )
	window_button_up->setVisible( false );	
	
	if( window_label_directory )
	window_label_directory->setVisible( false );
}

void COSWindow::Init()
{
	pOS->iOpenedWindows++;

	if( window_button_up )
		window_button_up->addActionSignal( new command_open( pOS, pOS->szOldOSPath ) );//this button will open the previous directory

/*
	char fake_path[512];

//	sprintf( file_name, "%s", szWndTitle  );

	  char str[] ="This is a simple string";
	  char * pch;
	  pch = strstr (str,"simple");
	  strncpy (pch,"sample",6);
	  puts (str);
	  return 0;*/



	//TODO: DO NOT SHOW DIRECTORY IF IT'S MY PC
	//TODO: SHOW A FAKE PATH
	if( window_label_directory )
	window_label_directory->setText( pOS->szCurrentOSPath );
}































COpSys::COpSys() : Panel(0, 0, ScreenWidth, ScreenHeight)
{	
	CSchemeManager *pSchemes = gViewPort->GetSchemeManager();
 	SchemeHandle_t hTextScheme = pSchemes->getSchemeHandle( "OS" );
	Font *pDefFont = pSchemes->getFont( hTextScheme );

	SchemeHandle_t hTextSchemeArab = pSchemes->getSchemeHandle( "OS_arabic" );
	Font *pDefFontArab = pSchemes->getFont( hTextSchemeArab );


	setVisible(true);
	gViewPort->UpdateCursorState();
    
	setPaintBackgroundEnabled(true);
	setBgColor(0, 0, 0, ALPHA_INVISIBLE);

	fAlpha = ALPHA_INVISIBLE;
//	iOpenedWindows = 0;
	bCameraActive = false;

	iTimeMenuShow = 200;

	//moved on the message call
//	bLogged = false;
//	fDelay = gEngfuncs.pfnRandomFloat( 500.0, 2000.0 );
	//TODO CREATE MAIN MAINEL

	my_panel_desktop	= new Panel(0, 0, ScreenWidth, ScreenHeight);

	my_panel_desktop->setParent(this);
	my_panel_desktop->setPaintBackgroundEnabled(true);

	my_panel_desktop->setBgColor(255, 0, 255, 0);//pink color for debug

/*	Label * label				= new Label("", 30, 0, 0, 0);

	if( label )
	{
		label->setParent(this);
		label->setFont(pDefFont);
		label->setContentFitted(true);//to fit the button to the current characters size

		label->setPaintBackgroundEnabled(false);
		label->setFgColor(255, 255, 255, ALPHA_VISIBLE);//this is useless because will be overidden
		
		label->setText( "ENABLE/DISABLE DESKTOP" );

		label->setVisible(true);//to prevent show the button at the corner on the very first frame
	}

	pSwitch = new CCheckButton2();
	pSwitch->setParent(this);
		
	pSwitch->SetImages("gfx/vgui/checked.tga", "gfx/vgui/unchecked.tga");
	pSwitch->SetText("");
	pSwitch->setPos(0, 0);
	pSwitch->SetCheckboxLeft(false);*/

	return;
}


void COpSys::OSMenuShow( CMyOSButton* button )
{
	if( bMenuActive )
		return;

	if( button->bFile == true )
	{
		menu_button_cut->setVisible(true);
		menu_button_copy->setVisible(true);
		menu_button_paste->setVisible(true);
		menu_button_delete->setVisible(true);

		int x, y;
		button->getPos( x, y );

		menu_button_cut->setPos( x + 40, y );
		menu_button_copy->setPos( x + 40, y + MENU_ELEMENT_H );
		menu_button_paste->setPos( x + 40, y + MENU_ELEMENT_H*2 );
		menu_button_delete->setPos( x + 40, y + MENU_ELEMENT_H*3 );

		iTimeMenuShow = 200;

		bMenuActive = true;
	}
}

void COpSys::OSMenuHide()
{
	gEngfuncs.Con_Printf("OSMenuHide\n" );

	menu_button_cut->setVisible(false);
	menu_button_copy->setVisible(false);
	menu_button_paste->setVisible(false);
	menu_button_delete->setVisible(false);

	bMenuActive = false;
}

void COpSys::OSSetBaseDirectory( char * path )
{
	sprintf( szCurrentOSPath, path );
	sprintf( szOldOSPath, path );
}

//creates either folders or files for the specified path on the specified window(pParent), which is a panel
void COpSys::OSCreateContents(char* path, Panel* pParent)
{
	char media[512];
	sprintf( media, "%s%s*", gEngfuncs.pfnGetGameDirectory(), path );

	struct _finddata_t c_file;
	long hFile;
	
	int icons_x = 40;
	int	icons_y = 60;

//	int window_w, window_h;			
//	pParent->getSize( window_w, window_h );

	if(!strcmp(szPCName, "pc1") )
	{
		char szFileContents[128];
			
		char file[512];
		sprintf( file, "%s%sasdas.txt", gEngfuncs.pfnGetGameDirectory(), path );

		fstream file_op(file, /*ios_base::in |*/ ios_base::out | ios_base::trunc );

		sprintf( szFileContents, "%i\n%04d\n%04d\n", iFinalCode, gEngfuncs.pfnRandomLong(0000,9999), gEngfuncs.pfnRandomLong(0000,9999) );
//		sprintf( szFileContents, "%i\n%04d\n%04d\n", iFinalCode, iFinalCode - 256, iFinalCode - 512 );
		gEngfuncs.Con_Printf( "%s\n", szFileContents );

		file_op << szFileContents <<endl;
		file_op.close();
	}
	//TODO
	//if folder is PAGES, call OSCreateAPP, but check unlocked pages first!

	if ( (hFile = _findfirst(media, &c_file)) == -1L )
	{
		gEngfuncs.Con_Printf("No files in current directory - %s\n", path);
	}
	else
	{
		do
		{				
			char file_name[128];
			char file_name_with_path[256];
			char icon_path[512];

			sprintf( file_name, "%s", c_file.name  );
							
			gEngfuncs.Con_Printf("*********** file_name  %s\n", c_file.name  );

			if( strpbrk( ".", c_file.name ) != NULL )//dot, extension, file!
			{
			//	gEngfuncs.Con_Printf("not a folder\n" );
				sprintf( file_name_with_path, "%s%s", szCurrentOSPath, c_file.name  );//don't use / at the end, isn't a folder ¬¬

				if ( strstr(c_file.name, ".tga") != NULL )
				{			
					if (strstr(c_file.name, "os_") == NULL )//if not an icon	
					{
						sprintf( icon_path, "scripts/OS/img.tga" );

						gEngfuncs.Con_Printf( "*********** %s  is USING %s as ICON\n", c_file.name, icon_path );

						OSCreateAPP( icons_x, icons_y, icon_path, file_name, file_name_with_path, pParent );
						icons_x+=80;
					}
				}

				if (strstr(c_file.name, ".txt") != NULL || strstr(c_file.name, ".cpp") != NULL || strstr(c_file.name, ".h") != NULL )
				{
					sprintf( icon_path, "scripts/OS/txt.tga" );
					OSCreateAPP( icons_x, icons_y, icon_path, file_name, file_name_with_path, pParent );
										
					icons_x+=80;
				}
				if (strstr(c_file.name, ".mp3") != NULL )
				{
					sprintf( icon_path, "scripts/OS/snd.tga" );
					OSCreateAPP( icons_x, icons_y, icon_path, file_name, file_name_with_path, pParent );
										
					icons_x+=80;
				}			
				if (strstr(c_file.name, ".app") != NULL )
				{
				/*	sprintf( icon_path, "scripts/OS/app.tga" );
					OSCreateAPP( icons_x, icons_y, icon_path, file_name, file_name_with_path, pParent );

					icons_x+=80;*/
								
					if (strstr(c_file.name, "os_") == NULL )//if not an icon	
					{
						sprintf( icon_path, "%sos_%s.tga", szCurrentOSPath, c_file.name );

						//create dummy image to load the icon we want, if not present, set another one
						CImageLabel * dummy_image	= new CImageLabel( "", 0, 0 );
						
						if(dummy_image)
						{
							dummy_image->setParent(my_panel_desktop);
							dummy_image->m_pTGA = vgui_LoadTGA( icon_path );
						
							if(!dummy_image->m_pTGA)
							sprintf( icon_path, "scripts/OS/app.tga" );
						}	
					
						OSCreateAPP( icons_x, icons_y, icon_path, file_name, file_name_with_path, pParent );

					//	icons_x+=dummy_image->getImageWide()*2;
						icons_x+=80;
					}
				}
					
				//the following files are hidden
				//SHOULD NOT BE DISPLAYED!
					//SET THIS ON THE AUTOEXEC! COULD BE USEFUL

				if ( strstr(c_file.name, ".") != NULL )
				{		
				//	sprintf( icon_path, "scripts/OS/default.tga" );
				//	icons_x-=80;

				//	OSCreateAPP( icons_x, icons_y, icon_path, file_name, file_name_with_path, pParent );
				}

				if ( strstr(c_file.name, "..") != NULL )
				{		
				//	sprintf( icon_path, "scripts/OS/default.tga" );
				//	icons_x-=80;
				//	OSCreateAPP( icons_x, icons_y, icon_path, file_name, file_name_with_path, pParent );
				}


				if( icons_x >= window_default_size[0] ) 
				{
					icons_x = 40;//reset
					icons_y = icons_y * 2;
				}
			}	
			else
			{
				//TODO: place apps fitting the window!
				sprintf( file_name_with_path, "%s%s/", szCurrentOSPath, c_file.name  );//use / at the end is a folder

				sprintf( icon_path, "%sos_%s.tga", szCurrentOSPath, c_file.name );


				//create dummy image to load the icon we want, if not present, set another one
				CImageLabel * dummy_image	= new CImageLabel( "", 0, 0 );
				
				if(dummy_image)
				{
					dummy_image->setParent(my_panel_desktop);
					dummy_image->m_pTGA = vgui_LoadTGA( icon_path );
				
					if(!dummy_image->m_pTGA)
					sprintf( icon_path, "%sos_default_folder.tga", szDirectoryBase );
				}

				OSCreateAPP( icons_x, icons_y, icon_path, file_name, file_name_with_path, pParent );
				icons_x+=80;
			}
		} 
		while ( _findnext(hFile, &c_file) == 0 );
		
		_findclose(hFile);
	}
}

bool COpSys::Init()
{	
	CSchemeManager *pSchemes = gViewPort->GetSchemeManager();
 	SchemeHandle_t hTextScheme = pSchemes->getSchemeHandle( "OS" );
	Font *pDefFont = pSchemes->getFont( hTextScheme );

	SchemeHandle_t hTextSchemeArab = pSchemes->getSchemeHandle( "OS_arabic" );
	Font *pDefFontArab = pSchemes->getFont( hTextSchemeArab );


	sprintf( szDirectoryBase, "/scripts/OS/%s/", szPCName);

	OSSetBaseDirectory( szDirectoryBase );

	bArabic = false;
	bInitFailed = false;
	iOpenedWindows = 0;

	char file[512];
	sprintf( file, "scripts/os/%s/autoexec.ini", szPCName );

	char *szFile = (char *)gEngfuncs.COM_LoadFile( file, 5, NULL);
	char szToken[512];

	if (!szFile)
	{
		gEngfuncs.Con_Printf("Couldn't open OS file %s for simulation\n", file );
		bInitFailed = true;
		return false;
	}
	else
	{
		szFile = gEngfuncs.COM_ParseFile(szFile, szToken);

		while (szFile)
		{
			if ( !stricmp( szToken, "desktop_r" ) )
			{
				szFile = gEngfuncs.COM_ParseFile(szFile,szToken);
				desktop_color[0] = atoi(szToken);
			}
			else if ( !stricmp( szToken, "use_arabic_font" ) )
			{
				szFile = gEngfuncs.COM_ParseFile(szFile,szToken);
				bArabic = bool(atoi(szToken));
			}
			else if ( !stricmp( szToken, "desktop_g" ) )
			{
				szFile = gEngfuncs.COM_ParseFile(szFile,szToken);
				desktop_color[1] = atoi(szToken);
			}
			else if ( !stricmp( szToken, "desktop_b" ) )
			{
				szFile = gEngfuncs.COM_ParseFile(szFile,szToken);
				desktop_color[2] = atoi(szToken);
			}	
			if ( !stricmp( szToken, "label_small_r" ) )
			{
				szFile = gEngfuncs.COM_ParseFile(szFile,szToken);
				label_color[0] = atoi(szToken);
			}
			else if ( !stricmp( szToken, "label_small_g" ) )
			{
				szFile = gEngfuncs.COM_ParseFile(szFile,szToken);
				label_color[1] = atoi(szToken);
			}
			else if ( !stricmp( szToken, "label_small_b" ) )
			{
				szFile = gEngfuncs.COM_ParseFile(szFile,szToken);
				label_color[2] = atoi(szToken);
			}			
			else if ( !stricmp( szToken, "default_window_w" ) )
			{
				szFile = gEngfuncs.COM_ParseFile(szFile,szToken);
				window_default_size[0] = atoi(szToken);
			}		
			else if ( !stricmp( szToken, "default_window_h" ) )
			{
				szFile = gEngfuncs.COM_ParseFile(szFile,szToken);
				window_default_size[1] = atoi(szToken);
			}
			

			szFile = gEngfuncs.COM_ParseFile(szFile, szToken);
		}
	}

	my_panel_desktop->setBgColor(desktop_color[0], desktop_color[1], desktop_color[2], 0);

	gEngfuncs.COM_FreeFile( szFile );




	my_window_camera = new COSWindow( "Camera", NULL, 0, 0,  ScreenWidth, ScreenHeight, NULL);
	my_window_camera->setParent(this);
	my_window_camera->pOS = this;
	my_window_camera->Init();
	my_window_camera->bIsCamera = true;
	my_window_camera->setVisible(false);




	int tga_size_x, tga_size_y;

	my_wallpaper	= new CImageLabel( "", 0, 0 );
	
	if(my_wallpaper)
	{
		my_wallpaper->setParent(my_panel_desktop);


		sprintf( wallpaper_image, "scripts/os/%s/os_wallpaper.tga", szPCName );//TODO: LOAD A DUMMY TGA!


		my_wallpaper->m_pTGA = vgui_LoadTGA( wallpaper_image );

		my_wallpaper->setImage(my_wallpaper->m_pTGA);

 		my_wallpaper->m_pTGA->getSize( tga_size_x, tga_size_y );
		my_wallpaper->setSize( tga_size_x, tga_size_y );

		my_wallpaper->setPos( ScreenWidth/2 - tga_size_x/2, ScreenHeight/2 - tga_size_y/2 );
	}
	

	my_image_logo_white	= new CImageLabel( "", 0, 0 );
	
	if(my_image_logo_white)
	{
		my_image_logo_white->setParent(my_panel_desktop);

		char image[128];
		sprintf( image, "scripts/OS/logo_white.tga");

		my_image_logo_white->m_pTGA = vgui_LoadTGA( image );

		my_image_logo_white->setImage(my_image_logo_white->m_pTGA);

 		my_image_logo_white->m_pTGA->getSize( tga_size_x, tga_size_y );
		my_image_logo_white->setSize( tga_size_x, tga_size_y );

		my_image_logo_white->setPos( ScreenWidth/2 - tga_size_x/2, ScreenHeight/2 - tga_size_y/2 );
				
		my_image_logo_white->m_pTGA->setColor( Color(255,255,255, 200) );
	}


	//small os date, below
	my_label_time = new Label("", 0, 0, 0, ALPHA_VISIBLE);

	if( my_label_time )
	{
		my_label_time->setParent(my_panel_desktop);
		my_label_time->setFont(pDefFont);
		my_label_time->setPaintBackgroundEnabled(false);
		my_label_time->setContentFitted(true);

		my_label_time->setFgColor(255, 255, 255, ALPHA_VISIBLE);
		
		char szDate[128];
		sprintf( szDate, "%s", __DATE__ );

		my_label_time->setText( szDate );

		my_label_time->setVisible(true);//to prevent show the button at the corner on the very first frame

		int itext_width, itext_height;

		my_label_time->getSize( itext_width, itext_height );
		my_label_time->setPos( ScreenWidth - itext_width, ScreenHeight - itext_height);
	}

 	//small os path, below
	my_label_current_path = new Label("", 0, 0, 0, ALPHA_VISIBLE);

	if( my_label_current_path )
	{
		my_label_current_path->setParent(my_panel_desktop);
		my_label_current_path->setFont(pDefFont);
		my_label_current_path->setPaintBackgroundEnabled(false);
		my_label_current_path->setContentFitted(true);

		my_label_current_path->setFgColor(255, 255, 255, ALPHA_VISIBLE);
	}
	

	my_button_power	= new CMyOSButton("",  0,  0, 0, ALPHA_VISIBLE);

	int close_button_w, close_button_h;

	if( my_button_power )
	{
		my_button_power->setParent(my_panel_desktop);
		my_button_power->bFile = false;

		my_button_power->m_pTGA = vgui_LoadTGA("scripts/OS/power.tga");
		my_button_power->setImage(my_button_power->m_pTGA);

 		my_button_power->m_pTGA->getSize( close_button_w, close_button_h );
		my_button_power->setSize( close_button_w, close_button_h );
	
		my_button_power->addActionSignal( new command_open( this, "quit" ) );

		my_button_power->setPos(  ScreenWidth /2 - close_button_w/2 , ScreenHeight -  close_button_h );
	}
	
	menu_button_cut = new CMyMenuButton("cut", 0,  0, 0, ALPHA_VISIBLE);
	menu_button_copy = new CMyMenuButton("copy", 0,  0, 0, ALPHA_VISIBLE);
	menu_button_paste = new CMyMenuButton("paste", 0,  0, 0, ALPHA_VISIBLE);
	menu_button_delete = new CMyMenuButton("delete", 0,  0, 0, ALPHA_VISIBLE);

	menu_button_cut->setParent(this);
	menu_button_copy->setParent(this);
	menu_button_paste->setParent(this);
	menu_button_delete->setParent(this);

	menu_button_cut->setSize(MENU_ELEMENT_W, MENU_ELEMENT_H);
	menu_button_copy->setSize(MENU_ELEMENT_W, MENU_ELEMENT_H);
	menu_button_paste->setSize(MENU_ELEMENT_W, MENU_ELEMENT_H);
	menu_button_delete->setSize(MENU_ELEMENT_W, MENU_ELEMENT_H);

	menu_button_cut->addActionSignal( new command_open( this, "cut" ) );
	menu_button_copy->addActionSignal( new command_open( this, "copy" ) );
	menu_button_paste->addActionSignal( new command_open( this, "paste" ) );
	menu_button_delete->addActionSignal( new command_open( this, "delete" ) );

	OSMenuHide();


	if ( fDelay != 0.0)
		return true;

	if( bLogged == false )
	{
		OSAPPLogIN();
		return true;
	}
	

	PlaySound("scripts/OS/startup.wav", 1);//for some reason doesn't plays twice, weird

	my_image_logo_white->setVisible( false );

	gEngfuncs.pfnClientCmd( "os_logged" );//flag this entity as logged

	OSCreateContents(szCurrentOSPath, my_panel_desktop);
	
	return true;
}

void COpSys::paint()
{
	if(bInitFailed)
	{
		CloseWindow();
		return;
	}

	if (fDelay > 0.0)
	fDelay -= (gHUD.m_flTimeDelta * 500);

	if (fDelay < 0.0)
	{
		fDelay = 0.0;
		
		if(	Init() == false) 
		return;
				
		gEngfuncs.Con_Printf("Init()\n" );
	}

	if (fAlpha > 0.0)
	fAlpha -= (gHUD.m_flTimeDelta * 100);

	if (fAlpha < 0.0)
	fAlpha = 0.0;
	

	if (iTimeMenuShow > 0)
	iTimeMenuShow -= (gHUD.m_flTimeDelta * 100);

	if (iTimeMenuShow < 0)
	iTimeMenuShow = 0;

	if( iTimeMenuShow <= 0 )
	OSMenuHide();

	gEngfuncs.Con_Printf("iTimeMenuShow %i\n", iTimeMenuShow );

/*	if(my_wallpaper)
	if(!my_wallpaper->m_pTGA)
	{
		CloseWindow();
		return;
	}*/

	if( bLogged == false )
	{
		my_wallpaper->m_pTGA->setColor( Color(255,255,255, 200) );
		my_label_current_path->setVisible( false );
		my_label_time->setVisible( false );
	}
	else
	{
		my_wallpaper->m_pTGA->setColor( Color(255,255,255, 0) );
		my_label_current_path->setVisible( true );
		my_label_time->setVisible( true );
			
/*		if(pSwitch->IsChecked())
		my_panel_desktop->setVisible( false );
		else
		my_panel_desktop->setVisible( true );*/
	}
	
	//updates the path label
	if( my_label_current_path )
	{
		my_label_current_path->setText( szCurrentOSPath );

		int itext_width, itext_height;

		my_label_current_path->getSize( itext_width, itext_height );
		my_label_current_path->setPos( 0, ScreenHeight - itext_height);
	}
}

void COpSys::ActionSignal(int cmd)//not used, using OSOpenFolder instead
{
	

}
	


void COpSys::OSOpenAPP( char* full_path_to_app )
{
	PlaySound("scripts/OS/clicked.wav", 1);

	//menu buttons
	if(!strcmp(full_path_to_app, "cut") )
	{
		PlaySound("scripts/OS/error.wav", 1);	
		OSMenuHide();

		return;
	}

	if(!strcmp(full_path_to_app, "copy") )
	{
		PlaySound("scripts/OS/error.wav", 1);	
		OSMenuHide();

		return;
	}

	if(!strcmp(full_path_to_app, "paste") )
	{
		PlaySound("scripts/OS/error.wav", 1);	
		OSMenuHide();

		return;
	}

	if(!strcmp(full_path_to_app, "delete") )
	{
		PlaySound("scripts/OS/error.wav", 1);	
		OSMenuHide();

		return;
	}
	//menu buttons

	if(!strcmp(full_path_to_app, "quit") )
	{
		CloseWindow();
		gFMOD.StopMusic();

		gEngfuncs.pfnClientCmd( "os_exit" );

		//TODO: sonido de logout?	
		return;
	}

	if(!strcmp(full_path_to_app, "logon") )//user clicked on the button, lets check for strings
	{
		char text_user[2048];	
		char text_pass[2048];
		
		entry_user->getText(0,text_user,2048);
		entry_user->setText(null,0);

		entry_pass->getText(0,text_pass,2048);
		entry_pass->setText(null,0);
		
		if( !strcmp(szLogUser, text_user) && !strcmp(szLogPassword, text_pass) )
		{
			bLogged = true;
			my_window_login->CloseWindow();
			Init();
		}
		else
		PlaySound("scripts/OS/error.wav", 1);	

		return;
	}

	//TODO: UNDO BUTTON OPEN FILES! WRONG!!
	if (strstr(szOldOSPath, full_path_to_app) == NULL)//test
	{
		sprintf( szOldOSPath, "%s", szCurrentOSPath );//stores the old location - only if different
	}

	sprintf( szCurrentOSPath, "%s", full_path_to_app );//changes the current location
 
	if ( strstr(full_path_to_app, ".tga") != NULL )
	{
		OSAPPImageViewer( full_path_to_app );
		
		return;//do not open a window, the viewer already created one
	}

	if ( strstr(full_path_to_app, ".txt") != NULL || strstr(full_path_to_app, ".cpp") != NULL || strstr(full_path_to_app, ".h") != NULL)
	{
		OSAPPTextViewer( full_path_to_app );
		return;//do not open a window,  the viewer already created one
	}

	if ( strstr(full_path_to_app, ".mp3") != NULL )
	{
		gFMOD.Play2DSound(full_path_to_app);

		return;//do not open a window,  the viewer already created one
	}

	if ( strstr(full_path_to_app, "ainbows") != NULL )
	{
		COSWindow * my_window_error = new COSWindow( "Access denied", NULL, ScreenWidth/2-window_default_size[0]/2,ScreenHeight/2-window_default_size[1]/2, window_default_size[0],window_default_size[1], NULL);
		my_window_error->setParent(my_panel_desktop);
		my_window_error->pOS = this;
		my_window_error->Init();
		my_window_error->doDeniedButtons();

		return;//do not open a window,  the viewer already created one
	}

	if ( strstr(full_path_to_app, "stadistics") != NULL )
	{
		COSWindow * my_window_error = new COSWindow( "Player Stadistics", NULL, ScreenWidth/2-window_default_size[0]/2,ScreenHeight/2-window_default_size[1]/2, window_default_size[0],window_default_size[1], NULL);
		my_window_error->setParent(my_panel_desktop);
		my_window_error->pOS = this;
		my_window_error->Init();
		my_window_error->doStadisticsWindow();

		return;//do not open a window,  the viewer already created one
	}

	if ( strstr(full_path_to_app, ".app") != NULL )
	{
		char command_to_send[128];
		char sz_msg_box[128];
		bool bShowMsgbox = false;
		bool bIsMap = false;

	//	char file[512];
	//	sprintf( file, "scripts/os/%s/script.app", szPCName );//TODO:¡'''00'¡¡¡¡¡¡¡

		char *szFile = (char *)gEngfuncs.COM_LoadFile( full_path_to_app, 5, NULL);
		char szToken[512];

		if (!szFile)
		{
			gEngfuncs.Con_Printf("Couldn't open OS file %s for simulation\n", full_path_to_app );
			return;
		}
		else
		{
			szFile = gEngfuncs.COM_ParseFile(szFile, szToken);

			while (szFile)
			{
				if ( !stricmp( szToken, "run" ) )
				{
					szFile = gEngfuncs.COM_ParseFile(szFile,szToken);	
					strncpy( command_to_send, szToken, sizeof(command_to_send) );
							
					gEngfuncs.Con_Printf("szToken %s   \n", szToken );
				}

				if ( !stricmp( szToken, "msg_box" ) )
				{
					bShowMsgbox = true;
									
				//	szFile = gEngfuncs.COM_ParseFile(szFile,szToken);	
					sprintf( sz_msg_box, "%s", szFile );
				//	strncpy( sz_msg_box, szToken, sizeof(sz_msg_box) );
				}

				if ( !stricmp( szToken, "map" ) )
				{
					szFile = gEngfuncs.COM_ParseFile(szFile,szToken);	
					strncpy( command_to_send, szToken, sizeof(command_to_send) );

					bIsMap = true;
				}

				szFile = gEngfuncs.COM_ParseFile(szFile, szToken);
			}
		}

		gEngfuncs.COM_FreeFile( szFile );

		char final_message[128];
		sprintf( final_message, "os_run %s", command_to_send );

		gEngfuncs.pfnClientCmd( final_message );
	
		PlaySound("scripts/OS/run.wav", 1);

		if ( strstr(command_to_send, "cam") != NULL )//it's a camera! :A
		{
			if( bCameraActive == false )
			{
				my_panel_desktop->setVisible( false );
				my_window_camera->setVisible( true );
				bCameraActive = true;
			}
			else
			{
				my_panel_desktop->setVisible( true );
				my_window_camera->setVisible( false );
				bCameraActive = false;
			}
		}

		if( bShowMsgbox == true )
		{
			COSWindow * my_window_error = new COSWindow( "Message", NULL, ScreenWidth/2-window_default_size[0]/2,ScreenHeight/2-window_default_size[1]/2, window_default_size[0]/2, window_default_size[1]/2, NULL);
			my_window_error->setParent(my_panel_desktop);
			my_window_error->pOS = this;
			my_window_error->Init();
			my_window_error->doInfoWindow( sz_msg_box );
		}

		if ( bIsMap )//it's a map :O
		{
			sprintf( final_message, "map %s", command_to_send );

			gEngfuncs.pfnClientCmd( final_message );
		}

		return;//do not open a window
	}






	char realtitle[128];

	//TODO: CHECK IF IS A FOLDER

	//IF FOLDER
	sprintf( realtitle, "%s", extract_folder_name(full_path_to_app) );
	//ELSE
	//EXTRACT FILE NAME AND ADD APLICATION NAME AFTER IE: DOC.TXT - TEXT VIEWER

	OSCreateWindow( realtitle, NULL, window_default_size[0], window_default_size[1], NULL );
}

void COpSys::OSAPPLogIN()
{
	CSchemeManager *pSchemes = gViewPort->GetSchemeManager();
 	SchemeHandle_t hTextScheme = pSchemes->getSchemeHandle( "OS" );
	Font *pDefFont = pSchemes->getFont( hTextScheme );

	SchemeHandle_t hTextSchemeArab = pSchemes->getSchemeHandle( "OS_arabic" );
	Font *pDefFontArab = pSchemes->getFont( hTextSchemeArab );

	const int wdw_size_x = window_default_size[0];
	const int wdw_size_y = window_default_size[1];

	my_window_login = new COSWindow( "User Authentication Required", NULL, ScreenWidth/2-wdw_size_x/2,ScreenHeight/2-wdw_size_y/2, wdw_size_x,wdw_size_y, NULL);
	my_window_login->setParent(my_panel_desktop);
	my_window_login->pOS = this;
	my_window_login->Init();
	my_window_login->doCreateLOGInButton();//TODO, REMOVE WINDOW MENU AND CLOSE BUTTONS!

	const int text_enty_w = 200;
	const int text_enty_h = 20;
	const int text_enty_x = wdw_size_x/2 - text_enty_w/2;
	const int text_enty_y = wdw_size_y - text_enty_h *4;

	
	my_label_username = new Label("", 0, 0, 0, ALPHA_VISIBLE);

	if( my_label_username )
	{
		my_label_username->setParent(my_window_login);
		if( bArabic )
		my_label_username->setFont(pDefFontArab);
		else
		my_label_username->setFont(pDefFont);
		my_label_username->setPaintBackgroundEnabled(false);
		my_label_username->setContentFitted(true);

		my_label_username->setFgColor(0, 0, 0, ALPHA_VISIBLE);

		my_label_username->setText( "USERNAME" );//TODO: TITLES.TXT ME!

		my_label_username->setVisible(true);

		int itext_width, itext_height;

		my_label_username->getSize( itext_width, itext_height );
		my_label_username->setPos( text_enty_x - itext_width, text_enty_y-text_enty_h*2);
	}


	entry_user =new TextEntry("",text_enty_x,text_enty_y-text_enty_h*2,text_enty_w,text_enty_h);
	entry_user->setParent(my_window_login);
	entry_user->setFont(pDefFont);

	my_label_password = new Label("", 0, 0, 0, ALPHA_VISIBLE);

	if( my_label_password )
	{
		my_label_password->setParent(my_window_login);
		my_label_password->setFont(pDefFont);
		my_label_password->setPaintBackgroundEnabled(false);
		my_label_password->setContentFitted(true);

		my_label_password->setFgColor(0, 0, 0, ALPHA_VISIBLE);

		my_label_password->setText( "PASSWORD" );//TODO: TITLES.TXT ME!

		my_label_password->setVisible(true);

		int itext_width, itext_height;

		my_label_password->getSize( itext_width, itext_height );
		my_label_password->setPos( text_enty_x - itext_width, text_enty_y);
	}

	entry_pass =new TextEntry("",text_enty_x,text_enty_y,text_enty_w,text_enty_h);
	entry_pass->setParent(my_window_login);
	entry_pass->setFont(pDefFont);

	PlaySound("scripts/OS/warning.wav", 1);
}


void COpSys::OSAPPImageViewer( char* full_path_to_file )
{
	setVisible(true);
	setPaintBackgroundEnabled(true);
	gViewPort->UpdateCursorState();


	//check if there are images on this folder
	//put these in a array

	//using buttons to navigate through array
//call void OSOpenAPP to open the image

	int tga_size_x, tga_size_y;

	CImageLabel * image_label	= new CImageLabel( "", 0, 0 );

	if(image_label)
	{
		image_label->setParent(this);

		image_label->m_pTGA = vgui_LoadTGA( full_path_to_file );

		image_label->setImage(image_label->m_pTGA);

 		image_label->m_pTGA->getSize( tga_size_x, tga_size_y );
		image_label->setSize( tga_size_x, tga_size_y );

//		image_label->setPos( wdw_size_x/2- tga_size_x/2, WINDOW_MENU_H + (wdw_size_y/2) - (tga_size_y/2) );
		image_label->setPos( 0, 0 );//we are going to change the position later
	}

	int wdw_size_x = tga_size_x;
	int wdw_size_y = tga_size_y;

	if( wdw_size_x < window_default_size[0] )
	wdw_size_x = window_default_size[0];

	if( wdw_size_y < window_default_size[1] )
	wdw_size_y = window_default_size[1];

/*	int wdw_size_x = window_default_size[0];
	int wdw_size_y = window_default_size[1];*/

	//TODO, THE MAIN WINDOW CLASS MUST EXTRACT FOLDER NAME AUTOMATICALLY!
	char realtitle[128];
	sprintf( realtitle, "%s", extract_folder_name(full_path_to_file) );//BUGBUG: IF IT'S A FILE, NOT A FOLDER, THE LAST LETTER WILL BE CHOPPED!

	COSWindow * main_window = new COSWindow( realtitle, NULL, ScreenWidth/2-wdw_size_x/2,ScreenHeight/2-wdw_size_y/2, wdw_size_x,wdw_size_y, NULL);
	main_window->setParent(my_panel_desktop);
	main_window->pOS = this;
	main_window->Init();

		
	if(main_window)
	{
		image_label->setParent(main_window);
		image_label->setPos( wdw_size_x/2- tga_size_x/2, WINDOW_MENU_H + (wdw_size_y/2) - (tga_size_y/2) );
	}
//TODO: ADD A BUTTON TO KEEP BROWSING FILES!
	//TODO: RESIZE WINDOW IF IMAGE IS TOO BIG
}

void COpSys::OSAPPTextViewer( char* full_path_to_file )
{
	setVisible(true);
	setPaintBackgroundEnabled(true);
	gViewPort->UpdateCursorState();




	int fileLength;
	char* pFile = (char*)gEngfuncs.COM_LoadFile( full_path_to_file, 5, &fileLength );
/*
	if (!pFile)
	{
		char buf[128];
		sprintf(buf, "Unable to load file %s\n\n", full_path_to_file);
		return;
	}*/



/*	char* ptext = pFile;
	char tagName[128];

	if (!FindNextTag(ptext))
	{
		char buf[128];
		sprintf(buf, "%s - empty file", full_path_to_file);
		return;
	}*/

	
	int size_x = 0;
	int size_y = 0;

	int upperBound = YRES(10);

	CSchemeManager *pSchemes = gViewPort->GetSchemeManager();
 	SchemeHandle_t hTextScheme = pSchemes->getSchemeHandle( "OS" );
	Font *pDefFont = pSchemes->getFont( hTextScheme );

	SchemeHandle_t hTextSchemeArab = pSchemes->getSchemeHandle( "OS_arabic" );
	Font *pDefFontArab = pSchemes->getFont( hTextSchemeArab );


//	ptext++;
//	int hasParams = GetTagName(ptext, tagName);

	char panelImage[128];
	char buttonImage[128];
	char scrollImage[128];
	panelImage[0] = 0;
	buttonImage[0] = 0;
	scrollImage[0] = 0;
	int butclr_r = 255, butclr_g = 255, butclr_b = 255, butclr_a = 0;
	int scroll_x = 0, scroll_y = 0, scroll_wide = 0, scroll_tall = 0;

	// parse HEAD section
/*	if(!strcmp(tagName, "HEAD"))
	{
		if (hasParams)
		{
			char paramName[128];
			char paramValue[128];

			while( GetTagParameter(ptext, paramName, paramValue) )
			{
				if (!strcmp(paramName, "xsize"))
					size_x = atoi(paramValue);
				else if (!strcmp(paramName, "ysize"))
					size_y = atoi(paramValue);
				else
					gEngfuncs.Con_Printf("File %s - unknown HEAD parameter: [%s]\n", full_path_to_file, paramName);
			}
		}

		if (!FindNextTag(ptext))
		{
			char buf[128];
			sprintf(buf, "%s - got nothing, except HEAD", full_path_to_file);
 		//	return;
		}

		ptext++;
		hasParams = GetTagName(ptext, tagName);
	}
	else
	{
	}
*/

	// create main panel
	size_x = window_default_size[0];
	size_y = window_default_size[1];

	//TODO, THE MAIN WINDOW CLASS MUST EXTRACT FOLDER NAME AUTOMATICALLY!
	char realtitle[128];
	sprintf( realtitle, "%s", extract_folder_name(full_path_to_file) );//BUGBUG: IF IT'S A FILE, NOT A FOLDER, THE LAST LETTER WILL BE CHOPPED!

	COSWindow * main_window = new COSWindow( realtitle, NULL, ScreenWidth/2-size_x/2,ScreenHeight/2-size_y/2, size_x,size_y, NULL);
	main_window->setParent(my_panel_desktop);
	main_window->pOS = this;
	main_window->Init();
				






	//set text size
	scroll_wide = size_x;
	scroll_tall = size_y - WINDOW_HANDLE_H*2;
	scroll_x = 0;
	scroll_y = WINDOW_HANDLE_H*2;



	CMyOSScrollPanel * m_pScrollPanel = new CMyOSScrollPanel(scrollImage, scroll_x, scroll_y, scroll_wide, scroll_tall );
	m_pScrollPanel->setParent(main_window);
 
	m_pScrollPanel->setScrollBarAutoVisible(true, true);
	m_pScrollPanel->setScrollBarVisible(false, false);
	m_pScrollPanel->validate();

	// including panel
//	int panelsize = YRES(5);
	int panelsize = 0;//spacing
	Panel* pDocument = new Panel(0, 0, m_pScrollPanel->getClientClip()->getWide(), 64);
	pDocument->setParent(m_pScrollPanel->getClient());
	

	pDocument->setPaintBackgroundEnabled(true);
	pDocument->setBgColor(255, 255, 255, 0);
			
	pDocument->setBorder(new LineBorder(1, Color(0, 0, 0, 0)));

	// reading document elements
/*	while(1)
	{
		// parse text field
		if(!strcmp(tagName, "TEXT"))
		{*/
			TextPanel* pTextPanel = new TextPanel("", XRES(5), panelsize, pDocument->getWide() - XRES(10), 64);
			pTextPanel->setParent(pDocument);
			pTextPanel->setPaintBackgroundEnabled(false);
			if( bArabic )
			pTextPanel->setFont(pDefFontArab); // default font
			else
			pTextPanel->setFont(pDefFont); // default font

			pTextPanel->setFgColor(0, 0, 0, 0); // default color
/*
			int dspace = 0;

			bool bgColorSet = false;

			if (hasParams)
			{
				char paramName[128];
				char paramValue[128];

				while( GetTagParameter(ptext, paramName, paramValue) )
				{
					if (!strcmp(paramName, "font"))
					{
						SchemeHandle_t hTextScheme = pSchemes->getSchemeHandle(paramValue);
						Font* pTextFont = pSchemes->getFont( hTextScheme );
						pTextPanel->setFont(pTextFont);
					}
					else if (!strcmp(paramName, "color"))
					{
						int r, g, b, a;
						ParseColor(paramValue, r, g, b, a);
						pTextPanel->setFgColor(r, g, b, a);
					}
					else
						gEngfuncs.Con_Printf("File %s - unknown TEXT parameter: [%s]\n", full_path_to_file, paramName);
				}
			}

		//	char* textStart = ptext;
			int haveNextTag = FindNextTag(ptext);*/
		//	int textLength = ptext - textStart + 1;
		//	pTextPanel->getTextImage()->setText(textLength, textStart);
			pTextPanel->getTextImage()->setText(pFile);
			pTextPanel->getTextImage()->setSize( pTextPanel->getWide(), pTextPanel->getTall() );


			int wrappedX, wrappedY;
			pTextPanel->getTextImage()->getTextSizeWrapped(wrappedX, wrappedY);

		/*	if (bgColorSet)
			{
				int x, y, realY, unused;
				pTextPanel->setSize( m_pScrollPanel->getClientClip()->getWide() , wrappedY );
				pTextPanel->getTextImage()->setSize( wrappedX , wrappedY );
				pTextPanel->getPos(x, realY);
				pTextPanel->getTextImage()->getPos(unused, y);
				pTextPanel->getTextImage()->setPos(x, y);
				pTextPanel->setPos(0, realY);
			}
			else*/
				pTextPanel->setSize( wrappedX , wrappedY );

			panelsize += (wrappedY/* + dspace*/);

		/*	if (!haveNextTag)
				break; // no more tags

			ptext++;
		
			hasParams = GetTagName(ptext, tagName);
		}
		else
		{
			gEngfuncs.Con_Printf("File %s - unknown tag: [%s]\n", full_path_to_file, tagName);

			if (!FindNextTag(ptext))
				break; // no more tags

			ptext++;
			
			hasParams = GetTagName(ptext, tagName);
		}
	}*/

	gEngfuncs.COM_FreeFile(pFile);

	gEngfuncs.Con_Printf("8\n" );

	// document is ready, panelsize now contains the height of panel
	panelsize += YRES(5);
	int doc_x = pDocument->getWide();
	pDocument->setSize(doc_x, panelsize);
	m_pScrollPanel->validate();


	m_iMaxScrollValue = panelsize - m_pScrollPanel->getClientClip()->getTall();
}


// return 0 to hook key
// return 1 to allow key
int COpSys::KeyInput(int down, int keynum, const char *pszCurrentBinding)
{
	if (!down)
		return 1; // dont disable releasing of the keys

	switch (keynum)
	{
	// close window
	case K_ENTER:
		/*
	case K_ESCAPE://not detected
	{
		CloseWindow();
		gMP3.StopMP3();
		gEngfuncs.pfnClientCmd( "os_exit" );

		return 0;
	}*/

	case 96:
	case 126:
/*	case K_F1:
	case K_F2:
	case K_F3:
	case K_F4:*/
	case K_F5:
/*	case K_F6:
	case K_F7:
	case K_F8:
	case K_F9:
	case K_F10:
	case K_F11:
	case K_F12:*/
	case K_MOUSE1:
	case K_MOUSE2:
	case K_MOUSE3:
/*	case K_MOUSE4:
	case K_MOUSE5:*/
		return 1;
	}

	return 0;
}

void COpSys::OSCreateWindow( char* title, char* path, int w, int h, int win_id )
{
	//TODO: COSWINDOW NEVER USES THE PATH AND NEVER USES WIN_ID, REMOVE!
	COSWindow * pWindow = new COSWindow( title, NULL, ScreenWidth/2 - w/2,ScreenHeight/2 - h/2, w, h, NULL);
	pWindow->setParent(my_panel_desktop);
	pWindow->pOS = this;
	pWindow->Init();

//	sprintf( szOldOSPath, "%s", szCurrentOSPath );//stores the old location
//	sprintf( szCurrentOSPath, "%s", path );//changes the current location

	OSCreateContents(szCurrentOSPath, pWindow);
 	
	gEngfuncs.Con_Printf("szOldOSPath %s, szCurrentOSPath %s\n", szOldOSPath, szCurrentOSPath );
}	

void COpSys::OSCreateAPP( int x,int y, char* icon_name, char* icon_label, char* full_path, Panel* pParent)
{
	CSchemeManager *pSchemes = gViewPort->GetSchemeManager();
 	SchemeHandle_t hTextScheme = pSchemes->getSchemeHandle( "OS" );
	Font *pDefFont = pSchemes->getFont( hTextScheme );

	SchemeHandle_t hTextSchemeArab = pSchemes->getSchemeHandle( "OS_arabic" );
	Font *pDefFontArab = pSchemes->getFont( hTextSchemeArab );

	
//	SchemeHandle_t hTextScheme_OS_arabic = pSchemes->getSchemeHandle( "OS_arabic" );
//	Font *pOSFontSmallArabic = pSchemes->getFont( hTextScheme_OS_arabic );

	int tga_size_x, tga_size_y;

	CMyOSButton*	button	= new CMyOSButton("",  x,  y, 0, ALPHA_VISIBLE);		

	if( button )
	{
		button->pOS = this;
		button->bFile = true;

		button->setParent(pParent);

		button->m_pTGA = vgui_LoadTGA( icon_name );
			
 		gEngfuncs.Con_Printf("icon name for %s is: %s\n", full_path, icon_name );
		
		if(!button->m_pTGA)
		button->m_pTGA = vgui_LoadTGA( "scripts/OS/default.tga" );

		button->setImage(button->m_pTGA);

 		button->m_pTGA->getSize( tga_size_x, tga_size_y );
		button->setSize( tga_size_x, tga_size_y );

		button->addActionSignal(new command_open( this, full_path ) );

		button->setPos( x, y );
	}

	Label * label = new Label("", 0, 0, 0, ALPHA_VISIBLE);

	//TODO: CREATE TWO LABELS IF THE TEXT IS TOO BIG!!!
	if( label )
	{
		label->setParent(pParent);

		if(bArabic)
		label->setFont(pDefFontArab);
		else
		label->setFont(pDefFont);

		label->setPaintBackgroundEnabled(false);
		label->setContentFitted(true);

			
		Panel* my_parent = label->getParent();	

		//ugly way to check if the panel is a desktop
		if( my_parent->getWide() == ScreenWidth &&  my_parent->getTall() == ScreenHeight )
		label->setFgColor(255, 255, 255, ALPHA_VISIBLE);
		else
		label->setFgColor(label_color[0], label_color[1], label_color[2], ALPHA_VISIBLE);

		label->setText( icon_label );

		label->setVisible(true);

 
		
		int itext_width, itext_height;

		label->getSize( itext_width, itext_height );
		label->setPos( x + tga_size_x/2 - itext_width /2, y + tga_size_y);
	}
}	


/*
void COpSys::StateChanged(CCheckButton2 *pButton)
{
//	g_DontDrawWorld = !g_DontDrawWorld;
}
*/

void COpSys::CloseWindow()
{
/*	if(!strcmp(szPCName, "pc1") )
	{
		char szFileContents[256];
			
		char file[512];
		sprintf( file, "%s/scripts/os/%s/asdas.txt", gEngfuncs.pfnGetGameDirectory(), szPCName );

		fstream file_op(file,  ios_base::out | ios_base::trunc );

		sprintf( szFileContents, "Dear mofo: This only works in game.\n\nNow, since I've so much space left on the buffer (256 chars, actually) I am going to tell a joke, so, here is:\n\n-Why did the chicken cross the road?\n-Because he wanted to play Arrangement (?)\nYou shouldn't be messing around here." );
		gEngfuncs.Con_Printf( "%s\n", szFileContents );

		file_op << szFileContents <<endl;
		file_op.close();
	}*/

	setVisible(false);
	gViewPort->UpdateCursorState();
}

COpSys :: ~COpSys()
{
}