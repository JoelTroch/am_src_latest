#include "hud.h"
#include "cl_util.h"
#include "glmanager.h"
#include "ColorCorrection.h"

#include "gl_texloader.h"
#include "gl_shaders.h"
#include "glStuff.h"

CColorCorrection gColorCorrection;

void CColorCorrection::InitColorCorrection()
{
	if (inited)
		return;

	inited = false;

	//Now init a textures:
	CreateEmptyTex(ScreenWidth, ScreenHeight, screentex, GL_TEXTURE_2D, GL_RGB, true);

	gEngfuncs.Con_Printf("ColorCorrection has been inited;\n");
	r = g = b = s = ir = ig = ib = is = 1.0;

	inited = true;
}

void CColorCorrection::DrawEffect()
{

	float v1, v2, v3, v4;
	char *str = CVAR_GET_STRING("ccor");
	sscanf(str, "%f %f %f %f", &v1, &v2, &v3, &v4);

	bool dev = false;
	if (v1 != 1.0 || v2 != 1.0 || v3 != 1.0 || v4 != 1.0){
		dev = true;
	}

	InterpolateIt();

	if (r==1.0 && g == 1.0 && b == 1.0 && s == 1.0 && !dev)
	{
		gl.glColor4f(1, 1, 1, 1);
		gl.glBindTexture(GL_TEXTURE_2D, screentex);
		gl.glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, ScreenWidth, ScreenHeight, 0);
		gGlStuff.DrawQuadS(0, 0, screentex, ScreenWidth, ScreenHeight, false);
		return;
	}

	float values[4];
	values[0] = r; values[1] = g;
	values[2] = b; values[3] = s;

	gl.glColor4f(1, 1, 1, 1);

	gl.glEnable(GL_BLEND);
	gl.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	gl.glBindTexture(GL_TEXTURE_2D, screentex);
	gl.glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, ScreenWidth, ScreenHeight, 0);

	gl.glEnable(GL_BLEND);
	gl.glColor4f(1.0,1.0,1.0,1.0);

	if (colorCorrectionShader){
		colorCorrectionShader->bind();
		gl.glUniform4fvARB(colorCorrectionShader->rgbsat, 1, values);
		gGlStuff.DrawQuadS(0, 0, screentex, ScreenWidth, ScreenHeight, false);
		colorCorrectionShader->unbind();

		//gEngfuncs.Con_Printf("Draw this shit. %f %f %f %f\n");
	}

	if (colorCorrectionShader && dev){
		colorCorrectionShader->bind();
		values[0] = v1; values[2] = v3;
		values[1] = v2; values[3] = v4;
		gl.glUniform4fvARB(colorCorrectionShader->rgbsat, 1, values);

		glBindTexture(GL_TEXTURE_2D, screentex);
		glBegin(GL_QUADS);
			glTexCoord2f(0.0, 1);
			glVertex3f(0, 0, 0);
			glTexCoord2f(0.0, 0);
			glVertex3f(0, ScreenHeight, 0);
			glTexCoord2f(0.5, 0);
			glVertex3f(ScreenWidth/2, ScreenHeight, 0);
			glTexCoord2f(0.5, 1);
			glVertex3f(ScreenWidth/2, 0, 0);
		glEnd();

		colorCorrectionShader->unbind();
	}

}

void CColorCorrection::SetNew(float _r,float _g,float _b,float _s,float _ftime)
{
	ir = _r;
	ig = _g;
	ib = _b;
	is = _s;
	fadetime = _ftime;

	lastchanged = gEngfuncs.GetClientTime();
}

void CColorCorrection::ResetColorCorrection()
{
	r = g = b = s = ir = ig = ib = is = 1.0;
}

void CColorCorrection::InterpolateIt()
{
	if (fadetime <= 0)
		fadetime = 0.05;

	float frac = (gEngfuncs.GetClientTime() - lastchanged) / fadetime;

	if (frac < 0)
		return;
	if (frac > 1)
		return;

	r = LnrIntrpltn(r, ir, 1.0 - frac);
	g = LnrIntrpltn(g, ig, 1.0 - frac);
	b = LnrIntrpltn(b, ib, 1.0 - frac);
	s = LnrIntrpltn(s, is, 1.0 - frac);
}