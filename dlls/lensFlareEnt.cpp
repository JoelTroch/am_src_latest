#include "extdll.h"
#include "util.h"

#include "cbase.h"
#include "gamerules.h"
#include "game.h"
#include "pm_shared.h"

#include "lensFlareEnt.h"
extern int gmsgSpecMsg;
#include "specMessages.h"
LINK_ENTITY_TO_CLASS(env_sun, CLensFlareEnt);

void CLensFlareEnt::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "pitch"))
	{
		pev->fuser1 = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue(pkvd);
}

void CLensFlareEnt::Spawn(void)
{
	pev->classname = MAKE_STRING("env_sun");
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	pev->body = 0;
	sendMessage();
}

void CLensFlareEnt::sendMessage(){
	if (gmsgSpecMsg <= 0) return;

	MESSAGE_BEGIN(MSG_ALL, gmsgSpecMsg);
		WRITE_BYTE(CL_SUNPARAMS);
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z);
		WRITE_COORD(pev->fuser1);
	MESSAGE_END();
}