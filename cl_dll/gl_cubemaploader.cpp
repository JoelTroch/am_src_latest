/***
*
*	Copyright (c) 2007, 
*	
*   HAWK cubemap loader 
*   Big thanks to terror 
*	Tutor: http://www.gamedev.ru/code/articles/?id=4172
****/

#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "com_model.h"
#include "glmanager.h"
#include "gl_texloader.h"
#include "gl_shaders.h"

extern int current_ext_texture_id;
#define TEXBUFFER_SIZE	(4096*4096)
#define MAX_CUBEMAPS 512 //per map.
int cubemap_count = 1;
CubeMap cubicmaps[MAX_CUBEMAPS];
extern cvar_t *cv_cubemap;

int LoadCubeTexture(const char* filename,int cubeid,int target,int &cubewidth)
{
	int len;
	byte*buff=AM_LoadFile((char*)filename,5,&len); // a?ocei oaee
	
	int width,height; // ?acia?u ea?oeiee
	TGA_HEADER hdr; // Caaieiaie oaeea

	if (!buff)
	{
		gEngfuncs.Con_Printf("%s texture non found.\n",filename);
		return 0;
	}

	memcpy(&hdr,&buff[0],18);

	width=hdr.width;
	height=hdr.height;
	
	int type=GL_RGB;
	int bpp;

	switch(hdr.bits)
	{
	case 24:
		bpp=3;
		break;
	case 32:
		type=GL_RGBA;
		bpp=4;
		break;
	default:
		AM_FreeFile(buff);
		gEngfuncs.Con_Printf("%s CreateTexture - not 32/24 bpp texture!\n",filename); // iaoa?einy ?oi ea?oeiea  - aaiii
		return -1;
	}

	int imgsize=width*height*bpp+18;
	byte temp;

    if (!(hdr.descriptor & 0x20))  // bottom up, need to swap scanlines
    {
       unsigned char *temp = (unsigned char *)malloc(hdr.width * bpp);

       for (int i = 0; i < hdr.height/2; i++)
       {
          memcpy(temp, &buff[18] + i*hdr.width*bpp, hdr.width*bpp);
          memcpy(&buff[18] + i*hdr.width*bpp,&buff[18] + (hdr.height-i-1)*hdr.width*bpp, hdr.width*bpp);
          memcpy(&buff[18] + (hdr.height-i-1)*hdr.width*bpp, temp, hdr.width*bpp);
       }
	   free(temp);
    }

	for(GLuint i=18; i<int(imgsize); i+=bpp) 
	{                                                    
		temp=buff[i];                       
		buff[i]=buff[i+2];
		buff[i+2] = temp;                 
	}

	cubewidth = width;

	glHint( GL_GENERATE_MIPMAP_HINT_SGIS, GL_NICEST );
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_GENERATE_MIPMAP, GL_TRUE);
	glTexImage2D(target,0,type,width,height,0,type,GL_UNSIGNED_BYTE,&buff[18]);

	AM_FreeFile(buff);
	return cubeid;
}

void transpose ( const float *m1, float *m2 );

