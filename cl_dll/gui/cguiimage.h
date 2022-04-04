/*
CGuiImage - provides ability to load and draw image.
Written by HAWK (Alexandr Bakanov) 04.02.2015.
*/
#include "cguipanel.h"
#include "gl_texloader.h"

#ifndef __GUIIMAGE__
#define __GUIIMAGE__

enum
{
	IMAGEMODE_STRETCH = 0,
	IMAGEMODE_REPEAT,
	IMAGEMODE_NOREPEAT,
};

class CGuiImage : public CGuiPanel
{
public:
	CGuiImage(char *image,int x = 64, int y = 64, int w = 320, int h = 320, CGuiPanel *_parent = NULL);

	virtual int drawPanel(float flTime);
	virtual int type() { return PANEL_PICTURE; }
	virtual void setImageMode(short mode) { imageMode = mode; }
	virtual void setImage(char *image);
	virtual void setTexture(cl_texture_t *texture);
	virtual void setTexture(int tex, short w, short h);
	virtual int passableFlags() { return PASSABLE_BUTFULLMOUSE; }
	virtual bool mouseClickEvent(int button, int down){ return true; }
protected:
	short imgw, imgh;
	short imageMode;
	cl_texture_t *texture;
	unsigned int rawTexture;
};
#endif
