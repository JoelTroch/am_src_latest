//
// written by BUzer for HL: Paranoia modification
//
//		2006

#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "com_model.h"
#include "gl_renderer.h"
#include "gl_texloader.h"
#include <map>
#include "dds.h"

#define BASE_EXT_TEXTURE_ID		(1<<25) // dont use zero
int current_ext_texture_id = BASE_EXT_TEXTURE_ID;
std::map <std::string, Vector> texColors;

int loadDDS(const char * filename, int useid, int *w, int *h, vec3_t *color)
{
	DDSURFACEDESC2	ddsd;
	char				filecode[4];
	int				factor;
	int				bufferSize;
	int				format;

	int len = 0;
	byte *buf = AM_LoadFile(filename, 5, &len);
	byte *fbuf = buf;

	if (!buf || !len)
	{
	//	gEngfuncs.Con_DPrintf("unable to load dds %s\n", filename);
		return 0;
	}
	
	// Verify the file is a true .dds file
	memcpy(filecode, &buf[0], 4);
	buf += 4;

	if (strncmp(filecode, "DDS ", 4) != 0)
	{
		gEngfuncs.Con_DPrintf("\"%s\" is not a valid .dds file !", filename);
		return 0;
	}

	// Get the surface descriptor
	memcpy(&ddsd, &buf[0], sizeof(ddsd));
	buf += sizeof(ddsd);

	//
	// Type de compression
	switch (ddsd.ddpfPixelFormat.dwFourCC)
	{
	case FOURCC_DXT1:
		// DXT1's compression ratio is 8:1
		format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
		factor = 2;
		break;

	case FOURCC_DXT3:
		// DXT3's compression ratio is 4:1
		format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		factor = 4;
		break;

	case FOURCC_DXT5:
		// DXT5's compression ratio is 4:1
		format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		factor = 4;
		break;

	default:
		gEngfuncs.Con_DPrintf("The file \"%s\" doesn't appear to be compressed using DXT1, DXT3, or DXT5!", filename);
		AM_FreeFile(fbuf);
		return 0;
		break;
	}

/*	if (!ddsd.dwLinearSize)
	{
		gEngfuncs.Con_DPrintf("ddsd.dwLinearSize is 0 !");
		AM_FreeFile(fbuf);
		return 0;
	}*/
	if (ddsd.dwMipMapCount > 1)	bufferSize = ddsd.dwLinearSize * factor;
	else									bufferSize = ddsd.dwLinearSize;

	// Sauvegarde les infos
	int Width = ddsd.dwWidth;
	int Height = ddsd.dwHeight;

	//
	// Generation de la texture
	int GLid = 0;
	
	if (!useid)
	{
		GLid = current_ext_texture_id;
		current_ext_texture_id++;
	}
	else
		GLid = useid;

	glBindTexture(GL_TEXTURE_2D, GLid);

	if (ddsd.dwMipMapCount > 1)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	// Chargement des mipmaps
	int nSize = 0;
	int nOffset = 0;
	int nHeight = Height;
	int nWidth = Width;
	int nNumMipMaps = ddsd.dwMipMapCount;
	int nBlockSize = (format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16;

	if (w)
		*w = (int)nWidth;
	if (h)
		*h = (int)nHeight;

	for (unsigned long i = 0; i < ddsd.dwMipMapCount; ++i)
	{
		if (nWidth == 0) nWidth = 1;
		if (nHeight == 0) nHeight = 1;

		//max(1, ( (width + 3) / 4 ) ) x max(1, ( (height + 3) / 4 ) ) x 8(DXT1) or 16(DXT2-5)
		nSize = max(1, ((nWidth + 3) / 4)) * max(1, ((nHeight + 3) / 4)) * nBlockSize;

		gl.glCompressedTexImage2DARB(GL_TEXTURE_2D,i,format,nWidth,nHeight,0,nSize,&buf[0] + nOffset);

		nOffset += nSize;

		// Moitie de la taille pour le prochain mipmap...
		nWidth /= 2;
		nHeight /= 2;
	}

	//gEngfuncs.Con_DPrintf("success load %s w:%i h:%i\n", filename,Width,Height);
	AM_FreeFile(fbuf);
	return GLid;
}

int loadTga(const char* filename, int mipmaps, int useid, int *w, int *h, vec3_t *color)
{
	int len;
	byte*buff = AM_LoadFile((char*)filename, 5, &len);

	int width, height;
	TGA_HEADER hdr;

	if (!buff)
		return 0;

	memcpy(&hdr, &buff[0], 18);

	width = hdr.width;
	height = hdr.height;

	int type = GL_RGB;
	int bpp;

	switch (hdr.bits)
	{
	case 24:
		bpp = 3;
		break;
	case 32:
		type = GL_RGBA;
		bpp = 4;
		break;
	default:
		AM_FreeFile(buff);
		gEngfuncs.Con_Printf("loadTga - not 32/24 bpp texture!");
		return -1;
	}

	int imgsize = width*height*bpp + 18;
	byte temp;

	if (!(hdr.descriptor & 0x20))
	{
		unsigned char *temp = (unsigned char *)malloc(hdr.width * bpp);

		for (int i = 0; i < hdr.height / 2; i++)
		{
			memcpy(temp, &buff[18] + i*hdr.width*bpp, hdr.width*bpp);
			memcpy(&buff[18] + i*hdr.width*bpp, &buff[18] + (hdr.height - i - 1)*hdr.width*bpp, hdr.width*bpp);
			memcpy(&buff[18] + (hdr.height - i - 1)*hdr.width*bpp, temp, hdr.width*bpp);
		}
		free(temp);
	}


	for (GLuint i = 18; i<int(imgsize); i += bpp)
	{
		temp = buff[i];
		buff[i] = buff[i + 2];
		buff[i + 2] = temp;

		if (color)
		{
			color->x += (float)buff[i];
			color->y += (float)buff[i + 1];
			color->z += (float)buff[i + 2];
		}
	}

	if (color)
	{
		color->x /= (float)(width*height);
		color->y /= (float)(width*height);
		color->z /= (float)(width*height);
		color->x /= 255.0f;
		color->y /= 255.0f;
		color->z /= 255.0f;
	}

	if (!useid)
		glBindTexture(GL_TEXTURE_2D, current_ext_texture_id);
	else
		glBindTexture(GL_TEXTURE_2D, useid);

	if (mipmaps != MIPS_LIGHT)
	{
		if (mipmaps == MIPS_YES)
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		else
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glHint(GL_GENERATE_MIPMAP_HINT_SGIS, GL_NICEST);
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);

		if (mipmaps == MIPS_NO)
		{
			gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}
		else
		{
			gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		}

		glTexParameteri(GL_TEXTURE_2D, GL_CLAMP_TO_BORDER, GL_TRUE);
		glTexImage2D(GL_TEXTURE_2D, 0, type, width, height, 0, type, GL_UNSIGNED_BYTE, &buff[18]);
	}
	else
	{
		gl.glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		gl.glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, &buff[18]);
	}

	AM_FreeFile(buff);

	if (w != NULL)
		*w = width;
	if (h != NULL)
		*h = height;

	if (useid)
		return useid;

	current_ext_texture_id++;
	return (current_ext_texture_id - 1);
}

