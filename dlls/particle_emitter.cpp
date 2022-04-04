#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "particle_emitter.h"
#include <ctype.h>
#include "player.h"

#define SF_START_ON 1
extern int gmsgParticles;
void CParticleEmitter::Spawn(void)
{
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	SET_MODEL(ENT(pev), STRING(pev->model));
	pev->effects |= EF_NODRAW;

	UTIL_SetOrigin(pev, pev->origin);
	UTIL_SetSize(pev, pev->absmin, pev->absmax);

	SetUse(&CParticleEmitter::Use);
	pev->air_finished = -1;
	pev->fuser4 = -1;

	char szFile[128];
	sprintf(szFile, "Am/%s", STRING(pev->message));
	FILE *fp = fopen(szFile, "r");
	if (fp)
	{
		while (!feof(fp) && !ferror(fp))
		{
			char str[64];
			char prop[64], val[64];
			fgets(str, 63, fp);
			sscanf(str, "%s %s", prop, val);
			float lifeTime = 0.0;
			if (!strcmp(prop, "system_life"))
			{
				lifeTime = atof(val);
			}

			if (lifeTime <= 0)
			{
				pev->air_finished = -1;
				pev->fuser4 = -1;
			}
			else
			{
				pev->air_finished = gpGlobals->time + lifeTime;
				pev->fuser4 = lifeTime;
			}
		}
		fclose(fp);
	}
	else
		ALERT(at_console, "Unable to load: %s\n", szFile);

	if (pev->spawnflags & SF_START_ON)
	{
		ALERT(at_console, "CParticleEmitter Spawn in <ON> Mode\nNextthink in 0.5 Seconds\n");
		pev->rendermode = 1;
		
		SetThink(&CParticleEmitter::ResumeThink);
		pev->nextthink = gpGlobals->time + 0.1;
	}
	else
	{
		pev->rendermode = 0;
	}
}

void CParticleEmitter::Precache(void)
{
	SetThink(&CParticleEmitter::ResumeThink);
	pev->nextthink = gpGlobals->time + .4;
}

void CParticleEmitter::ResumeThink(void)
{
	if (pev->rendermode > 0)
		SetThink(&CParticleEmitter::TurnOn);
	else
		SetThink(&CParticleEmitter::TurnOff);
		
	pev->nextthink = gpGlobals->time + 0.1;
}

void CParticleEmitter::TurnOn(void)
{
	if (pev->air_finished > 0 &&
		pev->air_finished < gpGlobals->time)
	{
		SetThink(&CParticleEmitter::TurnOff);
		pev->nextthink = gpGlobals->time + 0.01;//100fps.
		return;
	}

	pev->rendermode = 1;
	Vector vForward;
	g_engfuncs.pfnAngleVectors(pev->angles, vForward, NULL, NULL);

	MESSAGE_BEGIN(MSG_ALL, gmsgParticles);
	WRITE_SHORT(entindex());
	WRITE_BYTE(pev->netname);
	WRITE_COORD(pev->origin.x);
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z);
	WRITE_COORD(vForward.x);
	WRITE_COORD(vForward.y);
	WRITE_COORD(vForward.z);
	WRITE_SHORT(0);
	WRITE_STRING(STRING(pev->message));
	MESSAGE_END();

	SetThink(&CParticleEmitter::ResumeThink);
	pev->nextthink = gpGlobals->time + 0.1;
}

void CParticleEmitter::TurnOff(void)
{
	return;

	pev->rendermode = 0;
	MESSAGE_BEGIN(MSG_ALL, gmsgParticles);
	WRITE_SHORT(entindex());
	WRITE_BYTE(2);
	WRITE_COORD(pev->origin.x);
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z);
	WRITE_COORD(pev->angles.x);
	WRITE_COORD(pev->angles.y);
	WRITE_COORD(pev->angles.z);
	WRITE_SHORT(0);
	WRITE_STRING(STRING(pev->message));
	MESSAGE_END();

	SetThink(&CParticleEmitter::ResumeThink);
	pev->nextthink = gpGlobals->time + 5.0;
}

void CParticleEmitter::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if (pev->rendermode > 0)
		TurnOff();
	else
	{
		if (pev->fuser4 > 0)
			pev->air_finished = gpGlobals->time + pev->fuser4;
		else
			pev->air_finished = -1;

		TurnOn();
	}
}

LINK_ENTITY_TO_CLASS(env_particleemitter, CParticleEmitter);
