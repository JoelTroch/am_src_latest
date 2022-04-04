#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "player.h"
#include "effects.h"
#include "weapons.h"
#include "decals.h"
#include "func_break.h"
#include "shake.h"

#define	B1(t)		(t*t)
#define	B2(t)		(2*t*(1-t))
#define	B3(t)		((1-t)*(1-t))

struct pointer_ent
{
	CBeam *pBeam;
};

#define START 0
#define MID_POINT 1
#define END 2
#define MAX_SEGMENTS 64

#define SF_ROPE_RANDOM_SEG			1
#define SF_ROPE_RANDOM_LENGHT		2
#define SF_ROPE_RANDOM_THICK		4

#define SF_ROPE_STARTON 1
#define SF_ROPE_NO_PVS_CHECK 2

class CEnvRope : public CPointEntity
{
public:
	void Spawn(void);
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);

	void EXPORT	RopeThink(void);
	void EXPORT	RopeTurnOn(void);
	void EXPORT	RopeThinkContinous(void);

	virtual int		Save(CSave &save);
	virtual int		Restore(CRestore &restore);
	static	TYPEDESCRIPTION m_SaveData[];

	BOOL	m_fRopeActive;
	BOOL	m_fRopeNoUpdate;
	BOOL	m_fRopeDeactivated;
};

LINK_ENTITY_TO_CLASS(env_rope, CEnvRope);

TYPEDESCRIPTION	CEnvRope::m_SaveData[] =
{
	DEFINE_FIELD(CEnvRope, m_fRopeActive, FIELD_BOOLEAN),
	DEFINE_FIELD(CEnvRope, m_fRopeDeactivated, FIELD_BOOLEAN),
};

IMPLEMENT_SAVERESTORE(CEnvRope, CPointEntity);

void CEnvRope::Spawn(void)
{
	PRECACHE_MODEL("sprites/rope.spr");//CLIENT SIDE IS USING THIS

	pev->solid = SOLID_NOT;
	pev->effects |= EF_NODRAW;

	if (FStringNull(pev->targetname))
		pev->spawnflags |= 1;

	ALERT(at_console, "Rope spawn\n");
	pev->nextthink = gpGlobals->time + 1.0;
	SetThink(&CEnvRope::RopeThink);
}
void CEnvRope::RopeThink(void)
{
	if (!(pev->spawnflags & SF_ROPE_STARTON))
	{
		m_fRopeActive = FALSE;
		m_fRopeDeactivated = TRUE;
	}
	else
	{
		m_fRopeActive = TRUE;
		m_fRopeDeactivated = FALSE;
		SetThink(&CEnvRope::RopeTurnOn);
		pev->nextthink = gpGlobals->time + 0.1;
	}
};

typedef struct Rope
{
	vec3_t origin;
	vec3_t target;
	char name[256];
}rope_t;

rope_t SysRopes[512];

int ropekey = 0;

void CEnvRope::RopeTurnOn(void)
{
	if (m_fRopeActive)
	{
		CBaseEntity *pTarget = UTIL_FindEntityByTargetname(NULL, STRING(pev->target));
		extern int gmsgAddRope;

		if (pTarget != NULL)
		{
			MESSAGE_BEGIN(MSG_ALL, gmsgAddRope);

			WRITE_BYTE(entindex());

			WRITE_COORD(pev->origin.x);	// X
			WRITE_COORD(pev->origin.y);	// Y
			WRITE_COORD(pev->origin.z);	// Z

			WRITE_COORD(pTarget->pev->origin.x);	// X
			WRITE_COORD(pTarget->pev->origin.y);	// Y
			WRITE_COORD(pTarget->pev->origin.z);	// Z

			WRITE_STRING(STRING(pev->message));

			MESSAGE_END();
		}
	}

	m_fRopeNoUpdate = TRUE;

	SetThink(&CEnvRope::RopeThinkContinous);
	pev->nextthink = gpGlobals->time + 0.5;
}

void CEnvRope::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if (m_fRopeDeactivated)
	{
		m_fRopeActive = TRUE;
		m_fRopeDeactivated = FALSE;
		SetThink(&CEnvRope::RopeTurnOn);
		pev->nextthink = gpGlobals->time + 0.1;
		return;
	}
	else
	{
		m_fRopeActive = FALSE;
		m_fRopeDeactivated = TRUE;
		SetThink(&CEnvRope::RopeTurnOn);
		pev->nextthink = gpGlobals->time + 0.001;
		return;
	}
};

void CEnvRope::RopeThinkContinous(void)
{
	if (!m_fRopeNoUpdate && m_fRopeActive && !m_fRopeDeactivated)
	{
		SetThink(&CEnvRope::RopeTurnOn);
		pev->nextthink = gpGlobals->time + 0.001;
	}
	else
	{
		SetThink(&CEnvRope::RopeThinkContinous);
		pev->nextthink = gpGlobals->time + 0.1;
	}
}