void initTextures()
{
	int len = 0;
	byte *file = gEngfuncs.COM_LoadFile("gfx/texturecolors.txt", 5, &len);
	if (!file || !len)
	{
		gEngfuncs.Con_Printf("Unable to load gfx/texturecolors.txt file\n");
		return;
	}

	int cnt = 0;
	char token[1024];
	char texName[64];
	float r, g, b;
	r = g = b = 0;
	memset(texName, 0, 64);
	int si = 0;
	for (int i = 0; i < len; i++)
	{
		if (file[i] == '\n')
		{
			sscanf(token, "%s %f %f %f", &texName, &r, &g, &b);
			std::string str(texName);
			texColors[str] = Vector(r, g, b);
			Vector n = texColors[str];
			memset(token, 0, 1024);
			si = 0;
			cnt++;
			continue;
		}
		token[si++] = file[i];
	}
}


char *lowcase(char *string)
{
	char *temp;

	for (temp = string; *temp; temp++)
		*temp = tolower(*temp);

	return string;
}

void pathtofilename(char *in, char *out)
{
	int lastdot = 0;
	int lastbar = 0;
	int pathlength = 0;

	for (int i = 0; i < (int)strlen(in); i++)
	{
		if (in[i] == '/' || in[i] == '\\')
			lastbar = i + 1;

		if (in[i] == '.')
			lastdot = i;
	}

	for (int i = lastbar; i < (int)strlen(in); i++)
	{
		if (i == lastdot)
			break;

		out[pathlength] = in[i];
		pathlength++;
	}
	out[pathlength] = 0;
}


