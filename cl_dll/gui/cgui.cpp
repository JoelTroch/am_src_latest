/*
CGui - It's main gui manager.
Written by HAWK (Alexandr Bakanov) 04.02.2015.
*/
#include "hud.h"
#include "cl_util.h"

#include "cguipanel.h"
#include "cguiimage.h"

#include "gl_texloader.h"
#include "cgui.h"
#include "glStuff.h"

CGui *gGui;

void IN_VoidActivateMouse(void);
void IN_VoidDeactivateMouse(void);

CGui::CGui()
{
	mainPanel = new CGuiPanel(0, 0, ScreenWidth, ScreenHeight);
	pointer = CreateTexture("gfx/gui/pointer.tga", MIPS_NO, 0);

	CGuiPanel *p = new CGuiImage("gfx/env/52h04bk.tga", 64, 64, 320, 320, mainPanel);
	new CGuiImage("gfx/env/dashnight256_bk.tga", 280, 256, 128, 64, p);
	new CGuiImage("gfx/env/dashnight256_bk.tga", 0, 0, 96, 96, p);
	new CGuiImage("gfx/env/dashnight256_bk.tga", 128, 128, 64, 64, p);

	visible = false;
}

CGui::~CGui()
{
	if (mainPanel)
		delete mainPanel;
}

void CGui::keyEvent(int key, int down, char *bind)
{
	if (mainPanel)
	{
		if (down)
			mainPanel->keyButtonPress(key);
		else 
			mainPanel->keyButtonRelease(key);
	}
}

void CGui::draw(float flTime)
{
	if (mainPanel)
	{
		mainPanel->setsize(ScreenWidth, ScreenHeight);
	}

	if (mainPanel && visible)
		mainPanel->drawPanel(flTime);

	if (!visible)
		return;

	gGlStuff.DrawQuad(mx, my, pointer, mx + 32, my + 32);
}

void CGui::think(float flTime)
{
	IN_GetMousePos(&mx, &my);
	/*
	if (visible)
	{
		IN_VoidActivateMouse();
	}
	else
	{
		IN_VoidDeactivateMouse();
	}*/

	if (mainPanel)
		mainPanel->think(flTime);
}

void CGui::mouseClick(int button, int down)
{
//	gEngfuncs.Con_DPrintf("try to click %i %i\n", button, down);

	if (mainPanel)
	{
		bool r = mainPanel->mouseClick(button, down);
	//	gEngfuncs.Con_Printf("Result = %i\n", (int)r);
	}

}