/*
CGui - It's main gui manager.
Written by HAWK (Alexandr Bakanov) 04.02.2015.
*/

#include "cguipanel.h"

#ifndef __GUIMAIN__
#define __GUIMAIN__

class CGui
{
public:
	CGui();
	~CGui();
	CGuiPanel *mainPanel;
	void draw(float flTime);
	void think(float flTime);
	void mouseClick(int button,int down);
	void keyEvent(int key, int down, char*bind);
	int pointer;

	int mx, my;
	bool visible;
};

extern CGui *gGui;
#endif