class DetailTexture
{
public:
	DetailTexture(char *_name, int _id);
	~DetailTexture();
	
	char	name[32];
	int		gl_id;
	DetailTexture *pnext;
};


DetailTexture *p_detailTexturesList = NULL;
ExtTextureData *p_extTexDataList = NULL;
ExtStudioTextureData *p_extStudioTexDataList = NULL;

// DetailTexture
DetailTexture::DetailTexture(char *_name, int _id) : gl_id(_id)
{
	strcpy(name, _name);
	pnext = p_detailTexturesList;
	p_detailTexturesList = this;
}

DetailTexture::~DetailTexture()
{
	if (pnext) delete pnext;
}

// ExtTextureData
ExtTextureData::ExtTextureData(char *_name)
{
	strcpy(name, _name);
	pnext = p_extTexDataList;
	p_extTexDataList = this;
}

ExtTextureData::~ExtTextureData()
{
	if (pnext) delete pnext;
}



void SortTexturesByDetailTexID();


// ===========================
// Mip maps creation
//
// ===========================

// test
int WriteTGA_24( color24 *pixels, unsigned short width, unsigned short height, const char *filename);

void GenerateMipMap (color24 *in, int width, int height)
{
	color24	*out = in;
	height >>= 1;
	for (int y = 0; y < height; y++, in+=width*2)
	{
		for (int x = 0; x < width/2; x++, out++)
		{
			out->r = (in[x*2].r + in[x*2+1].r + in[width+x*2].r + in[width+x*2+1].r) >> 2;
			out->g = (in[x*2].g + in[x*2+1].g + in[width+x*2].g + in[width+x*2+1].g) >> 2;
			out->b = (in[x*2].b + in[x*2+1].b + in[width+x*2].b + in[width+x*2+1].b) >> 2;
		//	out->r = (in[x*2].r/4 + in[x*2+1].r/4 + in[width+x*2].r/4 + in[width+x*2+1].r/4);
		//	out->g = (in[x*2].g/4 + in[x*2+1].g/4 + in[width+x*2].g/4 + in[width+x*2+1].g/4);
		//	out->b = (in[x*2].b/4 + in[x*2+1].b/4 + in[width+x*2].b/4 + in[width+x*2+1].b/4);
		}
	}
}


void NormalizeMipMap (color24 *buf, int width, int height)
{
	color24 *vec = buf;
	int numpixels = width * height;
	for (int i = 0; i < numpixels; i++)
	{
	/*	vec[i].r = 127;
		vec[i].g = 127;
		vec[i].b = 255;*/
		vec3_t flvec;
		flvec[0] = ((float)vec[i].r / 127.0) - 1;
		flvec[1] = ((float)vec[i].g / 127.0) - 1;
		flvec[2] = ((float)vec[i].b / 127.0) - 1;

		VectorNormalize(flvec);

		vec[i].r = (unsigned char)((flvec[0]+1)*127);
		vec[i].g = (unsigned char)((flvec[1]+1)*127);
		vec[i].b = (unsigned char)((flvec[2]+1)*127);
	}
}


//===================================
// CreateTexture
//
// loads 24bit TGAs
//	returns gl texture id, or 0 in error case
// TODO: compressed images support
//===================================
typedef struct tgaheader_s
{
	unsigned char	IdLength;
	unsigned char	ColorMap;
	unsigned char	DataType;
	unsigned char	unused[5];
	unsigned short	OriginX;
	unsigned short	OriginY;
	unsigned short	Width;
	unsigned short	Height;
	unsigned char	BPP;
	unsigned char	Description;
} TGAheader;

#define TEXBUFFER_SIZE	(512*512)

char dds[5] = ".dds";
int CreateTexture(const char* filename, int mipmaps, int useid,int *w,int *h,vec3_t *color)
{
	if (!filename)
		return 0;

	int ret = loadTga(filename, mipmaps, useid, w, h, color);
	if (ret <= 0)
	{
		char name[256];
		strcpy(name, filename);
		int slen = strlen(name);
		for (int i = slen - 4; i < slen; i++)
		{
			name[i] = dds[i - (slen - 4)];
		}
		ret = loadDDS(name, useid, w, h, color);
	}

	return ret;
}

