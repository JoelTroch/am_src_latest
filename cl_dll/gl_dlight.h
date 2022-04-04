#ifndef __DLIGHT_H__
#define __DLIGHT_H__

#define MAX_DLIGHTS 32
#include "../common/com_model.h"
#include "gl_frustum.h"

struct DynamicLight
{
	vec3_t	origin;
	float	radius;
	vec3_t	color; // ignored for spotlights, they have a texture
	float	die;				// stop lighting after this time
	float	decay;				// drop this each second
	int		key;

	// spotlight specific:
	vec3_t	angles;	
	float	cone_hor;
	float	cone_ver;
	char	spot_texture[64];
	int depth;

	float *cMV;
	float *cP;
	float *lMV;
	float *lP;

	FrustumCheck frustum;
	float *resultProjectionMatrix;
};

DynamicLight* MY_AllocDlight (int key);

int GetDlightsForPoint(vec3_t point, vec3_t mins, vec3_t maxs, DynamicLight **pOutArray, int alwaysFlashlight);
void LightDrawPoly(msurface_t *surf, bool spotlight = false);
extern vec3_t current_light_origin;
extern vec3_t current_spot_vec_forward;
extern DynamicLight cl_dlights[MAX_DLIGHTS];
vec3_t LightForOrigin(vec3_t origin);
vec3_t StaticLightDirectionForOrigin(vec3_t origin);
vec3_t StaticLightBaselightForOrigin(vec3_t origin);
vec3_t StaticLightAddlightForOrigin(vec3_t origin);

#endif