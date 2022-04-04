/*
CGuiImage - provides ability to load and draw image.
Written by HAWK (Alexandr Bakanov) 04.02.2015.
*/
#include "hud.h"
#include "cl_util.h"

#include "glmanager.h"
#include "gl_renderer.h"
#include "cguiimage.h"
#include "glStuff.h"

CGuiImage::CGuiImage(char *image, int x, int y, int w, int h, CGuiPanel *_parent) : CGuiPanel(x,y,w,h,_parent)
{
	texture = LoadTextureWide(image);
}

int CGuiImage::drawPanel(float flTime)
{
	if (!visible)
		return 0;

	//HAWK UNDONE
	/*
	We need to write the code which will check imageMode
	and draw image according to selected mode.
	*/
	gl.glColor4f(1, 1, 1, 1);

	if (texture && texture->iIndex)
		gl.glBindTexture(GL_TEXTURE_2D, texture->iIndex);
	else
		gl.glBindTexture(GL_TEXTURE_2D, rawTexture);
	
	gl.glDisable(GL_BLEND);
	gGlStuff.DrawQuadRAW(xpos,ypos, xpos+pwidth, ypos+pheight);

	drawChildren(flTime);
	return 1;
}

void CGuiImage::setImage(char *imagePath)
{
	texture = LoadTextureWide(imagePath);
}

void CGuiImage::setTexture(int raw, short w, short h)
{
	rawTexture = raw;
	imgw = w;
	imgh = h;
}

void CGuiImage::setTexture(cl_texture_t *tex)
{
	texture = tex;
}