int GenerateShadowMap()
{
	if (!gl.IsGLAllowed())
	{
		gEngfuncs.Con_Printf("GenerateShadowMap error: glmanager is not initialized!\n");
		return 0;
	}

	gl.glBindTexture(GL_TEXTURE_2D, current_ext_texture_id);
	gl.glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, gl_shadowsize->value, gl_shadowsize->value, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
	gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_COMPARE_R_TO_TEXTURE);
	gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC_ARB, GL_LEQUAL);
	gl.glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE_ARB, GL_INTENSITY);

	current_ext_texture_id++;
	return (current_ext_texture_id - 1);
}

int GenerateEmptyRGB(int w,int h)
{
	if (!gl.IsGLAllowed())
	{
		gEngfuncs.Con_Printf("GenerateShadowMap error: glmanager is not initialized!\n");
		return 0;
	}

	gl.glBindTexture(GL_TEXTURE_2D, current_ext_texture_id);
	gl.glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	current_ext_texture_id++;
	return (current_ext_texture_id - 1);
}

//===================================
// Create_DSDT_Texture
//
// loads 24bit TGAs, and converts it to DSDT NV format
//	returns gl texture id, or 0 in error case
//===================================
typedef struct
{
	byte ds, dt;
} colorDSDT;

int Create_DSDT_Texture(const char* filename)
{
	colorDSDT buf[TEXBUFFER_SIZE];

	if (!gl.IsGLAllowed())
	{
		gEngfuncs.Con_Printf("Create_DSDT_Texture error: glmanager is not initialized!\n");
		return 0;
	}

	int length = 0;
	char *file = (char *)gEngfuncs.COM_LoadFile( (char*)filename, 5, &length );
	if (!file)
	{
		gEngfuncs.Con_DPrintf("Create_DSDT_Texture failed to load file: %s\n", filename);
		return 0;
	}

	TGAheader *hdr = (TGAheader *)file;
	if (!(/*hdr->DataType == 10 ||*/ hdr->DataType == 2) || (hdr->BPP != 24))
	{
		gEngfuncs.Con_Printf("Error: texture %s uses unsupported data format -->\n(only 24-bit noncompressed TGA supported)\n", filename);
		gEngfuncs.COM_FreeFile(file);
		return 0;
	}

	int maxtexsize;
	gl.glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxtexsize);
	if (hdr->Width > maxtexsize || hdr->Height > maxtexsize)
	{
		gEngfuncs.Con_Printf("Error: texture %s: sizes %d x %d are not supported by your hardware\n", filename, hdr->Width, hdr->Height);
		gEngfuncs.COM_FreeFile(file);
		return 0;
	}

	 // lame :)
	if (!(hdr->Width == 16 || hdr->Width == 32 || hdr->Width == 64 || hdr->Width == 128 || hdr->Width == 256 || hdr->Width == 512 || hdr->Width == 1024) ||
		!(hdr->Height == 16 || hdr->Height == 32 || hdr->Height == 64 || hdr->Height == 128 || hdr->Height == 256 || hdr->Height == 512 || hdr->Height == 1024))
	{
		gEngfuncs.Con_Printf("Error: texture %s has bad dimensions (%d x %d)\n", filename, hdr->Width, hdr->Height);
		gEngfuncs.COM_FreeFile(file);
		return 0;
	}

	int numpixels = hdr->Width * hdr->Height;
	if (numpixels > TEXBUFFER_SIZE)
	{
		gEngfuncs.Con_Printf("Error: texture %s (%d x %d) doesn't fit in static buffer\n", filename, hdr->Width, hdr->Height);
		gEngfuncs.COM_FreeFile(file);
		return 0;
	}

	color24 *pixels = (color24*)(file + sizeof(TGAheader) + hdr->IdLength);
	colorDSDT *dest = buf;
	
	// non-compressed image
	for (int i = 0; i < numpixels; i++, pixels++, dest++)
	{
		dest->dt = pixels->g;
		dest->ds = pixels->r;
	}

	// flip image
	if (!(hdr->Description & 0x20))
	{
		colorDSDT line[1024];
		for (int i = 0; i < hdr->Height / 2; i++)
		{
			int linelen = hdr->Width * sizeof(colorDSDT);
			memcpy(line, &buf[i*hdr->Width], linelen);
			memcpy(&buf[i*hdr->Width], &buf[(hdr->Height - i - 1)*hdr->Width], linelen);
			memcpy(&buf[(hdr->Height - i - 1)*hdr->Width], line, linelen);
		}
	}
	
	// upload image
	gl.glBindTexture(GL_TEXTURE_2D, current_ext_texture_id);
	gl.glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	gl.glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
	gl.glTexImage2D (GL_TEXTURE_2D, 0, GL_DSDT8_NV, hdr->Width, hdr->Height, 0, GL_DSDT_NV, GL_UNSIGNED_BYTE, buf);

	gEngfuncs.COM_FreeFile(file);

	gEngfuncs.Con_DPrintf("Loaded DSDT texture %s\n", filename);

	current_ext_texture_id++;
	return (current_ext_texture_id - 1);
}



