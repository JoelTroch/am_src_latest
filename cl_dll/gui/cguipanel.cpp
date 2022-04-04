#include "hud.h"
#include "cl_util.h"
#include "glmanager.h"

#include "cguipanel.h"
#include "cgui.h"

int CGuiPanel::drawPanel(float flTime)
{
	if (!visible)
		return 0;

	drawChildren(flTime);
	return 1;
}

void CGuiPanel::drawChildren(float flTime)
{
	for (int i = 0; i < children.size(); i++)
	{
		gl.glTranslatef(x(), y(), 0);
		if (children.at(i))
			children.at(i)->drawPanel(flTime);
		gl.glTranslatef(-x(), -y(), 0);
	}
}

void CGuiPanel::mouseHover(){
	mouseInto = true;
	mouseHoverEvent();
}

void CGuiPanel::mouseLeave(){
	mouseInto = false;
	mouseLeaveEvent();
}

bool CGuiPanel::keyButtonPress(int button)
{
	if (!(passableFlags() & PASSABLE_BUTKEYBOARD) || !isVisible())
		return false;
	return true;
}

bool CGuiPanel::keyButtonRelease(int button)
{
	if (!(passableFlags() & PASSABLE_BUTKEYBOARD) || !isVisible())
		return false;
	return true;
}

void CGuiPanel::think(float flTime)
{
	int mx, my;
	mx = gGui->mx;
	my = gGui->my;

	preThink(flTime);

	if (nextThink > 0 && nextThink <= flTime && thinkFunc != NULL)
		thinkFunc(flTime);

	thinkChildren(flTime);

	if (passableFlags() == PASSABLE_YES) return;
	
	if (isMouseCatched(mx, my)){
		if (!mouseIntoPanel())
			mouseHover();
	}else{
		if (mouseIntoPanel()) mouseLeave();
	}
}

void CGuiPanel::thinkChildren(float flTime)
{
	for (int i = 0; i < children.size(); i++)
	if (children.at(i))
		children.at(i)->think(flTime);
}

void CGuiPanel::preThink(float flTime)
{

}

bool CGuiPanel::isMouseCatched(int xx, int yy)
{
	if (!(passableFlags() & PASSABLE_BUTMOUSE) || !isVisible())
		return false;

	if (xx >= x() && yy >= y() &&
		xx <= x() + width() && yy <= y() + height())
	{
		return true;
	}

	return false;
}

bool CGuiPanel::mouseClick(int button,int down)
{
	if (!isVisible())
		return false;
	
	for (int i = 0; i < children.size(); i++)
	if (children.at(i))
	if (children.at(i)->mouseClick(button, down))
		return true;

	if (isMouseCatched(gGui->mx, gGui->my))
	return mouseClickEvent(button, down);
	else return false;
}

CGuiPanel::CGuiPanel(int x, int y, int w, int h, CGuiPanel *_parent)
{
	parent = _parent;
	if (parent)
		parent->addChild(this);

	children.resize(0);
	xpos = x; ypos = y;
	pwidth = w; pheight = h;
	setTabTagging(true);
	thinkFunc = NULL;
	nextThink = -1;
	visible = true;
}

void CGuiPanel::addChild(CGuiPanel*pan)
{
	if (pan == 0)
		return;
	pan->parent = this;
	children.push_back(pan);
	int c = children.size();
	gEngfuncs.Con_Printf("%i children\n", c);
}

CGuiPanel::~CGuiPanel()
{
	for (int i = 0; i < children.size(); i++)
	{
		if (children.at(i))
		{
			delete children.at(i);
			children.at(i) = NULL;
		}
	}
}