float VectorLengthSquared(const vec3_t v)
{
	return (v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}
float VectorLength(const vec3_t v)
{
	return sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

int GetCubemapIdByPoint(vec3_t point)
{
	float flLen = 2000000000;//hack
	int cubeId = -1;

	for(int i=1;i<cubemap_count;i++)
	{
		float nflLen = VectorLengthSquared(point - cubicmaps[i].position);
		if(nflLen < flLen)
		{
		//	gEngfuncs.Con_Printf("%f < %f\n",nflLen,flLen);
			flLen = nflLen;
			cubeId = i;
		}
	}
	
	if(cubeId < 0)
		return -1;

//	gEngfuncs.Con_Printf("Return cubemap: %s %i\n",cubicmaps[cubeId].texName,cubicmaps[cubeId].cubeid);
	return cubeId;
}

int GetExactCubeMap(vec3_t position)
{
	for(int i=1;i<cubemap_count;i++)
	{
		if(cubicmaps[i].position == position)
		{
			return i;
		}
	}
	return -1;
}

unsigned int fp_cubemap,vp_cubemap,fp_cubemap_noblur;

void InitCubemaps()
{
	if(fp_cubemap && vp_cubemap)
		return;

	bool f = LoadShaderFile("shaders/cubemap_f.asm",fp_cubemap,true);
	bool v = LoadShaderFile("shaders/cubemap_v.asm",vp_cubemap,false);
	bool fnb = LoadShaderFile("shaders/cubemap_f_noblur.asm",fp_cubemap_noblur,true);

	gEngfuncs.Con_Printf("Init cubemaps: %i -- %i -- %i\n",(int)f,(int)v,(int)fnb);
}

void ResetCubemaps()
{
	for (int i = 0; i < MAX_CUBEMAPS; i++)
	{
		CubeMap *map = &cubicmaps[i];
		glDeleteTextures(6,&map->textures[0]);
	}

	memset(&cubicmaps[0],0,sizeof(CubeMap)*MAX_CUBEMAPS);
	cubemap_count = 1;
}

int LoadCubeMap(vec3_t position,char *texName)
{
	InitCubemaps();

	char szMapName[64];
	void COM_FileBase ( const char *in, char *out);
	COM_FileBase( gEngfuncs.pfnGetLevelName(), szMapName );

	if(cubemap_count >= MAX_CUBEMAPS-2)
		return -1;

	int cacheCubemap = GetExactCubeMap(position);

	if(cacheCubemap >= 0)
	{
		return cacheCubemap;
	}

	int CubeMapTarget[6];
	CubeMapTarget[0] = GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB;
	CubeMapTarget[1] = GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB;
	CubeMapTarget[2] = GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB;
	CubeMapTarget[3] = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB;
	CubeMapTarget[4] = GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB;
	CubeMapTarget[5] = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB;

	const char *postfix[6] = {"_px.tga",
		"_nx.tga","_py.tga","_ny.tga","_pz.tga","_nz.tga"};

//	const char *postfix[6] = {"_nz.tga",
//		"_pz.tga","_px.tga","_nx.tga","_ny.tga","_py.tga"};

	char cuname[256];
	int cubewidth;

	glEnable(GL_TEXTURE_CUBE_MAP_ARB);
	glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, current_ext_texture_id);
	glTexParameterf ( GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameterf ( GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexParameterf ( GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
	glTexParameteri ( GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
	glTexParameteri ( GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	for ( int i = 0; i < 6; i++ )
	{
		sprintf(cuname,"gfx/cubemaps/%s/%s%s",szMapName,texName,postfix[i]);
		int ret = LoadCubeTexture(cuname,current_ext_texture_id,CubeMapTarget[i],cubewidth);
		gEngfuncs.Con_Printf("Loading texture %s in cubemap %i\n",cuname,cubemap_count);

		if(ret < 0)
		{
			gEngfuncs.Con_Printf("Unable to load %s in cubemap %s\n",cuname,texName);
		}

		cubicmaps[cubemap_count].textures[i] = ret;
	}

	glDisable(GL_TEXTURE_CUBE_MAP_ARB);
	glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, -1);

	if(cubewidth <= 0)
		cubewidth = 16;

	cubicmaps[cubemap_count].cubeid = current_ext_texture_id;
	cubicmaps[cubemap_count].cubewidth = cubewidth;
	sprintf(cubicmaps[cubemap_count].texName,texName);
	cubicmaps[cubemap_count].position = position;
	current_ext_texture_id++;

	gEngfuncs.Con_Printf("Texture %s has cubemap with index %i\n",texName,cubicmaps[cubemap_count].cubeid);

	cubemap_count++;
	return (cubemap_count-1);
}

int CubemapFilterEntities(int type, struct cl_entity_s *ent)
{
	if(!ent)
		return 1;

	if(ent->curstate.renderfx != 77)
		return 1;

	int cubeNum = ent->curstate.origin.x + ent->curstate.origin.y + ent->curstate.origin.z;
	cubeNum = abs(cubeNum);

	char cubemapName[256];
	sprintf(cubemapName,"cubemap_%i",cubeNum);

	LoadCubeMap(ent->origin,cubemapName);
	return 0;
}

extern vec3_t	render_origin;
void SetupCubemap(int cubemap)
{
		float m1[16],m2[16];
		glGetFloatv ( GL_MODELVIEW_MATRIX, m1 );
		transpose   ( m1, m2 );

		glDisable(GL_TEXTURE_2D);
		glEnable  ( GL_TEXTURE_CUBE_MAP_ARB );
		glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, cubicmaps[cubemap].cubeid);

		glTexGeni ( GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_ARB );
		glTexGeni ( GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_ARB );
		glTexGeni ( GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_ARB );
		glEnable  ( GL_TEXTURE_GEN_S );
		glEnable  ( GL_TEXTURE_GEN_T );
		glEnable  ( GL_TEXTURE_GEN_R );

		glMatrixMode ( GL_TEXTURE );
		glPushMatrix();
		glLoadMatrixf ( m2 );
		/*
		int shaderId = fp_cubemap;
		float blurFactor = 0.03f;
		blurFactor *= (32.0f/(float)cubicmaps[cubemap].cubewidth);

		if(blurFactor <= 0.00375 || cv_cubemap->value<2)
			shaderId = fp_cubemap_noblur;

		if(blurFactor > 0.06f)
			blurFactor = 0.06f;
		*/

}

void SetupCubemapWater(int cubemap,int fragment,int frag_noblur)
{
		float m1[16],m2[16];
		glGetFloatv ( GL_MODELVIEW_MATRIX, m1 );
		transpose   ( m1, m2 );

		glDisable(GL_TEXTURE_2D);
		glEnable  ( GL_TEXTURE_CUBE_MAP_ARB );
		glBindTexture ( GL_TEXTURE_CUBE_MAP_ARB, cubicmaps[cubemap].cubeid );

		glTexGeni ( GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_ARB );
		glTexGeni ( GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_ARB );
		glTexGeni ( GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_ARB );
		glEnable  ( GL_TEXTURE_GEN_S );
		glEnable  ( GL_TEXTURE_GEN_T );
		glEnable  ( GL_TEXTURE_GEN_R );

		glMatrixMode ( GL_TEXTURE );
		glPushMatrix();
		glLoadMatrixf ( m2 );
		
		int shaderId = fragment;
		float blurFactor = 0.03f;
		blurFactor *= (32.0f/(float)cubicmaps[cubemap].cubewidth);

		if(blurFactor <= 0.00375 || cv_cubemap->value<2)
			shaderId = frag_noblur;

		if(blurFactor > 0.06f)
			blurFactor = 0.06f;

		gl.glEnable(GL_FRAGMENT_PROGRAM_ARB);
		gl.glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB,shaderId);
		vec3_t camera = render_origin;

		glProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,0,
			camera.x,camera.y,camera.z,blurFactor);

		gl.glEnable(GL_VERTEX_PROGRAM_ARB);
		gl.glBindProgramARB(GL_VERTEX_PROGRAM_ARB,vp_cubemap);
}

void DisableCubemap(int studio)
{
	glMatrixMode(GL_TEXTURE);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glDisable ( GL_TEXTURE_GEN_S );
	glDisable ( GL_TEXTURE_GEN_T );
	glDisable ( GL_TEXTURE_GEN_R );

	glBindTexture ( GL_TEXTURE_CUBE_MAP_ARB, 0);
	glDisable ( GL_TEXTURE_CUBE_MAP_ARB );
	glEnable(GL_TEXTURE_2D);

	if (!studio)
	{
		gl.glDisable(GL_FRAGMENT_PROGRAM_ARB);
		gl.glDisable(GL_VERTEX_PROGRAM_ARB);
	}
}
extern int cubemaprendering;
extern cvar_t *buildcubemap;
void DrawCubemap(CubeMap *cubemap);
void DrawCubemaps()
{
	if(buildcubemap->value > 0)
	{
		gEngfuncs.Con_Printf("cubemap count = %i\n",cubemap_count);
		cubemaprendering = 1;

		for(int i=1;i<cubemap_count;i++)
		{
			
			DrawCubemap(&cubicmaps[i]);
		}
		gEngfuncs.Cvar_SetValue("buildcubemap",0);
		cubemaprendering = 0;
		buildcubemap->value = 0.0;
	}
}
