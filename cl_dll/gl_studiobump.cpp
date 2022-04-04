#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "com_model.h"
#include "studio.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "dlight.h"
#include "triangleapi.h"

#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <math.h>

#include "studio_util.h"
#include "r_studioint.h"

#include "StudioModelRenderer.h"
#include "GameStudioModelRenderer.h"

#include "gl_renderer.h"
#include "gl_dlight.h"

extern engine_studio_api_t IEngineStudio;

unsigned int fp_bump_l1;
unsigned int vp_bump_l1;

void BumpInitShaders()
{
	bool f1 = LoadShaderFile("shaders/bump/bump1light.arf",fp_bump_l1,true);
	gEngfuncs.Con_Printf("Init bumpmaps: 1light: %i\n",(int)f1);
	bool v1 = LoadShaderFile("shaders/bump/bump1light.arv",vp_bump_l1,false);
	gEngfuncs.Con_Printf("Init bumpmaps: v1light: %i\n",(int)v1);
}

void BindBumpShader()
{
	gl.glEnable(GL_FRAGMENT_PROGRAM_ARB);
	gl.glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB,fp_bump_l1);
	gl.glEnable(GL_VERTEX_PROGRAM_ARB);
	gl.glBindProgramARB(GL_VERTEX_PROGRAM_ARB,vp_bump_l1);
}

void UnBindBumpShader()
{
	gl.glDisable(GL_FRAGMENT_PROGRAM_ARB);
	gl.glDisable(GL_VERTEX_PROGRAM_ARB);
}

