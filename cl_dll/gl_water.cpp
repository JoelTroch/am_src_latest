/*

  It's a modern-new blur with class system.
  I newer use a classes, only procedual programming,
  Let me test it :)
*/

#include "hud.h"
#include "cl_util.h"
#include "glmanager.h"
#include "../common/com_model.h"

#include "water.h"
#include "gl_texloader.h"

CWaterEffect gWater;

void CWaterEffect::InitWater()
{
	inited = false;
	numSurfaces = 0;
	for(int i=0;i<MAX_WATER_SURFACES;i++)
	{
		surfaces[i] = NULL;
	}

/*	if(!shaderssupports)
	{
		gEngfuncs.Con_Printf("CWaterEffect::InitWater() Failed, your videocard doesn't support ARB shaders.\n");
		return;
	}
	*/

	//Now init a textures:
	CreateEmptyTex(ScreenWidth,ScreenHeight,screentex,GL_TEXTURE_2D,GL_RGB,true);

	if(!LoadShaderFile("shaders/cubemap_water_f.asm",fp_cubemap_water))
	{
		gEngfuncs.Con_Printf("Unable to load water frag. shader\n");
		return;
	}
	else
		gEngfuncs.Con_Printf("Water frag. shader loaded successfully\n");

	if(!LoadShaderFile("shaders/cubemap_water_noblur_f.asm",fp_cubemap_water_noblur))
	{
		gEngfuncs.Con_Printf("Unable to load water noblur frag. shader\n");
		return;
	}
	else
		gEngfuncs.Con_Printf("Water frag. noblur shader loaded successfully\n");

	gEngfuncs.Con_Printf("Water has been inited;\n");
	waterTexture = LoadCacheTexture("gfx/water.tga",MIPS_YES,false);
	inited = true;
}

void CWaterEffect::CaptureScreen()
{
	gl.glBindTexture(GL_TEXTURE_2D, screentex);
	gl.glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, ScreenWidth, ScreenHeight, 0);
}

void CWaterEffect::EmitWaterPolys( msurface_t *surf )
{
	glpoly_t *polys = surf->polys;
	glpoly_t	*p;
	float	*v, nv;
	float	s, t, os, ot;
	int	i;
	
	float time = gEngfuncs.GetClientTime();
	float waveHeight = 0.0;

	for( p = polys; p; p = p->next )
	{
		gl.glBegin( GL_POLYGON );

		for( i = 0, v = p->verts[0]; i < p->numverts; i++, v += VERTEXSIZE )
		{
			if( waveHeight )
			{
				nv = v[2] + waveHeight + ( waveHeight * sin(v[0] * 0.02f + time)
					* sin(v[1] * 0.02 + time) * sin(v[2] * 0.02f + time));
				nv -= waveHeight;
			}
			else nv = v[2];

			os = v[3];
			ot = v[4];

			s = os;
			s *= ( 1.0f / SUBDIVIDE_SIZE );

			t = ot;
			t *= ( 1.0f / SUBDIVIDE_SIZE );

			s+= cos(time/10.0f);
			t+= sin(time/10.0f);

			glNormal3f(0,0,1);
			gl.glTexCoord2f( s, t );
			gl.glVertex3f( v[0], v[1], nv );
		}
		gl.glEnd();
	}
}

void CWaterEffect::DrawWaterPoly(msurface_t *surf)
{

	gl.glEnable(GL_POLYGON_OFFSET_FILL);
	gl.glPolygonOffset ( -1, -1 );

	gl.glEnable(GL_BLEND);
	gl.glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	gl.glColor4f(1,1,1,1.0);

	gl.SaveStates();

	vec3_t point = surf->polys->verts[0];
	int cubeid = GetCubemapIdByPoint(point);

	if(cubeid < 0)
		return;

	gl.glActiveTextureARB(GL_TEXTURE0_ARB);
	gl.glBindTexture(GL_TEXTURE_2D,waterTexture);
	gl.glActiveTextureARB(GL_TEXTURE2_ARB);
	gl.glBindTexture(GL_TEXTURE_2D,screentex);
	
	gl.glActiveTextureARB(GL_TEXTURE1_ARB);
	SetupCubemapWater(cubeid,fp_cubemap_water,fp_cubemap_water_noblur);
	EmitWaterPolys(surf);
	DisableCubemap();
	gl.glDisable(GL_TEXTURE_2D);

	gl.glActiveTextureARB(GL_TEXTURE2_ARB);
	gl.glDisable(GL_TEXTURE_2D);

	gl.glActiveTextureARB(GL_TEXTURE0_ARB);
	gl.glDisable(GL_TEXTURE_CUBE_MAP_ARB);
	gl.glEnable(GL_TEXTURE_2D);

	gl.RestoreStates();
	gl.glDisable(GL_POLYGON_OFFSET_FILL);
}

void CWaterEffect::AddToChain(msurface_t *surf)
{
	numSurfaces++;
	surfaces[numSurfaces-1] = surf;
}

void CWaterEffect::DrawWater()
{
	for(int i=0;i<numSurfaces;i++)
	{
		msurface_t *surf = surfaces[i];
		DrawWaterPoly(surf);
	}

	numSurfaces = 0;
	for(int i=0;i<numSurfaces;i++)
	{
		surfaces[i] = NULL;
	}
}

void CWaterEffect::ResetWater()
{
}
