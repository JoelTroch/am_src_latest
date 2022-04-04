#include "hud.h"
#include "cl_util.h"
#include "glmanager.h"
#include "SSAO.h"

#include "gl_texloader.h"
#include "gl_shaders.h"
#include "glStuff.h"
#include "blur.h"

CSSAOEffect gSSAO;

#define BLURED_RESOLUTION 512

void CSSAOEffect::InitSSAO()
{
	if (inited)
		return;

	inited = false;

	//Now init a textures:
	CreateEmptyTex(ScreenWidth, ScreenHeight, screentex, GL_TEXTURE_2D, GL_RGB, true);
	CreateEmptyTex(BLURED_RESOLUTION, BLURED_RESOLUTION, bluredtex, GL_TEXTURE_2D, GL_RGB, true);
	CreateEmptyTex(ScreenWidth, ScreenHeight, ssaotex, GL_TEXTURE_2D, GL_RGB, true);

	CreateEmptyDepth(ScreenWidth, ScreenHeight, depthtex, true);

	/*	
	CVAR_CREATE("ssaonear", "400", FCVAR_ARCHIVE);
	CVAR_CREATE("ssaofar", "4096.0", FCVAR_ARCHIVE);
	CVAR_CREATE("ssaoradius", "10", FCVAR_ARCHIVE);
	CVAR_CREATE("ssaobias", "0.5", FCVAR_ARCHIVE);*/

	CVAR_CREATE("ssaonear", "3000", FCVAR_ARCHIVE);
	CVAR_CREATE("ssaofar", "6500.0", FCVAR_ARCHIVE);
	CVAR_CREATE("ssaoradius", "20", FCVAR_ARCHIVE);
	CVAR_CREATE("ssaobias", "0.3", FCVAR_ARCHIVE);
	CVAR_CREATE("prop_brightness", "0", FCVAR_ARCHIVE);


	CVAR_CREATE("ssaoblur", "0.1", FCVAR_ARCHIVE);

	inited = true;
}

void CSSAOEffect::DrawEffect()
{
	if (CVAR_GET_FLOAT("r_ssao") < 1.0)
		return;

	gl.SaveStates();

	gl.glActiveTextureARB(GL_TEXTURE0_ARB);
	gl.glBindTexture(GL_TEXTURE_2D, screentex);
	gl.glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, ScreenWidth, ScreenHeight, 0);

	gl.glBindTexture(GL_TEXTURE_2D, depthtex);
	gl.glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, 0, 0, ScreenWidth, ScreenHeight, 0);

	if (!SSAOShader)
		return;

	SSAOShader->bind();
	float local1[4];

	//m_pSSAORenderShader->SetParameter4f(0, 2.0f * m_zNear, m_zFar - m_zNear, (float)screenWidth, (float)screenHeight);
	//m_pSSAORenderShader->SetParameter4f(1, m_varSSAOMinZ, m_varSSAOMaxZ, m_varSSAOScale, 0);

	local1[0] = CVAR_GET_FLOAT("ssaonear");
	local1[1] = CVAR_GET_FLOAT("ssaofar");
	local1[2] = CVAR_GET_FLOAT("ssaoradius");
	local1[3] = CVAR_GET_FLOAT("ssaobias");
	gl.glUniform4fvARB(SSAOShader->local0, 1, local1);

	gl.bindToShader(SSAOShader->depth, 0, depthtex);

	gGlStuff.DrawQuadS(0, 0, -1, ScreenWidth, ScreenHeight, false);
	SSAOShader->unbind();
	gl.RestoreStates();

	//copy result.
	gl.glActiveTextureARB(GL_TEXTURE0_ARB);
	gl.glBindTexture(GL_TEXTURE_2D, ssaotex);
	gl.glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, ScreenWidth, ScreenHeight, 0);

	if (CVAR_GET_FLOAT("r_ssao") <= 1)//higher values disable blurring - DEBUG
	{
	//resize and blur
		gl.glViewport(0, 0, BLURED_RESOLUTION, BLURED_RESOLUTION);
			gGlStuff.DrawQuadS(0, 0, ssaotex, ScreenWidth, ScreenHeight, false);

			blurThis();

			SSAOMixShader->bind();
			gl.bindToShader(SSAOMixShader->screen, 1, screentex);
			gl.bindToShader(SSAOMixShader->blured, 0, bluredtex);

			gGlStuff.DrawQuadS(0, 0, -1, ScreenWidth, ScreenHeight, false);
			SSAOMixShader->unbind();
	}
	gl.RestoreStates();

	gl.glActiveTextureARB(GL_TEXTURE1_ARB);
	gl.glDisable(GL_TEXTURE_2D);
	gl.glActiveTextureARB(GL_TEXTURE2_ARB);
	gl.glDisable(GL_TEXTURE_2D);

	gl.glActiveTextureARB(GL_TEXTURE0_ARB);
}

void CSSAOEffect::blurThis(){
	float BLUR_STR = CVAR_GET_FLOAT("ssaoblur");
	gl.glColor4f(1, 1, 1, 1);
	gl.glDisable(GL_BLEND);

	gl.glActiveTextureARB(GL_TEXTURE0_ARB);
	gl.glBindTexture(GL_TEXTURE_2D, bluredtex);
	gl.glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, BLURED_RESOLUTION, BLURED_RESOLUTION, 0);

	gl.glEnable(GL_FRAGMENT_PROGRAM_ARB);
	gl.glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, gBlur.fp_blur);
	glProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 0, BLUR_STR, 0, 0, 0);//horizontal

	gGlStuff.DrawQuadS(0, 0, bluredtex, ScreenWidth, ScreenHeight, false);
	gl.glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, BLURED_RESOLUTION, BLURED_RESOLUTION, 0);

	glProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 0, 0, BLUR_STR, 0, 0);//vertical
	gGlStuff.DrawQuadS(0, 0, bluredtex, ScreenWidth, ScreenHeight, false);
	gl.glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, BLURED_RESOLUTION, BLURED_RESOLUTION, 0);
	gl.glDisable(GL_FRAGMENT_PROGRAM_ARB);

	//restore view to original.
	gl.glViewport(0, 0, ScreenWidth, ScreenHeight);
}