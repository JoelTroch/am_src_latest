#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "cdll_int.h"
#include "com_model.h"
#include "glmanager.h"
#include "entity_types.h"
#include "gl_texloader.h"

#include "studio_event.h" // def. of mstudioevent_t
#include "r_efx.h"
#include "event_api.h"
#include "pm_defs.h"
#include "pmtrace.h"
#include "parsemsg.h"
#include "gl_renderer.h"
#include "pm_materials.h"
#include "particle_engine.h"

#include "studio.h"
#include "studio_util.h"
#include "r_studioint.h"

#include "StudioModelRenderer.h"
#include "GameStudioModelRenderer.h"
extern CGameStudioModelRenderer g_StudioRenderer;

void CreateDecalTrace(vec3_t start, vec3_t end, const char *name, int persistent = 0);
vec3_t getTextureColor(char *name);
char* EV_HLDM_GetTextureName(pmtrace_t *ptr, float *vecSrc, float *vecEnd);

const char *getDecalName(int material)
{
	switch (material)
	{
	case CHAR_TEX_METAL:
	case CHAR_TEX_VENT:
	case CHAR_TEX_GRATE:
		return "metalshot";
	case CHAR_TEX_WOOD:
		return "woodshot";
	}
	return "wallshot";
}

const char *getParticles(int material)
{
	switch (material)
	{
	case CHAR_TEX_METAL:
	case CHAR_TEX_VENT:
	case CHAR_TEX_GRATE:
		return "engine_metal_impact.txt";
	case CHAR_TEX_SNOW:
		return "engine_snow_impact.txt";
	case CHAR_TEX_DIRT:
		return "engine_dirt_impact.txt";
	case CHAR_TEX_WOOD:
		return "engine_wood_impact.txt";
	}
	if (gEngfuncs.pfnRandomLong(0, 100) > 60)
	return "engine_concrete_impact.txt";
	else
	return "engine_concrete_impact2.txt";//more complex system
}

void fireWeapon(int weapon, int entity, vec3_t endPos,int material)
{
	if (!entity) return;
	cl_entity_t *attacker = gEngfuncs.GetEntityByIndex(entity);
	cl_entity_t *got = NULL;

	Vector start, end;
	start = attacker->origin;
  	start.z += 16;
	end = endPos;

	pmtrace_t pmtrace;
	gEngfuncs.pEventAPI->EV_SetTraceHull(2);
	gEngfuncs.pEventAPI->EV_PlayerTrace(start, endPos, PM_NORMAL, -1, &pmtrace);

	char *name = EV_HLDM_GetTextureName(&pmtrace, pmtrace.endpos + pmtrace.plane.normal * 2, pmtrace.endpos - pmtrace.plane.normal);
	vec3_t color = getTextureColor(name);

	CreateDecalTrace(start, pmtrace.endpos - pmtrace.plane.normal, getDecalName(material), 0);
	got = gEngfuncs.GetEntityByIndex(gEngfuncs.pEventAPI->EV_IndexFromTrace(&pmtrace));

//	gEngfuncs.Con_Printf("weapon idx: %i\n", weapon);

	if (got)
	{
		if (got && got->model && got->model->type == mod_studio)
		{
			//	gEngfuncs.Con_Printf("weapon idx: %i\n", weapon);

			if (weapon == 15 || weapon == 16 || weapon == 17 || weapon == 18)
			g_StudioRenderer.StudioDecalForEntity(pmtrace.endpos - pmtrace.plane.normal, pmtrace.plane.normal, "shotgunwound", got);
			else
				g_StudioRenderer.StudioDecalForEntity(pmtrace.endpos - pmtrace.plane.normal, pmtrace.plane.normal, "wound", got);

		}
		else
			gParticleEngine.CreateCluster((char*)getParticles(material), pmtrace.endpos, pmtrace.plane.normal, 0, color);
	}

}