// ===========================
// Detail textures uploading
//		(dont call from VidInit - we dont know the mapname at that point)
// ===========================

// parsed detail textures file data
#define MAX_DETAIL_FILE_ENTRIES		(512)

typedef struct detailtexFileEntry_s
{
	char	texname[16];
	char	detailtexname[32];
	int		xscale;
	int		yscale;
} detailtexFileEntry;

detailtexFileEntry parsedData[MAX_DETAIL_FILE_ENTRIES];
int numentries = 0;


int ParseDetailTextureDataFile()
{
	char levelname[256];
	numentries = 0;
	strcpy( levelname, gEngfuncs.pfnGetLevelName() );
	gEngfuncs.Con_DPrintf("\n>> loading detail textures\n");

	if ( strlen(levelname) == 0 )
	{
		gEngfuncs.Con_Printf("ERROR: ParseDetailTextureDataFile() cant get level name!\n");
		return FALSE;
	}

	levelname[strlen(levelname)-4] = 0;
	strcat(levelname, "_detail.txt");

	char *pfile = (char *)gEngfuncs.COM_LoadFile( levelname, 5, NULL);
	if (!pfile)
	{
		gEngfuncs.Con_Printf("No detail textures file %s\n", levelname);
		return FALSE;
	}

	numentries = 0;
	char *ptext = pfile;
	while(1)
	{
		char texture[256];
		char detailtexture[256];
		char sz_xscale[64];
		char sz_yscale[64];

		if (numentries >= MAX_DETAIL_FILE_ENTRIES)
		{
			gEngfuncs.Con_Printf("Too many entries in detail textures file %s\n", levelname);
			break;
		}

		ptext = gEngfuncs.COM_ParseFile(ptext, texture);
		if (!ptext) break;
		ptext = gEngfuncs.COM_ParseFile(ptext, detailtexture);
		if (!ptext) break;
		ptext = gEngfuncs.COM_ParseFile(ptext, sz_xscale);
		if (!ptext) break;
		ptext = gEngfuncs.COM_ParseFile(ptext, sz_yscale);
		if (!ptext) break;

		int i_xscale = atoi(sz_xscale);
		int i_yscale = atoi(sz_yscale);

		if (strlen(texture) > 16 || strlen(detailtexture) > 32)
		{
			gEngfuncs.Con_Printf("Error in detail textures file %s: token too large\n", levelname);
			gEngfuncs.Con_Printf("(entry %d: %s - %s)\n", numentries, texture, detailtexture);
			continue;
		}

	//	gEngfuncs.Con_Printf("%d: %s - %s (%s x %s)\n", numentries, texture, detailtexture, sz_xscale, sz_yscale);

		strcpy( parsedData[numentries].texname, texture );
		strcpy( parsedData[numentries].detailtexname, detailtexture );
		parsedData[numentries].xscale = i_xscale;
		parsedData[numentries].yscale = i_yscale;
		numentries++;
	}
	gEngfuncs.COM_FreeFile( pfile );
	return TRUE;
}


int GetDetailTextureData(char *_texname, int &glid, int &x_scale, int &y_scale)
{
	detailtexFileEntry *pentry = NULL;
	glid = 0;

	// get detail texture data from file
	for (int i = 0; i < numentries; i++)
	{
		if (!strcmp(parsedData[i].texname, _texname))
		{
			pentry = &parsedData[i];
			break;
		}
	}

	if (!pentry)
		return FALSE;

	x_scale = pentry->xscale;
	y_scale = pentry->yscale;

	// try to find detail texture in list
	DetailTexture *plist = p_detailTexturesList;
	while(plist)
	{
		if (!strcmp(plist->name, pentry->detailtexname))
		{
			glid = plist->gl_id;
			return TRUE;
		}
		plist = plist->pnext;
	}

	// load texture
	char path[256];
	sprintf(path, "gfx/%s.tga", pentry->detailtexname);

	glid = CreateTexture(path, MIPS_YES);
	if (!glid)
	{
		gEngfuncs.Con_Printf("Detail texture %s not found!\n", path);
		return FALSE;
	}

	new DetailTexture(pentry->detailtexname, glid);

	gEngfuncs.Con_DPrintf("Loaded detail texture %s\n", pentry->detailtexname);
	return TRUE;
}


