/*******************************************************
*
*  Lightmaps loading.
*
*  written by BUzer for Half-Life: Paranoia modification
*
********************************************************/

#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "cdll_int.h"
#include "com_model.h"
#include "glmanager.h"
#include "entity_types.h"
#include "gl_texloader.h"
#include "gl_renderer.h"

#define	BLOCK_WIDTH		128
#define	BLOCK_HEIGHT	128
#define	MAX_LIGHTMAPS	64
#define BLOCKLIGHTS_SIZE	(18*18)

// BUMP lightmaps base indexes
int g_lightmap_baselight = 0; // contains normal lightmap divided by two if no bump for surface
int g_lightmap_addlight = 0;
int g_lightmap_lightvecs = 0;

char szMapName[256]; // map for which we have lightmaps prepared
int lightmaps_initialized = 0;
int current_bumpstate = 0;
float current_gamma = 0;
float current_brightness = 0;
float current_contrast = 0;

color24	lightmaps[MAX_LIGHTMAPS*BLOCK_WIDTH*BLOCK_HEIGHT];
color24 blocklights[BLOCKLIGHTS_SIZE];

int lightmapstotal;

static model_t *world;

#define VectorMaximum(a) ( max( (a)[0], max( (a)[1], (a)[2] ) ) )

// test
int WriteTGA_24(color24 *pixels, unsigned short width, unsigned short height, const char *filename );

//
// HELPERS
//
int StyleIndex ( msurface_t *surf, int style )
{
	for (int j = 0 ; j < MAXLIGHTMAPS && surf->styles[j] != 255 ; j++)
	{
		if (surf->styles[j] == style)
			return j;
	}
	return -1;
}

float ApplyGamma( float value )
{
	if (current_gamma == 1) return (value * current_contrast + current_brightness);
	return (pow(value, current_gamma) * current_contrast + current_brightness);
}

/*
unsigned char Clamp( float value )
{
	value *= 255;
	if (value > 255) value = 255;
	if (value < 0) value = 0;
	return (unsigned char)value;
}
*/

void Clamp( vec3_t in, color24 *out )
{
	VectorScale(in, 255, in);
	float max = VectorMaximum(in);
	if (max > 255)
	{
		float scale = 255.0 / max;
		VectorScale(in, scale, in);
	}

	if (in[0] < 0) in[0] = 0;
	if (in[1] < 0) in[1] = 0;
	if (in[2] < 0) in[2] = 0;

	out->r = (unsigned char)in[0];
	out->g = (unsigned char)in[1];
	out->b = (unsigned char)in[2];
}

int LightmapSize( msurface_t *surf )
{
	int smax = (surf->extents[0]>>4)+1;
	int tmax = (surf->extents[1]>>4)+1;
	return smax*tmax;
}


//===================
// UploadBlocklights
// copies lightmap data from blocklights to common lightmap texture
//===================
void UploadBlocklights( msurface_t *surf )
{
	color24 *bl = blocklights;
	color24	*dest = lightmaps + surf->lightmaptexturenum*BLOCK_WIDTH*BLOCK_HEIGHT;
	dest += (surf->light_t * BLOCK_WIDTH + surf->light_s);	

	int smax = (surf->extents[0]>>4)+1;
	int tmax = (surf->extents[1]>>4)+1;

	for (int i=0 ; i<tmax ; i++, dest += BLOCK_WIDTH)
	{
		for (int j=0 ; j<smax ; j++)
		{
			dest[j].r = bl->r;
			dest[j].g = bl->g;
			dest[j].b = bl->b;
			bl++;
		}
	}
}


