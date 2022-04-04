//
// written by BUzer for HL: Paranoia modification
//
//		2006

#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "triangleapi.h"
#include "com_model.h"
#include "studio_util.h"
#include "glmanager.h"
#include "gl_texloader.h"

#define DEAD_GRAYSCALE_TIME		5


cvar_t *v_posteffects;
cvar_t *v_grayscale;
extern cvar_t *cv_specular_nocombiners;



float GetGrayscaleFactor()
{
	float grayscale = v_grayscale->value;
	return 0;
	/*
	if (gHUD.m_flDeadTime)
	{
		float fact = (gEngfuncs.GetClientTime() - gHUD.m_flDeadTime) / DEAD_GRAYSCALE_TIME;
		if (fact > 1) fact = 1;
		if (fact > grayscale) grayscale = fact;
	}

	return grayscale;*/
}


int g_screen_texture = 0;
int g_weights_texture = 0; //for cards with no register combiners support

int UseRectangleTextures()
{
	if (gl.texture_rectangle_supported &&
		gl.texture_rectangle_max_size >= ScreenWidth &&
		gl.texture_rectangle_max_size >= ScreenHeight)
		return TRUE;

	return FALSE;
}


void InitScreenTexture()
{
	if (g_screen_texture)
		return;
	
	if (!UseRectangleTextures())
	{
		// just create 256x256 texture
		unsigned char* pBlankTex = new unsigned char[256*256*3];
		memset(pBlankTex, 0, 256*256*3);
		int oldbinding;
		gl.glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldbinding);
		gl.glBindTexture(GL_TEXTURE_2D, current_ext_texture_id);
		gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		gl.glTexImage2D (GL_TEXTURE_2D, 0, 3, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, pBlankTex);
		gl.glBindTexture(GL_TEXTURE_2D, oldbinding);
		delete[] pBlankTex;
		g_screen_texture = current_ext_texture_id;
		current_ext_texture_id++;
		gEngfuncs.Con_DPrintf("Grayscale: created 256x256 2D texture\n");
		return;
	}

	unsigned char* pBlankTex = new unsigned char[ScreenWidth*ScreenHeight*3];
	memset(pBlankTex, 255, ScreenWidth*ScreenHeight*3);

	gl.glEnable(GL_TEXTURE_RECTANGLE_NV);
	gl.glBindTexture(GL_TEXTURE_RECTANGLE_NV, current_ext_texture_id);
	gl.glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gl.glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl.glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 3, ScreenWidth, ScreenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, pBlankTex);
	gl.glDisable(GL_TEXTURE_RECTANGLE_NV);
	gEngfuncs.Con_DPrintf("Grayscale: created %dx%d rectangle texture\n", ScreenWidth, ScreenHeight);

	g_screen_texture = current_ext_texture_id;
	current_ext_texture_id++;
	delete[] pBlankTex;
}


void MakeWeightsTexture()
{
	if (g_weights_texture)
		return;

	unsigned char buf[16*16*3];
	for (int i=0; i<(16*16); i++)
	{
		buf[3*i+0] = 168;
		buf[3*i+1] = 202;
		buf[3*i+2] = 139;
	}

	int oldbinding;
	gl.glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldbinding);
	gl.glBindTexture(GL_TEXTURE_2D, current_ext_texture_id);
	gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl.glTexImage2D (GL_TEXTURE_2D, 0, 3, 16, 16, 0, GL_RGB, GL_UNSIGNED_BYTE, buf);
	gl.glBindTexture(GL_TEXTURE_2D, oldbinding);
	g_weights_texture = current_ext_texture_id;
	current_ext_texture_id++;
	gEngfuncs.Con_DPrintf("Grayscale: created weights texture\n");
}


void DrawQuad(	int width, int height, int ofsX = 0, int ofsY = 0)
{
	gl.glTexCoord2f(ofsX,ofsY);
	gl.glVertex3f(0, 1, -1);
	gl.glTexCoord2f(ofsX,height+ofsY);
	gl.glVertex3f(0, 0, -1);
	gl.glTexCoord2f(width+ofsX,height+ofsY);
	gl.glVertex3f(1, 0, -1);
	gl.glTexCoord2f(width+ofsX,ofsY);
	gl.glVertex3f(1, 1, -1);
}



