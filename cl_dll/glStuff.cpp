#include "STDIO.H"
#include "STDLIB.H"
#include "MATH.H"

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include <string.h>

#include "com_model.h"

#include "glmanager.h"
#include "gl_texloader.h"
#include "gl_renderer.h"

#include "vgui_TeamFortressViewport.h"

#include "glStuff.h"
glStuff gGlStuff;

glStuff::glStuff(){}
glStuff::~glStuff(){}

void glStuff::DrawQuadS(float a, float b, int c, float d, float e, bool f)
{
	DrawQuad(d, b, c, a, e, f);
}

void glStuff::DrawQuadRAW(int x, int y, int ex, int ey)
{
	glBegin(GL_QUADS);
	glTexCoord2f(1, 1);
	glVertex3f(x, y, 0);
	glTexCoord2f(1, 0);
	glVertex3f(x, ey, 0);
	glTexCoord2f(0, 0);
	glVertex3f(ex, ey, 0);
	glTexCoord2f(0, 1);
	glVertex3f(ex, y, 0);
	glEnd();
}

void glStuff::DrawQuadRAW(int x, int y, int ex, int ey, float tx, float ty, float tex, float tey)
{
	glBegin(GL_QUADS);
	glTexCoord2f(tex, tey);
	glVertex3f(x, y, 0);
	glTexCoord2f(tex, ty);
	glVertex3f(x, ey, 0);
	glTexCoord2f(tx, ty);
	glVertex3f(ex, ey, 0);
	glTexCoord2f(tx, tey);
	glVertex3f(ex, y, 0);
	glEnd();
}

void DrawQuad(int x,int y,int texture,int ex,int ey,bool a)
{
	glEnable(GL_TEXTURE_2D);

	if(texture >= 0)
	glBindTexture( GL_TEXTURE_2D, texture);

	glEnable(GL_BLEND);

	if(!a)
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	else
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    glBegin(GL_QUADS);
	glTexCoord2f(1, 1);
    glVertex3f(x, y,0);
	glTexCoord2f(1, 0);
    glVertex3f(x,ey,0);
    glTexCoord2f(0,0);
    glVertex3f(ex,ey,0);
    glTexCoord2f(0, 1);
    glVertex3f(ex,y,0);

    glEnd();
}

void glStuff::DrawQuad(int x, int y, int texture, int ex, int ey, bool a)
{
	glEnable(GL_TEXTURE_2D);

	if (texture >= 0)
		glBindTexture(GL_TEXTURE_2D, texture);

	glEnable(GL_BLEND);

	if (!a)
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	else
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	DrawQuadRAW(x, y, ex, ey);
}

void glStuff::DrawQuadWH(int x, int y, int texture, int w, int h, bool a)
{
	DrawQuad(x, y, texture, x + w, y + h, a);
}

int fasttxt[128];

void LoadTextures()
{
	
	fasttxt[5] = CreateTexture("gfx/vgui/hud/noise.tga", MIPS_NO);

}

void glStuff::DrawScreenOV()
{
	if (gEngfuncs.pfnGetCvarFloat("cl_noise") == 1)
	{
		glColor4f(1, 1, 1, 1);
		DrawQuad(0, 0, fasttxt[5], ScreenWidth, ScreenHeight, true);
	}
}