//===================
// BuildLightMap_FromOriginal
//
// Builds lightmap from original zero style
//===================
void BuildLightMap_FromOriginal (msurface_t *surf)
{
	int size = LightmapSize(surf);
	color24 *lightmap = surf->samples;

	if (!lightmap || size > BLOCKLIGHTS_SIZE)
		return;

	WriteTGA_24(lightmap, size, 1, "lightmap.tga");
	for (int i=0 ; i<size ; i++)
	{
		vec3_t col;
		col[0] = (float)lightmap[i].r / 255;
		col[1] = (float)lightmap[i].g / 255;
		col[2] = (float)lightmap[i].b / 255;

	//	blocklights[i].r = Clamp( ApplyGamma(col[0])/2 );
	//	blocklights[i].g = Clamp( ApplyGamma(col[1])/2 );
	//	blocklights[i].b = Clamp( ApplyGamma(col[2])/2 );
		col[0] = ApplyGamma(col[0])/2;
		col[1] = ApplyGamma(col[1])/2;
		col[2] = ApplyGamma(col[2])/2;
		Clamp(col, &blocklights[i]);
	}

	UploadBlocklights(surf);
}

//===================
// BuildLightMap_FromBaselight
//
// Builds lightmap from baselight - for faces that doesnt have
// direct light in bump mode, but have it in normal mode
//===================
void BuildLightMap_FromBaselight (msurface_t *surf)
{
	int size = LightmapSize(surf);
	if (!surf->samples || size > BLOCKLIGHTS_SIZE)
		return;

	color24 *lightmap = surf->samples + size * StyleIndex(surf, BUMP_BASELIGHT_STYLE);

	for (int i=0 ; i<size ; i++)
	{
		vec3_t col;
		col[0] = (float)lightmap[i].r / 255;
		col[1] = (float)lightmap[i].g / 255;
		col[2] = (float)lightmap[i].b / 255;

		col[0] = ApplyGamma(col[0]);
		col[1] = ApplyGamma(col[1]);
		col[2] = ApplyGamma(col[2]);
		Clamp(col, &blocklights[i]);
	}

	UploadBlocklights(surf);
}


//===================
// BuildLightMap_Combine
//
// builds lightmap data combining three bump styles
// Used for surfaces, who have bump styles but doesn't have normal map loaded
//
// (should give better results than just taking zero style, because
// bump lightmaps has increased light intensity range)
//===================
void BuildLightMap_Combine (msurface_t *surf)
{
	int size = LightmapSize(surf);
	color24 *lightmap = surf->samples;

	if (!lightmap || size > BLOCKLIGHTS_SIZE)
		return;

	int baselight_index = StyleIndex(surf, BUMP_BASELIGHT_STYLE);
	int addlight_index = StyleIndex(surf, BUMP_ADDLIGHT_STYLE);
	int lightvecs_index = StyleIndex(surf, BUMP_LIGHTVECS_STYLE);

	color24 *baselight_offset = lightmap + size * baselight_index;
	color24 *addlight_offset = lightmap + size * addlight_index;
	color24 *lightvecs_offset = lightmap + size * lightvecs_index;

	for (int i=0 ; i<size ; i++)
	{
		float dot;
		vec3_t baselight, addlight, result;

		baselight[0] = (float)baselight_offset[i].r / 255;
		baselight[1] = (float)baselight_offset[i].g / 255;
		baselight[2] = (float)baselight_offset[i].b / 255;

		addlight[0] = (float)addlight_offset[i].r / 255;
		addlight[1] = (float)addlight_offset[i].g / 255;
		addlight[2] = (float)addlight_offset[i].b / 255;

		dot = (float)lightvecs_offset[i].b;
		dot = (dot / 127) - 1;

		VectorScale(addlight, dot, result);
		VectorAdd(result, baselight, result);

	//	blocklights[i].r = Clamp( ApplyGamma(result[0]*2)/2 );
	//	blocklights[i].g = Clamp( ApplyGamma(result[1]*2)/2 );
	//	blocklights[i].b = Clamp( ApplyGamma(result[2]*2)/2 );
		result[0] = ApplyGamma(result[0]*2)/2;
		result[1] = ApplyGamma(result[1]*2)/2;
		result[2] = ApplyGamma(result[2]*2)/2;
		Clamp(result, &blocklights[i]);
	}

	UploadBlocklights(surf);
}


