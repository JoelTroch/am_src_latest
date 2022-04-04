#ifndef __PA_EMITTER_
#define __PA_EMITTER_

static unsigned int iParticleIDCount = 0;

class CParticleEmitter : public CBaseEntity
{
public:
	void Spawn();
	void EXPORT ResumeThink(void);
	void EXPORT TurnOn(void);
	void EXPORT TurnOff(void);
	void Precache();

	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	virtual int	ObjectCaps(void) { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
};

#endif