// ===========================
// CreateExtDataForTextures()
//
// Loads all extra textures for base texture -
//		detail textures, normal maps, gloss maps
//
// Only non-animated textures support.
// pointer for extra data saved into anim_next texture_t pointer
// ===========================

ExtTextureData *FindTextureExtraData(char *texname)
{
	ExtTextureData *plist = p_extTexDataList;
	while(plist)
	{
		if (!strcmp(plist->name, texname))
			return plist;

		plist = plist->pnext;
	}
	return NULL; // nothing found
}


void CreateExtDataForTextures( )
{
	ParseDetailTextureDataFile();

	model_t* world = gEngfuncs.GetEntityByIndex(0)->model;
	if (!world) return;

	gEngfuncs.Con_DPrintf("\n>> Creating ext data for textures\n");

	texture_t** tex = (texture_t**)world->textures;
	for (int i = 0; i < world->numtextures; i++)
	{
		if (!tex[i])
			continue;

		if (tex[i] && tex[i]->anim_total || tex[i]->name[0] == 0 ||
			tex[i]->name[0] == '+' || tex[i]->name[0] == '-')
			continue;
		
		// get detail texture for it
		int det_texid = 0;
		int det_xscale = 0, det_yscale = 0;
		GetDetailTextureData(tex[i]->name, det_texid, det_xscale, det_yscale);

		ExtTextureData *pExtData;

		// data already allocated?
		pExtData = FindTextureExtraData(tex[i]->name);

		if (!pExtData)
		{
			// create new
			pExtData = new ExtTextureData(tex[i]->name);

			//gEngfuncs.Con_DPrintf("Loaded texture data for %s\n", tex[i]->name);
			// load other special textures here

			// try to load normal map
			char filename[256];
			sprintf(filename, "gfx/bumpmaps/%s_norm.tga", tex[i]->name);
			pExtData->gl_normalmap_id = CreateTexture(filename, MIPS_YES);

			// try to load gloss map
			sprintf(filename, "gfx/bumpmaps/%s_gloss.tga", tex[i]->name);
			pExtData->gl_glossmap_id = CreateTexture(filename, MIPS_YES);

			// try to load gloss map for high quality specular
			if (pExtData->gl_normalmap_id)
			{
				sprintf(filename, "gfx/bumpmaps/%s_glosshi.tga", tex[i]->name);
				pExtData->gl_extra_glossmap_id = CreateTexture(filename, MIPS_YES);
			}
			else
				pExtData->gl_extra_glossmap_id = 0;
		}

		// store detail texture info (this may change between maps)
		pExtData->gl_detailtex_id = det_texid;
		pExtData->detail_xscale = det_xscale;
		pExtData->detail_yscale = det_yscale;

		tex[i]->anim_next = (struct texture_s *)pExtData; // HACK
		tex[i]->anim_min = -666;				
	}
	gEngfuncs.Con_DPrintf("Finished creating ext data for textures\n");

	SortTexturesByDetailTexID();
}


// ===========================
// Sorting textures pointers in world->textures array by detail texture id.
// This may allow us make less state switches during rendering
// ===========================

int CompareDetailIDs( texture_t *a, texture_t *b )
{
	int detail_id_a = 0;
	int detail_id_b = 0;
	ExtTextureData *pExtData;

	if (!a || !b)
		return 0;

	if (a->anim_min == -666)
	{
		pExtData = (ExtTextureData*)a->anim_next;
		if (pExtData)
			detail_id_a = pExtData->gl_detailtex_id;
	}

	if (b->anim_min == -666)
	{
		pExtData = (ExtTextureData*)b->anim_next;
		if (pExtData)
			detail_id_b = pExtData->gl_detailtex_id;
	}

	return (detail_id_a > detail_id_b);
}


void SortTexturesByDetailTexID()
{
	model_t* world = gEngfuncs.GetEntityByIndex(0)->model;
	if (!world) return;

	gEngfuncs.Con_DPrintf("Sorting textures by detail texture id...");

	// use simple insert sort
	texture_t** tex = (texture_t**)world->textures;
	texture_t *t;
    int i, j;
    for (i = 0 + 1; i < world->numtextures; i++)
	{
		if (!tex[i]) continue;
        t = tex[i];

        /* Сдвигаем элементы вниз, пока */
        /*  не найдем место вставки.    */
        for (j = i-1; j >= 0 && CompareDetailIDs(tex[j], t); j--)
            tex[j+1] = tex[j];

        /* вставка */
        tex[j+1] = t;
    }

	gEngfuncs.Con_DPrintf("ok\n");
}