//===================
// BuildLightMap_FromBumpStyle
//
// Uses specified bump style to fill lightmap block data.
//===================
void BuildLightMap_FromBumpStyle (msurface_t *surf, int rstyle, int gammacorrection)
{
	int size = LightmapSize(surf);
	color24 *lightmap = surf->samples;

	if (!lightmap || size > BLOCKLIGHTS_SIZE)
		return;

	color24 *target_offset = lightmap + size * StyleIndex(surf, rstyle);

	color24 *baselight_offset, *addlight_offset, *lightvecs_offset;
	if (gammacorrection)
	{
		baselight_offset = lightmap + size * StyleIndex(surf, BUMP_BASELIGHT_STYLE);
		addlight_offset = lightmap + size * StyleIndex(surf, BUMP_ADDLIGHT_STYLE);
		lightvecs_offset = lightmap + size * StyleIndex(surf, BUMP_LIGHTVECS_STYLE);
	}

	for (int i=0 ; i<size ; i++)
	{
		if (gammacorrection)
		{
			// calculate "combine" lightmap and apply gamma to it
			float dot;
			vec3_t baselight, addlight, result;
			vec3_t scale = Vector(0, 0, 0);

			baselight[0] = (float)baselight_offset[i].r / 255;
			baselight[1] = (float)baselight_offset[i].g / 255;
			baselight[2] = (float)baselight_offset[i].b / 255;

			addlight[0] = (float)addlight_offset[i].r / 255;
			addlight[1] = (float)addlight_offset[i].g / 255;
			addlight[2] = (float)addlight_offset[i].b / 255;

			dot = (float)lightvecs_offset[i].b;
			dot = (dot / 127) - 1;

			VectorScale(addlight, dot, result);
			VectorAdd(result, baselight, result);
			VectorScale(result, 2, result);

			if (result[0]) scale[0] = ApplyGamma(result[0]) / result[0];
			if (result[1]) scale[1] = ApplyGamma(result[1]) / result[1];
			if (result[2]) scale[2] = ApplyGamma(result[2]) / result[2];

		//	blocklights[i].r = Clamp( (float)target_offset[i].r / 255 * scale[0] );
		//	blocklights[i].g = Clamp( (float)target_offset[i].g / 255 * scale[1] );
		//	blocklights[i].b = Clamp( (float)target_offset[i].b / 255 * scale[2] );
			result[0] = (float)target_offset[i].r / 255.0 * scale[0];
			result[1] = (float)target_offset[i].g / 255.0 * scale[1];
			result[2] = (float)target_offset[i].b / 255.0 * scale[2];
			Clamp(result, &blocklights[i]);
		}
		else
		{
			blocklights[i].r = target_offset[i].r;
			blocklights[i].g = target_offset[i].g;
			blocklights[i].b = target_offset[i].b;
		}
	}

	UploadBlocklights(surf);
}


//===================
// BumpMarkFaces
//
// makes checks easier
//===================
void BumpMarkFaces()
{
	gEngfuncs.Con_DPrintf("Marking faces...");
	model_t	*m = gEngfuncs.GetEntityByIndex(0)->model;
	if (!m) return;

	for (int i=0 ; i<m->numsurfaces ; i++)
	{
		msurface_t *surf = &m->surfaces[i];
		surf->flags &= ~SURF_BUMPREADY; // make shure that flag isn't set..
		surf->flags &= ~SURF_SPECULAR; // make shure that flag isn't set..

		if (surf->flags & (SURF_DRAWSKY|SURF_DRAWTURB))
			continue;
		
		if (StyleIndex(surf, BUMP_BASELIGHT_STYLE) == -1) continue;
		if (StyleIndex(surf, BUMP_ADDLIGHT_STYLE) == -1) continue;
		if (StyleIndex(surf, BUMP_LIGHTVECS_STYLE) == -1) continue;
		
		// has normal map for this texture?
		if (surf->texinfo->texture->anim_min != -666) continue;
		ExtTextureData *pExtData = (ExtTextureData*)surf->texinfo->texture->anim_next;
		if (pExtData->gl_normalmap_id)
		{
			// surface ready to be drawn bumped
			surf->flags |= SURF_BUMPREADY;
		}

		if (pExtData->gl_glossmap_id || (pExtData->gl_extra_glossmap_id && pExtData->gl_normalmap_id))
		{
			// surface ready to be drawn with specular
			surf->flags |= SURF_SPECULAR;
		}		
	}
	gEngfuncs.Con_DPrintf("ok\n");
}

