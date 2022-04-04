#include "hud.h"
#include "cl_util.h"
#include "vgui_TeamFortressViewport.h"
#include "vgui_weapon_panel.h"
#include "VGUI_LineBorder.h"
#include "VGUI_TextImage.h"
#include "../public/keydefs.h"
#include "triangleapi.h"

#include "r_studioint.h"
#include "com_model.h"

//TODO- RELEASE POINTERS!

CWeaponMenuPanel::CWeaponMenuPanel() : Panel(0, 0, ScreenWidth, ScreenHeight)
{	
//NEEDED SHIT	
//NEEDED SHIT	
	setVisible(true);
	setPaintBackgroundEnabled(false);

	gViewPort->UpdateCursorState();
//NEEDED SHIT	
//NEEDED SHIT	

	return;
}

void CWeaponMenuPanel::Init()
{
	CSchemeManager *pSchemes = gViewPort->GetSchemeManager();
	SchemeHandle_t hTextScheme = pSchemes->getSchemeHandle( "Default Text" );
	Font *pNormalFont = pSchemes->getFont( hTextScheme );


	PlaySound( "common/wpn_hudon.wav", 1 );

	int tga_size_x, tga_size_y;

//CUSTOM BUTTONS & LABELS - INITIALIZATION
//CUSTOM BUTTONS & LABELS - INITIALIZATION

//	mbutton			= new CMySpecialButton("",  0,  0, 0, 0);
//	mbutton_TEST	= new CMySpecialButton("",  0,  0, 0, 0);

	mbutton_close	= new CMySpecialButton("",  0,  0, 0, 0);

	label			= new Label("", 0, 0, 0, 0);
//	helper = new Label("", 0, 0, 0, 0);

//CUSTOM BUTTONS & LABELS - INITIALIZATION
//CUSTOM BUTTONS & LABELS - INITIALIZATION



	int sides = iNumButtons;
	int angles = 360/sides;
	int radius = 100;//THIS MUST BE SCREEN BASED! TODO: use a percentaje of the total height and width - or use buttons w,h

			
//	CONPRINT("iNumButtons %i= %i-%i-%i-%i-%i-%i\n", iNumButtons, sWeaponModifiers.sScope.iValue, sWeaponModifiers.sSilencer.iValue, sWeaponModifiers.sRedDot.iValue, sWeaponModifiers.sExtendedMag.iValue, sWeaponModifiers.sGL.iValue, sWeaponModifiers.sLaser.iValue);


	for( int i = 0; i < iNumButtons; i++)
	{
		int p = i;

		MenuButtons[i] = new CMySpecialButton("",  0,  0, 0, 0);

		if( MenuButtons[i] )
		{
			MenuButtons[i]->setParent(this);

			//set an image for this button, modifiers will change it accordingly (this avoid possible crashes)
			MenuButtons[i]->m_pTGA = vgui_LoadTGA("gfx/vgui/close.tga");

			//+1 because "i" starts from zero
			if( sWeaponModifiers.sFireMode.iOrder == i+1 )
			{
				MenuButtons[i]->m_pTGA = vgui_LoadTGA("gfx/vgui/saf.tga");
				MenuButtons[i]->addActionSignal( new CMenuHandler_StringCommand("mod_firemode") );
				MenuButtons[i]->myModType.sFireMode.iValue = MOD_ON;//set for this one
			}

			if( sWeaponModifiers.sScope.iOrder == i +1 )
			{
				MenuButtons[i]->m_pTGA = vgui_LoadTGA("gfx/vgui/scope.tga");
				MenuButtons[i]->addActionSignal( new CMenuHandler_StringCommand("mod_scope") );
				MenuButtons[i]->myModType.sScope.iValue = MOD_ON;//set for this one
			}

			if( sWeaponModifiers.sSilencer.iOrder == i +1 )
			{
				MenuButtons[i]->m_pTGA = vgui_LoadTGA("gfx/vgui/silencer.tga");
				MenuButtons[i]->addActionSignal( new CMenuHandler_StringCommand("mod_silencer") );
				MenuButtons[i]->myModType.sSilencer.iValue = MOD_ON;//set for this one
			}

			if( sWeaponModifiers.sRedDot.iOrder == i +1 )
			{
				MenuButtons[i]->m_pTGA = vgui_LoadTGA("gfx/vgui/reddot.tga");
				MenuButtons[i]->addActionSignal( new CMenuHandler_StringCommand("mod_reddot") );
				MenuButtons[i]->myModType.sRedDot.iValue = MOD_ON;//set for this one
			}

			if( sWeaponModifiers.sExtendedMag.iOrder == i +1 )
			{
				MenuButtons[i]->m_pTGA = vgui_LoadTGA("gfx/vgui/extended.tga");
				MenuButtons[i]->addActionSignal( new CMenuHandler_StringCommand("mod_extended") );
				MenuButtons[i]->myModType.sExtendedMag.iValue = MOD_ON;//set for this one
			}

			if( sWeaponModifiers.sGL.iOrder == i+1 )
			{
				MenuButtons[i]->m_pTGA = vgui_LoadTGA("gfx/vgui/gl.tga");
				MenuButtons[i]->addActionSignal( new CMenuHandler_StringCommand("mod_gl") );
				MenuButtons[i]->myModType.sGL.iValue = MOD_ON;//set for this one
			}

			if( sWeaponModifiers.sLaser.iOrder == i+1 )
			{
				MenuButtons[i]->m_pTGA = vgui_LoadTGA("gfx/vgui/laser.tga");
				MenuButtons[i]->addActionSignal( new CMenuHandler_StringCommand("mod_laser") );
				MenuButtons[i]->myModType.sLaser.iValue = MOD_ON;//set for this one
			}
			//moved up
/*
			if( sWeaponModifiers.sFireMode.iOrder == i+1 )
			{
				MenuButtons[i]->m_pTGA = vgui_LoadTGA("gfx/vgui/saf.tga");
				MenuButtons[i]->addActionSignal( new CMenuHandler_StringCommand("mod_firemode") );
				MenuButtons[i]->myModType.sFireMode.iValue = MOD_ON;//set for this one
			}*/


			MenuButtons[i]->setImage(MenuButtons[i]->m_pTGA);

 			MenuButtons[i]->m_pTGA->getSize( tga_size_x, tga_size_y );
			MenuButtons[i]->setSize( tga_size_x, tga_size_y );


			
			//p, same as i, but adds angles and sets cos-sin
			//I use -90 to rotate all the buttons -90º over the X axis, that way, the 0º will be right up-middle
			p = angles * i -90;

			if( p < 360 )//just to be sure isn't going to explode
			{
				MenuButtons[i]->desired_x = ( ScreenWidth /2 ) + ( radius * cos( p * M_PI / 180 ) ) - ( tga_size_x /2 );
				MenuButtons[i]->desired_y = ( ScreenHeight /2 ) + ( radius * sin( p * M_PI / 180 ) ) - ( tga_size_y /2 );

			//	CONPRINT("- desired_x %f - desired_y %f\n",  MenuButtons[i]->desired_x, MenuButtons[i]->desired_y );
			
				//start on the center of the screen
				MenuButtons[i]->setPos( ( ScreenWidth /2 ) - ( tga_size_x /2 ), ( ScreenHeight /2 ) - ( tga_size_y /2 ));
			}
		}
	}



//CLOSE WINDOW BUTTON
//CLOSE WINDOW BUTTON
	if( mbutton_close )
	{
		mbutton_close->setParent(this);
		
		mbutton_close->m_pTGA = vgui_LoadTGA("gfx/vgui/close.tga");
		mbutton_close->setImage(mbutton_close->m_pTGA);

 		mbutton_close->m_pTGA->getSize( tga_size_x, tga_size_y );
		mbutton_close->setSize( tga_size_x, tga_size_y );

		mbutton_close->addActionSignal(this);//closes the window
	
		mbutton_close->setPos( (ScreenWidth/2) - (tga_size_x/2), (ScreenHeight/2) - (tga_size_y/2) );
	}
//CLOSE WINDOW BUTTON
//CLOSE WINDOW BUTTON



	


//BLUE PANEL TEST	
//BLUE PANEL TEST	
	/*
	Panel* panel = new Panel(XRES(120), YRES(180), XRES(200), YRES(200));
	panel->setParent(this);
	panel->setBgColor(0, 0, 255, 100);
	panel->setBorder(new LineBorder);
	*/
//BLUE PANEL TEST	
//BLUE PANEL TEST	


//BUTTON LABEL TEST
//BUTTON LABEL TEST
	if( label )
	{
		label->setParent(this);
		label->setFont(pNormalFont);
		label->setPaintBackgroundEnabled(false);//TRUE FOR DEBUG PURPOSE
		label->setContentFitted(true);//to fit the button to the current characters size

//		label->setFgColor(0, 255, 0, ALPHA_INVISIBLE);//this is useless because will be overidden
		label->setFgColor(255, 255, 255, ALPHA_VISIBLE);//this is useless because will be overidden

	//	label->setVisible(false);//to prevent show the button at the corner on the very first frame
		label->setVisible(true);//to prevent show the button at the corner on the very first frame

	//	label->setContentAlignment( Label::a_center );
	}
//BUTTON LABEL TEST
//BUTTON LABEL TEST


//HELPERS LABEL TEST
//HELPERS LABEL TEST
	/*
	if( helper )
	{
		helper->setParent(this);
		helper->setFont(pNormalFont);
		helper->setPaintBackgroundEnabled(true);//TRUE FOR DEBUG PURPOSE
		helper->setContentFitted(true);//to fit the button to the current characters size

//		label->setFgColor(0, 255, 0, ALPHA_INVISIBLE);//this is useless because will be overidden
		helper->setFgColor(255, 255, 255, ALPHA_VISIBLE);//this is useless because will be overidden

	//	label->setVisible(false);//to prevent show the button at the corner on the very first frame
		helper->setVisible(true);//to prevent show the button at the corner on the very first frame

	//	label->setContentAlignment( Label::a_center );
	}
	*/
//HELPERS LABEL TEST
//HELPERS LABEL TEST





//FANCY BUTTON 1 SETTINGS
//FANCY BUTTON 1 SETTINGS
	/*
	if( mbutton )
	{
		mbutton->setParent(this);
		
		mbutton->m_pTGA = vgui_LoadTGA("gfx/vgui/button.tga");
		mbutton->setImage(mbutton->m_pTGA);

 		mbutton->m_pTGA->getSize( tga_size_x, tga_size_y );
		mbutton->setSize( tga_size_x, tga_size_y );

		mbutton->addActionSignal( new CMenuHandler_StringCommand("silencer") );

		mbutton->setPos( 200, 50 );
	}
	*/
//FANCY BUTTON 1 SETTINGS
//FANCY BUTTON 1 SETTINGS




//FANCY BUTTON 2 SETTINGS
//FANCY BUTTON 2 SETTINGS
	/*
	if( mbutton_TEST )
	{
		mbutton_TEST->setParent(this);
		
		mbutton_TEST->m_pTGA = vgui_LoadTGA("gfx/vgui/button.tga");
		mbutton_TEST->setImage(mbutton_TEST->m_pTGA);
		
		mbutton_TEST->m_pTGA->getSize( tga_size_x, tga_size_y );
		mbutton_TEST->setSize( tga_size_x, tga_size_y );

		mbutton_TEST->addActionSignal( new CMenuHandler_StringCommand("chupame_el_choto") );

		mbutton_TEST->setPos( 400, 50 );
	}
	*/
//FANCY BUTTON 2 SETTINGS
//FANCY BUTTON 2 SETTINGS

}