void DeleteExtTextures()
{
	if (p_detailTexturesList) delete p_detailTexturesList;
	if (p_extTexDataList) delete p_extTexDataList;
}



//=====================================
// Cached textures
//
// damn, i should make thi a long time ago,
// and finally the time has come...
//=====================================
#define MAX_CACHED_TEXTURES 2048

struct cache_tex {
	char name[64];
	cl_texture_t *tex;
};

cache_tex cached_textures[MAX_CACHED_TEXTURES];
int num_cached_textures;

cl_texture_t *LoadTextureWide(const char *fileName, int mips,int clamped)
{
	int i;
	for (i = 0; i < num_cached_textures; i++)
	{
		if (!strcmp(cached_textures[i].name, fileName))
			return cached_textures[i].tex;
	}

	if (i == MAX_CACHED_TEXTURES)
	{
		ONCE(gEngfuncs.Con_Printf("Error: LoadTextureWide failed: MAX_CACHED_TEXTURES exceeded!\n"););
		return 0;
	}

	int w, h;
	w = h = 0;

	cached_textures[i].tex = (cl_texture_t*)malloc(sizeof(cl_texture_s));
	memset(cached_textures[i].tex, 0, sizeof(cl_texture_s));
	
	cached_textures[i].tex->iIndex = CreateTexture(fileName, mips, 0, &w, &h);

	if (!cached_textures[i].tex->iIndex)
	{
		free(cached_textures[i].tex);
		cached_textures[i].tex = NULL;
		gEngfuncs.Con_Printf("LoadTextureWide: unable to open: %s\n", fileName);
		return 0;
	}
	cached_textures[i].tex->iHeight = h;
	cached_textures[i].tex->iWidth = w;

	strcpy(cached_textures[i].name, fileName);
	num_cached_textures++;
	if (clamped)
	{
		gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	}
	return cached_textures[i].tex;
}

int LoadCacheTexture(const char* filename, int mipmaps, int clamped)
{
	int i;
	for (i = 0; i < num_cached_textures; i++)
	{
		if (!strcmp(cached_textures[i].name, filename))
			return cached_textures[i].tex->iIndex;
	}

	if (i == MAX_CACHED_TEXTURES)
	{
		ONCE( gEngfuncs.Con_Printf("Error: LoadCacheTexture failed: MAX_CACHED_TEXTURES exceeded!\n"); );
		return 0;
	}

	int w, h;
	w = h = 0;

	cached_textures[i].tex = (cl_texture_t*)malloc(sizeof(cl_texture_s));
	cached_textures[i].tex->iIndex = CreateTexture(filename, mipmaps,0,&w,&h);

	if (!cached_textures[i].tex->iIndex)
	{
		free(cached_textures[i].tex);
		return 0;
	}
	cached_textures[i].tex->iHeight = h;
	cached_textures[i].tex->iWidth = w;

	strcpy(cached_textures[i].name, filename);
	num_cached_textures++;	
	if(clamped)
	{
		gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
		gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
	}
	return cached_textures[i].tex->iIndex;
}

void LoadHDTextures()
{
	cl_entity_t *worldEnt = gEngfuncs.GetEntityByIndex(0);
	if (!worldEnt || !worldEnt->model) return;
	model_t *wmod = worldEnt->model;
	for (int i = 0; i < wmod->numtextures; i++)
	{
		if (!wmod->textures[i])continue;
		char texname[256];
		sprintf(texname,"gfx/hdtextures/%s.tga",wmod->textures[i]->name);
		
		vec3_t color = Vector(0,0,0);
		int ret = CreateTexture(texname, MIPS_YES, wmod->textures[i]->gl_texturenum,0,0,&color);
		if (ret)
		{
			texColors[wmod->textures[i]->name] = color;
			//gEngfuncs.Con_Printf("Texture %s color = (%f %f %f)\n", wmod->textures[i]->name, color.x, color.y, color.z);
		}
	}

	extern unsigned int m_iScreen;
	CreateEmptyTex(ScreenWidth, ScreenHeight, m_iScreen, GL_TEXTURE_2D, GL_RGB, true);
}