//===================
// UploadLightmaps
//
// uploads all lightmaps that were filled
//===================
int WriteTGA_24old(color24 *pixels, unsigned short width, unsigned short height, const char *filename);
void UploadLightmaps( int base )
{
	char name[128];
	const char *mapName = gEngfuncs.pfnGetLevelName();
	for (int i = 0; i<(lightmapstotal + 1); i++)
	{
		color24 *lightmap = lightmaps + i*BLOCK_WIDTH*BLOCK_HEIGHT;
		/*	if (base == g_lightmap_baselight)
		{
			sprintf(name, "gfx/newlight/%s/map%i.tga", mapName,i);
			int l = CreateTexture(name, MIPS_LIGHT, base + i);
			if (!l)
			{
				gEngfuncs.Con_DPrintf("Unable to load lightmap: %s\n", name);
				gl.glBindTexture(GL_TEXTURE_2D, base + i);
				gl.glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				gl.glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				gl.glTexImage2D(GL_TEXTURE_2D, 0, 3, BLOCK_WIDTH, BLOCK_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, lightmap);
			}
		}
		else{*/
		gl.glBindTexture(GL_TEXTURE_2D, base + i);
		gl.glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		gl.glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		gl.glTexImage2D (GL_TEXTURE_2D, 0, 3, BLOCK_WIDTH, BLOCK_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, lightmap);
	//}
	}
}


int LightmapsNeedsUpdate()
{
	if (!lightmaps_initialized) return TRUE;
	if (current_gamma != cv_gamma->value) return TRUE;
	if (current_brightness != cv_brightness->value) return TRUE;
	if (current_contrast != cv_contrast->value) return TRUE;
	if (cv_bump->value && !current_bumpstate) return TRUE;
	if (!cv_bump->value && current_bumpstate) return TRUE;

	const char* curmapname = gEngfuncs.pfnGetLevelName();
	if (stricmp(curmapname, szMapName)) return TRUE;

	return FALSE;
}

