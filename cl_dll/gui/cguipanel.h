/*
	CGuiPanel - base gui class used by any other.
	All new controls should be interheated from this class.

	Written by HAWK (Alexandr Bakanov) 02.02.2015.
*/
#ifndef __GUIPANEL__
#define __GUIPANEL__
#include <vector>

enum{
	PANEL_BASEPANEL = 0,
	PANEL_PICTURE,
	PANEL_LABEL,
};

enum{
	PASSABLE_YES = 0,
	PASSABLE_BUTMOUSE = 1,
	PASSABLE_BUTMOUSECLICK = 2,
	PASSABLE_BUTKEYBOARD = 4,
	PASSABLE_BUTFULLMOUSE = PASSABLE_BUTMOUSE | PASSABLE_BUTMOUSECLICK,
	PASSABLE_NO = PASSABLE_BUTFULLMOUSE | PASSABLE_BUTKEYBOARD,
};


class CGuiPanel
{
public:
	CGuiPanel(int x=64, int y=64, int w=320, int h=320, CGuiPanel *_parent = NULL);
	~CGuiPanel();

	//generic.
	virtual int drawPanel(float flTime);
	virtual int x() { return xpos; }
	virtual int y() { return ypos; }
	virtual int width() { return pwidth; }
	virtual int height() { return pheight; }
	virtual void setpos(int x, int y) { reposition(xpos, x, ypos, y); }
	virtual void setsize(int x, int y) { resize(pwidth, x, pheight, y); }
	virtual int type() { return PANEL_BASEPANEL; }
	virtual CGuiPanel *getParent() { return parent; }
	virtual void resize(int oldw, int w, int oldh, int h) { pwidth = w; pheight = h; };
	virtual void reposition(int oldx, int x, int oldy, int y) { xpos = x; ypos = y; };
	virtual void addChild(CGuiPanel *panel);

	//focusing.
	virtual int passableFlags() { return PASSABLE_YES; }
	virtual int canTabTag() { return tabtagging; }
	virtual void setTabTagging(int value) { tabtagging = value; }
	virtual CGuiPanel *focusedPanel() { return focused; }
	virtual void setFocused(CGuiPanel *p) { focused = p; }

	//mouse
	void mouseHover();
	void mouseLeave();
	bool mouseClick(int button,int down);

	virtual void mouseHoverEvent() {};
	virtual void mouseLeaveEvent() {};
	virtual bool mouseClickEvent(int button,int down) { return false; };
	virtual bool isMouseCatched(int x, int y);
	
	//keyboard
	virtual bool keyButtonPress(int key);
	virtual bool keyButtonRelease(int key);

	virtual void show() { visible = true; }
	virtual void hide() { visible = false; }
	virtual void toggleVisible() { visible = !visible; }
	virtual bool isVisible() { return visible; }
	void think(float flTime);
	virtual void preThink(float flTime);
	virtual void drawChildren(float flTime);
	virtual void thinkChildren(float flTime);

	virtual bool mouseIntoPanel() { return mouseInto; }
	void setNextThink(float t) {nextThink = t;}
	float getNextThink() { return nextThink; }

	void setThink(void (*tf)(float flTime)){ thinkFunc = tf; }
protected:
	void(*thinkFunc)(float flTime);
	float nextThink;
	bool mouseInto;
	bool visible;
	CGuiPanel *parent;
	int xpos, ypos, pwidth, pheight, tabtagging;
	std::vector<CGuiPanel*> children;
	CGuiPanel *focused;
};
#endif