void CWeaponMenuPanel::paint()
{
	if( wasMousePressed(MOUSE_MIDDLE))
	{
		CloseWindow();
		return;
	}

	//cool effect to make the camera look at the weapon, hard coded though

	//HAWK : RESUMEIT DELETEIT
	//	gEngfuncs.Cvar_SetValue( "dev_punch_p", -0.6 );
//	gEngfuncs.Cvar_SetValue( "dev_punch_y", -0.8 );

	int button_x, button_y;
	int tga_size_x, tga_size_y;

	label->setFgColor(0, 0, 0, 255);//visible for the code, invisible for us
	label->setPos( 0, 0 );

	//move them on the position I want, to create a fucking circle
	for( int i = 0; i < iNumButtons; i++)
	{
		MenuButtons[i]->getPos( button_x, button_y );

		if( button_x > int(MenuButtons[i]->desired_x) )
		button_x -= /*20 **/ gHUD.m_flTimeDelta*300;

		if( button_x < int(MenuButtons[i]->desired_x) )
		button_x += /*20 **/ gHUD.m_flTimeDelta*300;

		if( button_x == int(MenuButtons[i]->desired_x) )
		button_x = int(MenuButtons[i]->desired_x);


		if( button_y > int(MenuButtons[i]->desired_y) )
		button_y -= /*20 **/ gHUD.m_flTimeDelta*300;

		if( button_y < int(MenuButtons[i]->desired_y) )
		button_y += /*20 **/ gHUD.m_flTimeDelta*300;

		if( button_y == int(MenuButtons[i]->desired_y) )
		button_y = int(MenuButtons[i]->desired_y);
					
		MenuButtons[i]->setPos( button_x, button_y);//place the buttons
	
		MenuButtons[i]->m_pTGA->setColor( Color(255,255,255, MenuButtons[i]->iAlpha) );//FIXME! this should done on the paint() function!
	

//		if ( MenuButtons[i]->bReady )//to prevent any color changes while it's in the process of setting desired pos
		if( MenuButtons[i]->isArmed() )
		{
			MenuButtons[i]->m_pTGA->getSize( tga_size_x, tga_size_y );

			label->setPos( int(MenuButtons[i]->desired_x) + tga_size_x, int(MenuButtons[i]->desired_y) /*+tga_size_y*/);

			if( MenuButtons[i]->iAlpha == ALPHA_VISIBLE )
			{
				//kinda tedious after a while...
				if( MenuButtons[i]->myModType.sScope.iValue == MOD_ON )
				label->setText( CHudTextMessage::BufferedLocaliseTextString( "#sScope_Add" ) );

				if( MenuButtons[i]->myModType.sSilencer.iValue == MOD_ON )
				label->setText( CHudTextMessage::BufferedLocaliseTextString( "#sSilencer_Add" ) );

				if( MenuButtons[i]->myModType.sRedDot.iValue == MOD_ON )
				label->setText( CHudTextMessage::BufferedLocaliseTextString( "#sRedDot_Add" ) );

				if( MenuButtons[i]->myModType.sExtendedMag.iValue == MOD_ON )
				label->setText( CHudTextMessage::BufferedLocaliseTextString( "#sExtendedMag_Add" ) );

				if( MenuButtons[i]->myModType.sGL.iValue == MOD_ON )
				label->setText( CHudTextMessage::BufferedLocaliseTextString( "#sGL_Add" ) );

				if( MenuButtons[i]->myModType.sLaser.iValue == MOD_ON )
				label->setText( CHudTextMessage::BufferedLocaliseTextString( "#sLaser_Add" ) );

				if( MenuButtons[i]->myModType.sFireMode.iValue == MOD_ON )
				label->setText( CHudTextMessage::BufferedLocaliseTextString( "#sFireMode_Add" ) );

				label->setFgColor(255, 255, 255, MenuButtons[i]->iAlpha);
			}
		}
	}



//	helper->setFgColor(0, 0, 0, 255);//hidden
//	helper->setPos( 0, 0 );


	//just changes the colour to let the player know is going to close that
	if( mbutton_close->isArmed() )
		mbutton_close->m_pTGA->setColor( Color(255,0,0, mbutton_close->iAlpha) );
	else
		mbutton_close->m_pTGA->setColor( Color(255,255,255, mbutton_close->iAlpha) );



	/*	
	if( mbutton->isArmed() )
	{
		mbutton->m_pTGA->setColor( Color(0,0,255, mbutton->iAlpha) );
		
		mbutton->getPos( button_x, button_y );
		label->setPos( button_x, button_y );

		if( mbutton->iAlpha == ALPHA_VISIBLE )
		{
			label->setText( CHudTextMessage::BufferedLocaliseTextString( "#RedDot_Add" ) );
			label->setFgColor(255, 0, 0, mbutton->iAlpha);
		}

		//doesn't quite work well on window mode, I'll leave this example here anyways
		
//		if( mbutton->iHelper == POP_UP_TIME )
//		{
//			helper->setFgColor(0, 0, 0, mbutton->iAlpha);
//			int x, y;
//
//			getApp()->getCursorPos(x,y);
//			helper->setPos( x, y );
//
//			helper->setText( "Do click to add" );
//		}
	}
	else
		mbutton->m_pTGA->setColor( Color(255,0,0, mbutton->iAlpha) );

	if( mbutton_TEST->isArmed() )//handle custom code here
	{
		mbutton_TEST->m_pTGA->setColor( Color(255,255,255, mbutton_TEST->iAlpha) );
	
		mbutton_TEST->getPos( button_x, button_y );
		label->setPos( button_x, button_y );
	
		if( mbutton_TEST->iAlpha == ALPHA_VISIBLE )
		{
			label->setText( CHudTextMessage::BufferedLocaliseTextString( "#Silencer_Add" ) );
			label->setFgColor(255, 0, 0, mbutton_TEST->iAlpha);
		}
	}
	else
		mbutton_TEST->m_pTGA->setColor( Color(255,255,255, mbutton_TEST->iAlpha) );
*/
}



void CWeaponMenuPanel::actionPerformed(Panel* panel)
{
	PlaySound("common/wpn_hudoff.wav", 1);
	CloseWindow();
}


// return 0 to hook key
// return 1 to allow key
int CWeaponMenuPanel::KeyInput(int down, int keynum, const char *pszCurrentBinding)
{
	if (!down)
		return 1; // dont disable releasing of the keys

	switch (keynum)
	{
	// close window
	case K_ENTER:
	case K_ESCAPE:
	{
		CloseWindow();
		return 0;
	}

	case 96:
	case 126:
/*	case K_F1:
	case K_F2:
	case K_F3:
	case K_F4:
	case K_F5:
	case K_F6:
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

void CWeaponMenuPanel::CloseWindow()
{
	setVisible(false);
	gViewPort->UpdateCursorState();
}

CWeaponMenuPanel :: ~CWeaponMenuPanel()
{
}
