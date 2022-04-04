#ifndef __TEXLOADER_H
#define __TEXLOADER_H

#define MAX_EXT_TEXTURE_NAME 128
#include "../common/com_model.h"
#include "glmanager.h"
void initTextures();

#pragma pack(push,1)
typedef struct
{

	byte  identsize;          // size of ID field that follows 18 byte header (0 usually)
	byte  colourmaptype;      // type of colour map 0=none, 1=has palette
	byte  imagetype;          // type of image 0=none,1=indexed,2=rgb,3=grey,+8=rle packed

	short colourmapstart;     // first colour map entry in palette
	short colourmaplength;    // number of colours in palette
	byte  colourmapbits;      // number of bits per palette entry 15,16,24,32

	short xstart;             // image x origin
	short ystart;             // image y origin
	short width;              // image width in pixels
	short height;             // image height in pixels
	byte  bits;               // image bits per pixel 8,16,24,32
	byte  descriptor;         // image descriptor bits (vh flip bits)
} TGA_HEADER;

typedef struct cubetex
{
	GLuint	textures[6];
	unsigned int	cubeid;
	int cubewidth;
	char texName[64];
	vec3_t position;
} CubeMap;
#pragma pack(pop)

class ExtTextureData
{
public:
	ExtTextureData(char *_name);
	~ExtTextureData();

	char	name[MAX_EXT_TEXTURE_NAME];
	int		gl_normalmap_id;
	int		gl_detailtex_id;
	int		gl_glossmap_id;
	int		gl_extra_glossmap_id;

	int		detail_xscale, detail_yscale;

	ExtTextureData *pnext;
};


class ExtStudioTextureData
{
public:
	ExtStudioTextureData();
	~ExtStudioTextureData();

	char	name[MAX_EXT_TEXTURE_NAME];
	int		gl_textureindex;
	int		gl_glossmap_id;
	int		gl_original;
	ExtStudioTextureData *pnext;
};


enum {
	MIPS_NO		= 0,	// dont generate mipmaps
	MIPS_YES	= 1,	// generate mipmaps
	MIPS_NORMALIZED = 2, // generate and renormalize mipmaps (for normal maps)
	MIPS_LIGHT = 3,
};

int CreateTexture		(const char* filename, int mipmaps, int useid = 0, int *width = 0,int *height = 0,vec3_t *color = 0);
//int Create_DSDT_Texture	(const char* filename);
int LoadCacheTexture	(const char* filename, int mipmaps, int clamped);
ExtTextureData* GetExtTexData (texture_t *tex);


void CreateExtDataForTextures();
void DeleteExtTextures();
int GenerateShadowMap();
int GenerateEmptyRGB(int w,int h);
void LoadHDTextures();

extern int current_ext_texture_id;
void CreateEmptyTex(int width, int height, unsigned int &Tex, int iType, int iFormat, bool clamped);
void CreateEmptyDepth(int width, int height, unsigned int &Tex, bool clamped);

ExtStudioTextureData* GetExtStudioTexData(int original);
ExtStudioTextureData* AddExtStudioData(char *name, int original,char *shortname = NULL);

//cubemap
int GetCubemapIdByPoint(vec3_t point);
int LoadCubeMap(vec3_t position, char *texName);
void DisableCubemap(int studio = 0);
void SetupCubemap(int cubemaid);
void SetupCubemapWater(int cubemaid, int fragment, int frag_noblur);
cl_texture_t *LoadTextureWide(const char *fileName, int mips = MIPS_YES, int clamped = true);

#endif