//===================
// UpdateLightmaps
// 
// (Must be called after BumpMarkFaces)
// detects map and light settings changes and loads appropriate lightmaps
//===================
void UpdateLightmaps()
{
	int i;
	world = gEngfuncs.GetEntityByIndex(0)->model;
	if (!world)
		return;

	if (!gl.IsGLAllowed())
	{
		ONCE( gEngfuncs.Con_Printf("ERROR: UpdateLightmaps called when GLmanager not initialized!\n"); );
		return;
	}

	if (!LightmapsNeedsUpdate())
		return;

	const char* curmapname = gEngfuncs.pfnGetLevelName();
	lightmaps_initialized = 1;
	current_gamma = cv_gamma->value;
	current_brightness = cv_brightness->value;
	current_contrast = cv_contrast->value;
	if (current_gamma < 0) current_gamma = 1;
	if (cv_bump->value) current_bumpstate = 1;
	else current_bumpstate = 0;
	strcpy(szMapName, curmapname);
	memset(lightmaps, 0, sizeof(lightmaps));
	gEngfuncs.Con_DPrintf("\n>> Loading additional lightmaps for %s\n", curmapname);
	gEngfuncs.Con_DPrintf("gamma: %f\nbrightness: %f\ncontrast: %f\n", current_gamma, current_brightness, current_contrast);
	gEngfuncs.Con_DPrintf("bump: %d\n", current_bumpstate);

	//==================================
	// Load BASELIGHT style lightmaps
	//==================================
	lightmapstotal = -1;
	for (i=0 ; i<world->numsurfaces ; i++)
	{
		msurface_t *surf = &world->surfaces[i];
		if (surf->flags & (SURF_DRAWSKY|SURF_DRAWTURB))
			continue;

		if (surf->lightmaptexturenum >= MAX_LIGHTMAPS)
		{
			gEngfuncs.Con_Printf("WARNING! surface %d uses lightmap index %d greater than MAX_LIGHTMAPS!\n", i, surf->lightmaptexturenum);
			continue;
		}

		if (surf->lightmaptexturenum > lightmapstotal)
			lightmapstotal = surf->lightmaptexturenum;

		if (StyleIndex(surf, BUMP_BASELIGHT_STYLE) == -1)
		{
			// face doesn't have bump lightmaps
			BuildLightMap_FromOriginal (surf);
			continue;
		}

		if (surf->flags & SURF_BUMPREADY && cv_bump->value)
		{
			// face has bump lightmaps and normal map, it will be drawn bumped
			BuildLightMap_FromBumpStyle (surf, BUMP_BASELIGHT_STYLE, TRUE);
			continue;
		}

		if (StyleIndex(surf, BUMP_ADDLIGHT_STYLE) == -1 ||
			StyleIndex(surf, BUMP_LIGHTVECS_STYLE) == -1)
		{
			// face has baselight style, but no other bump styles.
			// use this as single lightmap
			BuildLightMap_FromBaselight(surf);
			continue;
		}

		// face has bump lightmaps, but no normalmap (or bump is disabled)
		BuildLightMap_Combine (surf);
	}

	if (lightmapstotal == -1)
	{
		gEngfuncs.Con_Printf("ERROR: no lightmaps usage!\n");
		return;
	}

	UploadLightmaps( g_lightmap_baselight );

	//==================================
	// Load ADDLIGHT style lightmaps
	//==================================
	int needs_extra_lightmaps = 0;
	for (i=0 ; i<world->numsurfaces ; i++)
	{
		msurface_t *surf = &world->surfaces[i];
		if (surf->flags & (SURF_DRAWSKY|SURF_DRAWTURB))
			continue;

		if (surf->lightmaptexturenum >= MAX_LIGHTMAPS)
			continue;
		
		if (!(surf->flags & SURF_BUMPREADY) && !(surf->flags & SURF_SPECULAR))
			continue;

		BuildLightMap_FromBumpStyle (surf, BUMP_ADDLIGHT_STYLE, TRUE);
		needs_extra_lightmaps = 1;
	}

	if (needs_extra_lightmaps)
		UploadLightmaps( g_lightmap_addlight );

	//==================================
	// Load LIGHTVECS style lightmaps
	//==================================
	if (needs_extra_lightmaps)
	{
		for (i=0 ; i<world->numsurfaces ; i++)
		{
			msurface_t *surf = &world->surfaces[i];
			if (surf->flags & (SURF_DRAWSKY|SURF_DRAWTURB))
				continue;

			if (surf->lightmaptexturenum >= MAX_LIGHTMAPS)
				continue;
			
			if (!(surf->flags & SURF_BUMPREADY) && !(surf->flags & SURF_SPECULAR))
				continue;

			BuildLightMap_FromBumpStyle (surf, BUMP_LIGHTVECS_STYLE, FALSE);
		}

		UploadLightmaps( g_lightmap_lightvecs );
	}

	if (needs_extra_lightmaps)
		gEngfuncs.Con_Printf("Loaded %d x 3 additional lightmaps\n", lightmapstotal+1);
	else
		gEngfuncs.Con_Printf("Loaded %d additional lightmaps\n", lightmapstotal+1);
}


void SetLightmapBaseIndex()
{
	g_lightmap_baselight = current_ext_texture_id;
	current_ext_texture_id += MAX_LIGHTMAPS;

	g_lightmap_addlight = current_ext_texture_id;
	current_ext_texture_id += MAX_LIGHTMAPS;

	g_lightmap_lightvecs = current_ext_texture_id;
	current_ext_texture_id += MAX_LIGHTMAPS;
}