// ExtTextureData
ExtStudioTextureData::ExtStudioTextureData()
{
	pnext = p_extStudioTexDataList;
	p_extStudioTexDataList = this;
}

ExtStudioTextureData::~ExtStudioTextureData()
{
	if (pnext) delete pnext;
}

void DeleteAllStudioData()
{
	ExtStudioTextureData *plist = p_extStudioTexDataList;
	while (plist)
	{
		plist->gl_original = 0;
		plist->gl_textureindex = 0;
		plist->gl_glossmap_id = 0;
		plist = plist->pnext;
	}
}

ExtStudioTextureData *FindStudioTextureExtraData(int original)
{
	ExtStudioTextureData *plist = p_extStudioTexDataList;
	while (plist)
	{
		if (plist->gl_original == original)
			return plist;

		plist = plist->pnext;
	}
	return NULL; // nothing found
}

void filenamewoextention(char *in, char *out)
{
	int lastdot = 0;
	int lastbar = 0;
	int pathlength = 0;

	for (int i = 0; i < (int)strlen(in); i++)
	{
		if (in[i] == '.')
			lastdot = i;
	}

	for (int i = lastbar; i < (int)strlen(in); i++)
	{
		if (i == lastdot)
			break;

		out[pathlength] = in[i];
		pathlength++;
	}
	out[pathlength] = 0;
}


ExtStudioTextureData *AddExtStudioData(char *name, int original,char *texnameshort )
{
	ExtStudioTextureData *pTex = FindStudioTextureExtraData(original);

	if (pTex)
		return pTex;
	else
		pTex = new ExtStudioTextureData();

	pTex->gl_original = original;
	char texName[256];
	char tgaName[256];
	char shortcut[256];
	filenamewoextention(texnameshort, shortcut);
	filenamewoextention(name, texName);

	sprintf(tgaName, "gfx/studiotextures/%s.tga", texName);
	int texId = LoadCacheTexture(tgaName, MIPS_YES, true);
	if (!texId && texnameshort)
	{
		sprintf(tgaName, "gfx/studiotextures/common/%s.tga", shortcut);
		texId = LoadCacheTexture(tgaName, MIPS_YES, true);
	}

	sprintf(tgaName, "gfx/studionormal/%s.tga", texName);
	int glosstexId = LoadCacheTexture(tgaName, MIPS_YES, true);
	if (!glosstexId && texnameshort)
	{
		filenamewoextention(texnameshort, texName);
		sprintf(tgaName, "gfx/studionormal/common/%s.tga", shortcut);
		glosstexId = LoadCacheTexture(tgaName, MIPS_YES, true);
	}

	pTex->gl_textureindex = texId;
	pTex->gl_glossmap_id = glosstexId;
	return pTex;
}

void CreateEmptyTex(int width, int height, unsigned int &Tex, int iType, int iFormat, bool clamped)
{
	if (!gl.IsGLAllowed())
	{
		gEngfuncs.Con_Printf("CreateTextureEmpty error: glmanager is not initialized!\n");
		return;
	}

	gEngfuncs.Con_Printf("emptytex: %i\n", current_ext_texture_id);
	glBindTexture(iType, current_ext_texture_id);

	glTexParameterf(iType, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(iType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(iType, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(iType, 0, iFormat, width, height, 0, iFormat, GL_UNSIGNED_BYTE, 0);

	if (clamped)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	Tex = current_ext_texture_id;
	current_ext_texture_id++;
}

void CreateEmptyDepth(int width, int height, unsigned int &Tex, bool clamped)
{
	if (!gl.IsGLAllowed())
	{
		gEngfuncs.Con_Printf("CreateEmptyDepth error: glmanager is not initialized!\n");
		return;
	}

	gEngfuncs.Con_Printf("emptytex: %i\n", current_ext_texture_id);
	glBindTexture(GL_TEXTURE_2D, current_ext_texture_id);

	glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE_ARB, GL_LUMINANCE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);

	if (clamped)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	Tex = current_ext_texture_id;
	current_ext_texture_id++;
}
vec3_t getTextureColor(char *name)
{
	if (!name)
		return Vector(0.5, 0.5, 0.5);

	char texName[64];
	sprintf(texName, "%s", name);
	lowcase(texName);
	std::string texname(texName);

	if (!strcmp(texName, "body"))
		return Vector(0.6, 0.0, 0.0);

	if (!texColors[texname])
		return Vector(0.5,0.5,0.5);
	return texColors[texname];
}