void ApplyPostEffects()
{
	if (!gl.IsGLAllowed() || !v_posteffects->value)
		return;

	float grayscale = GetGrayscaleFactor();
	if (grayscale <= 0 || grayscale > 1)
		return;

	InitScreenTexture();
	int texenvmode1, texenvmode2, oldbinding1, oldbinding2;

	// setup ortho projection
	gl.glMatrixMode(GL_MODELVIEW);
	gl.glPushMatrix();
	gl.glLoadIdentity();
	gl.glMatrixMode(GL_PROJECTION);
	gl.glPushMatrix();
	gl.glLoadIdentity();
	gl.glOrtho(0, 1, 1, 0, 0.1, 100);

	int blendenabled = gl.glIsEnabled(GL_BLEND);
	gl.glEnable(GL_BLEND);
	gl.glColor4f(1, 1, 1, grayscale);
	gl.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//
	// Setup grayscale shader
	//
	if (gl.NV_combiners_supported && !cv_specular_nocombiners->value)
	{
		// use combiners
		GLfloat grayscale_weights[] = { 0.320000, 0.590000, 0.090000, 0.000000 };
		gl.glCombinerParameteriNV(GL_NUM_GENERAL_COMBINERS_NV, 1);
		
		// RC 1 setup: 
		// spare0.rgb = dot(tex0.rgb, {0.32, 0.59, 0.09})
		gl.glCombinerParameterfvNV(GL_CONSTANT_COLOR0_NV, grayscale_weights);
		gl.glCombinerInputNV (	GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_A_NV, GL_CONSTANT_COLOR0_NV,
								GL_SIGNED_IDENTITY_NV, GL_RGB);
		gl.glCombinerInputNV (	GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_B_NV, GL_TEXTURE0_ARB,
								GL_SIGNED_IDENTITY_NV, GL_RGB);
		gl.glCombinerOutputNV(	GL_COMBINER0_NV, GL_RGB,
								GL_SPARE0_NV,          // AB output
								GL_DISCARD_NV,         // CD output
								GL_DISCARD_NV,         // sum output
								GL_NONE, GL_NONE,
								GL_TRUE,               // AB = A dot B
								GL_FALSE, GL_FALSE );

		// Final RC setup:
		//	out.rgb = spare0.rgb
		//	out.a = spare0.a
		gl.glFinalCombinerInputNV ( GL_VARIABLE_A_NV, GL_ZERO,
									GL_UNSIGNED_IDENTITY_NV, GL_RGB );
		gl.glFinalCombinerInputNV ( GL_VARIABLE_B_NV, GL_ZERO,
									GL_UNSIGNED_IDENTITY_NV, GL_RGB );
		gl.glFinalCombinerInputNV ( GL_VARIABLE_C_NV, GL_ZERO,
									GL_UNSIGNED_IDENTITY_NV, GL_RGB );
		gl.glFinalCombinerInputNV ( GL_VARIABLE_D_NV, GL_SPARE0_NV,
									GL_UNSIGNED_IDENTITY_NV, GL_RGB );
		gl.glFinalCombinerInputNV ( GL_VARIABLE_E_NV, GL_ZERO,
									GL_UNSIGNED_IDENTITY_NV, GL_RGB );
		gl.glFinalCombinerInputNV ( GL_VARIABLE_F_NV, GL_ZERO,
									GL_UNSIGNED_IDENTITY_NV, GL_RGB );
		gl.glFinalCombinerInputNV ( GL_VARIABLE_G_NV, GL_PRIMARY_COLOR_NV,
									GL_UNSIGNED_IDENTITY_NV, GL_ALPHA );

		gl.glEnable(GL_REGISTER_COMBINERS_NV);
	}
	else
	{
		// use env_dot3
		MakeWeightsTexture();

		// 1st TU:
		// out = texture * 0.5 + 0.5
		gl.glActiveTextureARB(GL_TEXTURE0_ARB);
		gl.glGetTexEnviv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, &texenvmode1);

		GLfloat temp[] = { 0.5, 0.5, 0.5, 0.5 };
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
		gl.glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, temp);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_INTERPOLATE_ARB);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_TEXTURE);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_PRIMARY_COLOR_ARB); // {1, 1, 1}
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_ARB, GL_CONSTANT_ARB); // {0.5, 0.5, 0.5}
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1);

		// 2nd TU:
		// make dot3
		gl.glActiveTextureARB(GL_TEXTURE1_ARB);
		gl.glGetTexEnviv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, &texenvmode2);
		gl.glEnable(GL_TEXTURE_2D);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_DOT3_RGB_ARB);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_PREVIOUS_ARB);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_TEXTURE);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1);

		gl.glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldbinding2);
		gl.glBindTexture(GL_TEXTURE_2D, g_weights_texture);

		gl.glActiveTextureARB(GL_TEXTURE0_ARB);
	}

	// copy screen to texture
	if (UseRectangleTextures())
	{
		gl.glEnable(GL_TEXTURE_RECTANGLE_NV);
		gl.glBindTexture(GL_TEXTURE_RECTANGLE_NV, g_screen_texture);
	//	gl.glCopyTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_RGB, 0, 0, ScreenWidth, ScreenHeight, 0);
		gl.glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 0, 0, 0, 0, ScreenWidth, ScreenHeight);

		gl.glBegin(GL_QUADS);
		DrawQuad(ScreenWidth, ScreenHeight);
		gl.glEnd();

		gl.glDisable(GL_TEXTURE_RECTANGLE_NV);
	}
	else
	{
		gl.glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldbinding1);
		gl.glBindTexture(GL_TEXTURE_2D, g_screen_texture);

		int ofsy2 = ScreenHeight;
		for(int ofsy = 0; ofsy < ScreenHeight; ofsy+=256, ofsy2-=256)
		{
			for(int ofsx = 0; ofsx < ScreenWidth; ofsx+=256)
			{
				gl.glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, ofsx, ofsy, 256, 256);
				gl.glBegin(GL_QUADS);
					gl.glTexCoord2f(0,0);
					gl.glVertex3f(ofsx/(float)ScreenWidth, ofsy2/(float)ScreenHeight, -1);
	
					gl.glTexCoord2f(1,0);
					gl.glVertex3f((ofsx+256)/(float)ScreenWidth, ofsy2/(float)ScreenHeight, -1);
					
					gl.glTexCoord2f(1,1);
					gl.glVertex3f((ofsx+256)/(float)ScreenWidth, (ofsy2-256)/(float)ScreenHeight, -1);
					
					gl.glTexCoord2f(0,1);
					gl.glVertex3f(ofsx/(float)ScreenWidth, (ofsy2-256)/(float)ScreenHeight, -1);
				gl.glEnd();
			}
		}

		gl.glBindTexture(GL_TEXTURE_2D, oldbinding1);
	}

	if (gl.NV_combiners_supported && !cv_specular_nocombiners->value)
	{
		gl.glDisable(GL_REGISTER_COMBINERS_NV);
	}
	else
	{
		gl.glActiveTextureARB(GL_TEXTURE1_ARB);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, texenvmode2);
		gl.glBindTexture(GL_TEXTURE_2D, oldbinding2);
		gl.glDisable(GL_TEXTURE_2D);
		gl.glActiveTextureARB(GL_TEXTURE0_ARB);
		gl.glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, texenvmode1);
	}

	gl.glMatrixMode(GL_PROJECTION);
	gl.glPopMatrix();
	gl.glMatrixMode(GL_MODELVIEW);
	gl.glPopMatrix();

	if (!blendenabled) gl.glDisable(GL_BLEND);
}



void InitPostEffects()
{
	v_posteffects = gEngfuncs.pfnRegisterVariable( "gl_posteffects", "1", 0 );
	v_grayscale = gEngfuncs.pfnRegisterVariable( "gl_grayscale", "0", 0 );
}
