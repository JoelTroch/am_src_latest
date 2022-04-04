/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "customentity.h"
#include "effects.h"
#include "weapons.h"
#include "decals.h"
#include "func_break.h"
#include "shake.h"
#ifndef WIN32
#include <ctype.h> // tolower
#endif

#define	SF_GIBSHOOTER_REPEATABLE	1 // allows a gibshooter to be refired

#define SF_FUNNEL_REVERSE			1 // funnel effect repels particles instead of attracting them.


// Lightning target, just alias landmark
LINK_ENTITY_TO_CLASS( info_target, CPointEntity );


class CBubbling : public CBaseEntity
{
public:
	void	Spawn( void );
	void	Precache( void );
	void	KeyValue( KeyValueData *pkvd );
	
	void	EXPORT FizzThink( void );
	void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	virtual int		ObjectCaps( void ) { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	static	TYPEDESCRIPTION m_SaveData[];

	int		m_density;
	int		m_frequency;
	int		m_bubbleModel;
	int		m_state;
};

LINK_ENTITY_TO_CLASS( env_bubbles, CBubbling );

TYPEDESCRIPTION	CBubbling::m_SaveData[] = 
{
	DEFINE_FIELD( CBubbling, m_density, FIELD_INTEGER ),
	DEFINE_FIELD( CBubbling, m_frequency, FIELD_INTEGER ),
	DEFINE_FIELD( CBubbling, m_state, FIELD_INTEGER ),
	// Let spawn restore this!
	//	DEFINE_FIELD( CBubbling, m_bubbleModel, FIELD_INTEGER ),
};

IMPLEMENT_SAVERESTORE( CBubbling, CBaseEntity );


#define SF_BUBBLES_STARTOFF		0x0001

void CBubbling::Spawn( void )
{
	Precache( );
	SET_MODEL( ENT(pev), STRING(pev->model) );		// Set size

	pev->solid = SOLID_NOT;							// Remove model & collisions
	pev->renderamt = 0;								// The engine won't draw this model if this is set to 0 and blending is on
	pev->rendermode = kRenderTransTexture;
	int speed = pev->speed > 0 ? pev->speed : -pev->speed;

	// HACKHACK!!! - Speed in rendercolor
	pev->rendercolor.x = speed >> 8;
	pev->rendercolor.y = speed & 255;
	pev->rendercolor.z = (pev->speed < 0) ? 1 : 0;


	if ( !(pev->spawnflags & SF_BUBBLES_STARTOFF) )
	{
		SetThink( &CBubbling::FizzThink );
		pev->nextthink = gpGlobals->time + 2.0;
		m_state = 1;
	}
	else 
		m_state = 0;
}

void CBubbling::Precache( void )
{
	m_bubbleModel = PRECACHE_MODEL("sprites/bubble.spr");			// Precache bubble sprite
}


void CBubbling::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( ShouldToggle( useType, m_state ) )
		m_state = !m_state;

	if ( m_state )
	{
		SetThink( & CBubbling::FizzThink );
		pev->nextthink = gpGlobals->time + 0.1;
	}
	else
	{
		SetThink( NULL );
		pev->nextthink = 0;
	}
}


void CBubbling::KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "density"))
	{
		m_density = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "frequency"))
	{
		m_frequency = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "current"))
	{
		pev->speed = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue( pkvd );
}


void CBubbling::FizzThink( void )
{
	MESSAGE_BEGIN( MSG_PAS, SVC_TEMPENTITY, VecBModelOrigin(pev) );
		WRITE_BYTE( TE_FIZZ );
		WRITE_SHORT( (short)ENTINDEX( edict() ) );
		WRITE_SHORT( (short)m_bubbleModel );
		WRITE_BYTE( m_density );
	MESSAGE_END();

	if ( m_frequency > 19 )
		pev->nextthink = gpGlobals->time + 0.5;
	else
		pev->nextthink = gpGlobals->time + 2.5 - (0.1 * m_frequency);
}

// --------------------------------------------------
// 
// Beams
//
// --------------------------------------------------

LINK_ENTITY_TO_CLASS( beam, CBeam );

void CBeam::Spawn( void )
{
	pev->solid = SOLID_NOT;							// Remove model & collisions
	Precache( );
}

void CBeam::Precache( void )
{
	if ( pev->owner )
		SetStartEntity( ENTINDEX( pev->owner ) );
	if ( pev->aiment )
		SetEndEntity( ENTINDEX( pev->aiment ) );
}

void CBeam::SetStartEntity( int entityIndex ) 
{ 
	pev->sequence = (entityIndex & 0x0FFF) | ((pev->sequence&0xF000)<<12); 
	pev->owner = g_engfuncs.pfnPEntityOfEntIndex( entityIndex );
}

void CBeam::SetEndEntity( int entityIndex ) 
{ 
	pev->skin = (entityIndex & 0x0FFF) | ((pev->skin&0xF000)<<12); 
	pev->aiment = g_engfuncs.pfnPEntityOfEntIndex( entityIndex );
}


// These don't take attachments into account
const Vector &CBeam::GetStartPos( void )
{
	if ( GetType() == BEAM_ENTS )
	{
		edict_t *pent =  g_engfuncs.pfnPEntityOfEntIndex( GetStartEntity() );
		return pent->v.origin;
	}
	return pev->origin;
}


const Vector &CBeam::GetEndPos( void )
{
	int type = GetType();
	if ( type == BEAM_POINTS || type == BEAM_HOSE )
	{
		return pev->angles;
	}

	edict_t *pent =  g_engfuncs.pfnPEntityOfEntIndex( GetEndEntity() );
	if ( pent )
		return pent->v.origin;
	return pev->angles;
}


CBeam *CBeam::BeamCreate( const char *pSpriteName, int width )
{
	// Create a new entity with CBeam private data
	CBeam *pBeam = GetClassPtr( (CBeam *)NULL );
	pBeam->pev->classname = MAKE_STRING("beam");

	pBeam->BeamInit( pSpriteName, width );

	return pBeam;
}


void CBeam::BeamInit( const char *pSpriteName, int width )
{
	pev->flags |= FL_CUSTOMENTITY;
	SetColor( 255, 255, 255 );
	SetBrightness( 255 );
	SetNoise( 0 );
	SetFrame( 0 );
	SetScrollRate( 0 );
	pev->model = MAKE_STRING( pSpriteName );
	SetTexture( PRECACHE_MODEL( (char *)pSpriteName ) );
	SetWidth( width );
	pev->skin = 0;
	pev->sequence = 0;
	pev->rendermode = 0;
}


void CBeam::PointsInit( const Vector &start, const Vector &end )
{
	SetType( BEAM_POINTS );
	SetStartPos( start );
	SetEndPos( end );
	SetStartAttachment( 0 );
	SetEndAttachment( 0 );
	RelinkBeam();
}


void CBeam::HoseInit( const Vector &start, const Vector &direction )
{
	SetType( BEAM_HOSE );
	SetStartPos( start );
	SetEndPos( direction );
	SetStartAttachment( 0 );
	SetEndAttachment( 0 );
	RelinkBeam();
}


void CBeam::PointEntInit( const Vector &start, int endIndex )
{
	SetType( BEAM_ENTPOINT );
	SetStartPos( start );
	SetEndEntity( endIndex );
	SetStartAttachment( 0 );
	SetEndAttachment( 0 );
	RelinkBeam();
}

void CBeam::EntsInit( int startIndex, int endIndex )
{
	SetType( BEAM_ENTS );
	SetStartEntity( startIndex );
	SetEndEntity( endIndex );
	SetStartAttachment( 0 );
	SetEndAttachment( 0 );
	RelinkBeam();
}


void CBeam::RelinkBeam( void )
{
	const Vector &startPos = GetStartPos(), &endPos = GetEndPos();

	pev->mins.x = min( startPos.x, endPos.x );
	pev->mins.y = min( startPos.y, endPos.y );
	pev->mins.z = min( startPos.z, endPos.z );
	pev->maxs.x = max( startPos.x, endPos.x );
	pev->maxs.y = max( startPos.y, endPos.y );
	pev->maxs.z = max( startPos.z, endPos.z );
	pev->mins = pev->mins - pev->origin;
	pev->maxs = pev->maxs - pev->origin;

	UTIL_SetSize( pev, pev->mins, pev->maxs );
	UTIL_SetOrigin( pev, pev->origin );
}

#if 0
void CBeam::SetObjectCollisionBox( void )
{
	const Vector &startPos = GetStartPos(), &endPos = GetEndPos();

	pev->absmin.x = min( startPos.x, endPos.x );
	pev->absmin.y = min( startPos.y, endPos.y );
	pev->absmin.z = min( startPos.z, endPos.z );
	pev->absmax.x = max( startPos.x, endPos.x );
	pev->absmax.y = max( startPos.y, endPos.y );
	pev->absmax.z = max( startPos.z, endPos.z );
}
#endif


void CBeam::TriggerTouch( CBaseEntity *pOther )
{
	if ( pOther->pev->flags & (FL_CLIENT | FL_MONSTER) )
	{
		if ( pev->owner )
		{
			CBaseEntity *pOwner = CBaseEntity::Instance(pev->owner);
			pOwner->Use( pOther, this, USE_TOGGLE, 0 );
		}
		ALERT( at_console, "Firing targets!!!\n" );
	}
}


CBaseEntity *CBeam::RandomTargetname( const char *szName )
{
	int total = 0;

	CBaseEntity *pEntity = NULL;
	CBaseEntity *pNewEntity = NULL;
	while ((pNewEntity = UTIL_FindEntityByTargetname( pNewEntity, szName )) != NULL)
	{
		total++;
		if (RANDOM_LONG(0,total-1) < 1)
			pEntity = pNewEntity;
	}
	return pEntity;
}


void CBeam::DoSparks( const Vector &start, const Vector &end )
{
	if ( pev->spawnflags & (SF_BEAM_SPARKSTART|SF_BEAM_SPARKEND) )
	{
		if ( pev->spawnflags & SF_BEAM_SPARKSTART )
		{
			UTIL_Sparks( start );
		}
		if ( pev->spawnflags & SF_BEAM_SPARKEND )
		{
			UTIL_Sparks( end );
		}
	}
}


class CLightning : public CBeam
{
public:
	void	Spawn( void );
	void	Precache( void );
	void	KeyValue( KeyValueData *pkvd );
	void	Activate( void );

	void	EXPORT StrikeThink( void );
	void	EXPORT DamageThink( void );
	void	RandomArea( void );
	void	RandomPoint( Vector &vecSrc );
	void	Zap( const Vector &vecSrc, const Vector &vecDest );
	void	EXPORT StrikeUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void	EXPORT ToggleUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	
	inline BOOL ServerSide( void )
	{
		if ( m_life == 0 && !(pev->spawnflags & SF_BEAM_RING) )
			return TRUE;
		return FALSE;
	}

	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	void	BeamUpdateVars( void );

	int		m_active;
	int		m_iszStartEntity;
	int		m_iszEndEntity;
	float	m_life;
	int		m_boltWidth;
	int		m_noiseAmplitude;
	int		m_brightness;
	int		m_speed;
	float	m_restrike;
	int		m_spriteTexture;
	int		m_iszSpriteName;
	int		m_frameStart;

	float	m_radius;
};

LINK_ENTITY_TO_CLASS( env_lightning, CLightning );
LINK_ENTITY_TO_CLASS( env_beam, CLightning );

// UNDONE: Jay -- This is only a test
#if _DEBUG
class CTripBeam : public CLightning
{
	void Spawn( void );
};
LINK_ENTITY_TO_CLASS( trip_beam, CTripBeam );

void CTripBeam::Spawn( void )
{
	CLightning::Spawn();
	SetTouch( &CTripBeam::TriggerTouch );
	pev->solid = SOLID_TRIGGER;
	RelinkBeam();
}
#endif



TYPEDESCRIPTION	CLightning::m_SaveData[] = 
{
	DEFINE_FIELD( CLightning, m_active, FIELD_INTEGER ),
	DEFINE_FIELD( CLightning, m_iszStartEntity, FIELD_STRING ),
	DEFINE_FIELD( CLightning, m_iszEndEntity, FIELD_STRING ),
	DEFINE_FIELD( CLightning, m_life, FIELD_FLOAT ),
	DEFINE_FIELD( CLightning, m_boltWidth, FIELD_INTEGER ),
	DEFINE_FIELD( CLightning, m_noiseAmplitude, FIELD_INTEGER ),
	DEFINE_FIELD( CLightning, m_brightness, FIELD_INTEGER ),
	DEFINE_FIELD( CLightning, m_speed, FIELD_INTEGER ),
	DEFINE_FIELD( CLightning, m_restrike, FIELD_FLOAT ),
	DEFINE_FIELD( CLightning, m_spriteTexture, FIELD_INTEGER ),
	DEFINE_FIELD( CLightning, m_iszSpriteName, FIELD_STRING ),
	DEFINE_FIELD( CLightning, m_frameStart, FIELD_INTEGER ),
	DEFINE_FIELD( CLightning, m_radius, FIELD_FLOAT ),
};

IMPLEMENT_SAVERESTORE( CLightning, CBeam );


void CLightning::Spawn( void )
{
	if ( FStringNull( m_iszSpriteName ) )
	{
		SetThink( &CLightning::SUB_Remove );
		return;
	}
	pev->solid = SOLID_NOT;							// Remove model & collisions
	Precache( );

	pev->dmgtime = gpGlobals->time;

	if ( ServerSide() )
	{
		SetThink( NULL );
		if ( pev->dmg > 0 )
		{
			SetThink( &CLightning::DamageThink );
			pev->nextthink = gpGlobals->time + 0.1;
		}
		if ( pev->targetname )
		{
			if ( !(pev->spawnflags & SF_BEAM_STARTON) )
			{
				pev->effects = EF_NODRAW;
				m_active = 0;
				pev->nextthink = 0;
			}
			else
				m_active = 1;
		
			SetUse( &CLightning::ToggleUse );
		}
	}
	else
	{
		m_active = 0;
		if ( !FStringNull(pev->targetname) )
		{
			SetUse( &CLightning::StrikeUse );
		}
		if ( FStringNull(pev->targetname) || FBitSet(pev->spawnflags, SF_BEAM_STARTON) )
		{
			SetThink( &CLightning::StrikeThink );
			pev->nextthink = gpGlobals->time + 1.0;
		}
	}
}

void CLightning::Precache( void )
{
	m_spriteTexture = PRECACHE_MODEL( (char *)STRING(m_iszSpriteName) );
	CBeam::Precache();
}


void CLightning::Activate( void )
{
	if ( ServerSide() )
		BeamUpdateVars();
}


void CLightning::KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "LightningStart"))
	{
		m_iszStartEntity = ALLOC_STRING( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "LightningEnd"))
	{
		m_iszEndEntity = ALLOC_STRING( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "life"))
	{
		m_life = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "BoltWidth"))
	{
		m_boltWidth = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "NoiseAmplitude"))
	{
		m_noiseAmplitude = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "TextureScroll"))
	{
		m_speed = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "StrikeTime"))
	{
		m_restrike = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "texture"))
	{
		m_iszSpriteName = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "framestart"))
	{
		m_frameStart = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "Radius"))
	{
		m_radius = atof( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "damage"))
	{
		pev->dmg = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBeam::KeyValue( pkvd );
}


void CLightning::ToggleUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( !ShouldToggle( useType, m_active ) )
		return;
	if ( m_active )
	{
		m_active = 0;
		pev->effects |= EF_NODRAW;
		pev->nextthink = 0;
	}
	else
	{
		m_active = 1;
		pev->effects &= ~EF_NODRAW;
		DoSparks( GetStartPos(), GetEndPos() );
		if ( pev->dmg > 0 )
		{
			pev->nextthink = gpGlobals->time;
			pev->dmgtime = gpGlobals->time;
		}
	}
}


void CLightning::StrikeUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( !ShouldToggle( useType, m_active ) )
		return;

	if ( m_active )
	{
		m_active = 0;
		SetThink( NULL );
	}
	else
	{
		SetThink( &CLightning::StrikeThink );
		pev->nextthink = gpGlobals->time + 0.1;
	}

	if ( !FBitSet( pev->spawnflags, SF_BEAM_TOGGLE ) )
		SetUse( NULL );
}


int IsPointEntity( CBaseEntity *pEnt )
{
	if ( !pEnt->pev->modelindex )
		return 1;
	if ( FClassnameIs( pEnt->pev, "info_target" ) || FClassnameIs( pEnt->pev, "info_landmark" ) || FClassnameIs( pEnt->pev, "path_corner" ) )
		return 1;

	return 0;
}


void CLightning::StrikeThink( void )
{
	if ( m_life != 0 )
	{
		if ( pev->spawnflags & SF_BEAM_RANDOM )
			pev->nextthink = gpGlobals->time + m_life + RANDOM_FLOAT( 0, m_restrike );
		else
			pev->nextthink = gpGlobals->time + m_life + m_restrike;
	}
	m_active = 1;

	if (FStringNull(m_iszEndEntity))
	{
		if (FStringNull(m_iszStartEntity))
		{
			RandomArea( );
		}
		else
		{
			CBaseEntity *pStart = RandomTargetname( STRING(m_iszStartEntity) );
			if (pStart != NULL)
				RandomPoint( pStart->pev->origin );
			else
				ALERT( at_console, "env_beam: unknown entity \"%s\"\n", STRING(m_iszStartEntity) );
		}
		return;
	}

	CBaseEntity *pStart = RandomTargetname( STRING(m_iszStartEntity) );
	CBaseEntity *pEnd = RandomTargetname( STRING(m_iszEndEntity) );

	if ( pStart != NULL && pEnd != NULL )
	{
		if ( IsPointEntity( pStart ) || IsPointEntity( pEnd ) )
		{
			if ( pev->spawnflags & SF_BEAM_RING)
			{
				// don't work
				return;
			}
		}

		MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
			if ( IsPointEntity( pStart ) || IsPointEntity( pEnd ) )
			{
				if ( !IsPointEntity( pEnd ) )	// One point entity must be in pEnd
				{
					CBaseEntity *pTemp;
					pTemp = pStart;
					pStart = pEnd;
					pEnd = pTemp;
				}
				if ( !IsPointEntity( pStart ) )	// One sided
				{
					WRITE_BYTE( TE_BEAMENTPOINT );
					WRITE_SHORT( pStart->entindex() );
					WRITE_COORD( pEnd->pev->origin.x);
					WRITE_COORD( pEnd->pev->origin.y);
					WRITE_COORD( pEnd->pev->origin.z);
				}
				else
				{
					WRITE_BYTE( TE_BEAMPOINTS);
					WRITE_COORD( pStart->pev->origin.x);
					WRITE_COORD( pStart->pev->origin.y);
					WRITE_COORD( pStart->pev->origin.z);
					WRITE_COORD( pEnd->pev->origin.x);
					WRITE_COORD( pEnd->pev->origin.y);
					WRITE_COORD( pEnd->pev->origin.z);
				}


			}
			else
			{
				if ( pev->spawnflags & SF_BEAM_RING)
					WRITE_BYTE( TE_BEAMRING );
				else
					WRITE_BYTE( TE_BEAMENTS );
				WRITE_SHORT( pStart->entindex() );
				WRITE_SHORT( pEnd->entindex() );
			}

			WRITE_SHORT( m_spriteTexture );
			WRITE_BYTE( m_frameStart ); // framestart
			WRITE_BYTE( (int)pev->framerate); // framerate
			WRITE_BYTE( (int)(m_life*10.0) ); // life
			WRITE_BYTE( m_boltWidth );  // width
			WRITE_BYTE( m_noiseAmplitude );   // noise
			WRITE_BYTE( (int)pev->rendercolor.x );   // r, g, b
			WRITE_BYTE( (int)pev->rendercolor.y );   // r, g, b
			WRITE_BYTE( (int)pev->rendercolor.z );   // r, g, b
			WRITE_BYTE( pev->renderamt );	// brightness
			WRITE_BYTE( m_speed );		// speed
		MESSAGE_END();
		DoSparks( pStart->pev->origin, pEnd->pev->origin );
		if ( pev->dmg > 0 )
		{
			TraceResult tr;
			UTIL_TraceLine( pStart->pev->origin, pEnd->pev->origin, dont_ignore_monsters, NULL, &tr );
			BeamDamageInstant( &tr, pev->dmg );
		}
	}
}


void CBeam::BeamDamage( TraceResult *ptr )
{
	RelinkBeam();
	if ( ptr->flFraction != 1.0 && ptr->pHit != NULL )
	{
		CBaseEntity *pHit = CBaseEntity::Instance(ptr->pHit);
		if ( pHit )
		{
			ClearMultiDamage();
			pHit->TraceAttack( pev, pev->dmg * (gpGlobals->time - pev->dmgtime), (ptr->vecEndPos - pev->origin).Normalize(), ptr, DMG_ENERGYBEAM );
			ApplyMultiDamage( pev, pev );
			if ( pev->spawnflags & SF_BEAM_DECALS )
			{
				if ( pHit->IsBSPModel() )
					UTIL_DecalTrace( ptr, DECAL_BIGSHOT1 + RANDOM_LONG(0,4) );
			}
		}
	}
	pev->dmgtime = gpGlobals->time;
}


void CLightning::DamageThink( void )
{
	pev->nextthink = gpGlobals->time + 0.1;
	TraceResult tr;
	UTIL_TraceLine( GetStartPos(), GetEndPos(), dont_ignore_monsters, NULL, &tr );
	BeamDamage( &tr );
}



void CLightning::Zap( const Vector &vecSrc, const Vector &vecDest )
{
#if 1
	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_BEAMPOINTS);
		WRITE_COORD(vecSrc.x);
		WRITE_COORD(vecSrc.y);
		WRITE_COORD(vecSrc.z);
		WRITE_COORD(vecDest.x);
		WRITE_COORD(vecDest.y);
		WRITE_COORD(vecDest.z);
		WRITE_SHORT( m_spriteTexture );
		WRITE_BYTE( m_frameStart ); // framestart
		WRITE_BYTE( (int)pev->framerate); // framerate
		WRITE_BYTE( (int)(m_life*10.0) ); // life
		WRITE_BYTE( m_boltWidth );  // width
		WRITE_BYTE( m_noiseAmplitude );   // noise
		WRITE_BYTE( (int)pev->rendercolor.x );   // r, g, b
		WRITE_BYTE( (int)pev->rendercolor.y );   // r, g, b
		WRITE_BYTE( (int)pev->rendercolor.z );   // r, g, b
		WRITE_BYTE( pev->renderamt );	// brightness
		WRITE_BYTE( m_speed );		// speed
	MESSAGE_END();
#else
	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE(TE_LIGHTNING);
		WRITE_COORD(vecSrc.x);
		WRITE_COORD(vecSrc.y);
		WRITE_COORD(vecSrc.z);
		WRITE_COORD(vecDest.x);
		WRITE_COORD(vecDest.y);
		WRITE_COORD(vecDest.z);
		WRITE_BYTE(10);
		WRITE_BYTE(50);
		WRITE_BYTE(40);
		WRITE_SHORT(m_spriteTexture);
	MESSAGE_END();
#endif
	DoSparks( vecSrc, vecDest );
}

void CLightning::RandomArea( void )
{
	int iLoops = 0;

	for (iLoops = 0; iLoops < 10; iLoops++)
	{
		Vector vecSrc = pev->origin;

		Vector vecDir1 = Vector( RANDOM_FLOAT( -1.0, 1.0 ), RANDOM_FLOAT( -1.0, 1.0 ),RANDOM_FLOAT( -1.0, 1.0 ) );
		vecDir1 = vecDir1.Normalize();
		TraceResult		tr1;
		UTIL_TraceLine( vecSrc, vecSrc + vecDir1 * m_radius, ignore_monsters, ENT(pev), &tr1 );

		if (tr1.flFraction == 1.0)
			continue;

		Vector vecDir2;
		do {
			vecDir2 = Vector( RANDOM_FLOAT( -1.0, 1.0 ), RANDOM_FLOAT( -1.0, 1.0 ),RANDOM_FLOAT( -1.0, 1.0 ) );
		} while (DotProduct(vecDir1, vecDir2 ) > 0);
		vecDir2 = vecDir2.Normalize();
		TraceResult		tr2;
		UTIL_TraceLine( vecSrc, vecSrc + vecDir2 * m_radius, ignore_monsters, ENT(pev), &tr2 );

		if (tr2.flFraction == 1.0)
			continue;

		if ((tr1.vecEndPos - tr2.vecEndPos).Length() < m_radius * 0.1)
			continue;

		UTIL_TraceLine( tr1.vecEndPos, tr2.vecEndPos, ignore_monsters, ENT(pev), &tr2 );

		if (tr2.flFraction != 1.0)
			continue;

		Zap( tr1.vecEndPos, tr2.vecEndPos );

		break;
	}
}


void CLightning::RandomPoint( Vector &vecSrc )
{
	int iLoops = 0;

	for (iLoops = 0; iLoops < 10; iLoops++)
	{
		Vector vecDir1 = Vector( RANDOM_FLOAT( -1.0, 1.0 ), RANDOM_FLOAT( -1.0, 1.0 ),RANDOM_FLOAT( -1.0, 1.0 ) );
		vecDir1 = vecDir1.Normalize();
		TraceResult		tr1;
		UTIL_TraceLine( vecSrc, vecSrc + vecDir1 * m_radius, ignore_monsters, ENT(pev), &tr1 );

		if ((tr1.vecEndPos - vecSrc).Length() < m_radius * 0.1)
			continue;

		if (tr1.flFraction == 1.0)
			continue;

		Zap( vecSrc, tr1.vecEndPos );
		break;
	}
}



void CLightning::BeamUpdateVars( void )
{
	int beamType;
	int pointStart, pointEnd;

	edict_t *pStart = FIND_ENTITY_BY_TARGETNAME ( NULL, STRING(m_iszStartEntity) );
	edict_t *pEnd = FIND_ENTITY_BY_TARGETNAME ( NULL, STRING(m_iszEndEntity) );
	pointStart = IsPointEntity( CBaseEntity::Instance(pStart) );
	pointEnd = IsPointEntity( CBaseEntity::Instance(pEnd) );

	pev->skin = 0;
	pev->sequence = 0;
	pev->rendermode = 0;
	pev->flags |= FL_CUSTOMENTITY;
	pev->model = m_iszSpriteName;
	SetTexture( m_spriteTexture );

	beamType = BEAM_ENTS;
	if ( pointStart || pointEnd )
	{
		if ( !pointStart )	// One point entity must be in pStart
		{
			edict_t *pTemp;
			// Swap start & end
			pTemp = pStart;
			pStart = pEnd;
			pEnd = pTemp;
			int swap = pointStart;
			pointStart = pointEnd;
			pointEnd = swap;
		}
		if ( !pointEnd )
			beamType = BEAM_ENTPOINT;
		else
			beamType = BEAM_POINTS;
	}

	SetType( beamType );
	if ( beamType == BEAM_POINTS || beamType == BEAM_ENTPOINT || beamType == BEAM_HOSE )
	{
		SetStartPos( pStart->v.origin );
		if ( beamType == BEAM_POINTS || beamType == BEAM_HOSE )
			SetEndPos( pEnd->v.origin );
		else
			SetEndEntity( ENTINDEX(pEnd) );
	}
	else
	{
		SetStartEntity( ENTINDEX(pStart) );
		SetEndEntity( ENTINDEX(pEnd) );
	}

	RelinkBeam();

	SetWidth( m_boltWidth );
	SetNoise( m_noiseAmplitude );
	SetFrame( m_frameStart );
	SetScrollRate( m_speed );
	if ( pev->spawnflags & SF_BEAM_SHADEIN )
		SetFlags( BEAM_FSHADEIN );
	else if ( pev->spawnflags & SF_BEAM_SHADEOUT )
		SetFlags( BEAM_FSHADEOUT );
}


LINK_ENTITY_TO_CLASS( env_laser, CLaser );

TYPEDESCRIPTION	CLaser::m_SaveData[] = 
{
	DEFINE_FIELD( CLaser, m_pSprite, FIELD_CLASSPTR ),
	DEFINE_FIELD( CLaser, m_iszSpriteName, FIELD_STRING ),
	DEFINE_FIELD( CLaser, m_firePosition, FIELD_POSITION_VECTOR ),
};

IMPLEMENT_SAVERESTORE( CLaser, CBeam );

void CLaser::Spawn( void )
{
	if ( FStringNull( pev->model ) )
	{
		SetThink( &CLaser::SUB_Remove );
		return;
	}
	pev->solid = SOLID_NOT;							// Remove model & collisions
	Precache( );

	SetThink( &CLaser::StrikeThink );
	pev->flags |= FL_CUSTOMENTITY;

	PointsInit( pev->origin, pev->origin );

	if ( !m_pSprite && m_iszSpriteName )
		m_pSprite = CSprite::SpriteCreate( STRING(m_iszSpriteName), pev->origin, TRUE );
	else
		m_pSprite = NULL;

	if ( m_pSprite )
		m_pSprite->SetTransparency( kRenderGlow, pev->rendercolor.x, pev->rendercolor.y, pev->rendercolor.z, pev->renderamt, pev->renderfx );

	if ( pev->targetname && !(pev->spawnflags & SF_BEAM_STARTON) )
		TurnOff();
	else
		TurnOn();
}

void CLaser::Precache( void )
{
	pev->modelindex = PRECACHE_MODEL( (char *)STRING(pev->model) );
	if ( m_iszSpriteName )
		PRECACHE_MODEL( (char *)STRING(m_iszSpriteName) );
}


void CLaser::KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "LaserTarget"))
	{
		pev->message = ALLOC_STRING( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "width"))
	{
		SetWidth( (int) atof(pkvd->szValue) );
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "NoiseAmplitude"))
	{
		SetNoise( atoi(pkvd->szValue) );
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "TextureScroll"))
	{
		SetScrollRate( atoi(pkvd->szValue) );
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "texture"))
	{
		pev->model = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "EndSprite"))
	{
		m_iszSpriteName = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "framestart"))
	{
		pev->frame = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "damage"))
	{
		pev->dmg = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBeam::KeyValue( pkvd );
}


int CLaser::IsOn( void )
{
	if (pev->effects & EF_NODRAW)
		return 0;
	return 1;
}


void CLaser::TurnOff( void )
{
	pev->effects |= EF_NODRAW;
	pev->nextthink = 0;
	if ( m_pSprite )
		m_pSprite->TurnOff();
}


void CLaser::TurnOn( void )
{
	pev->effects &= ~EF_NODRAW;
	if ( m_pSprite )
		m_pSprite->TurnOn();
	pev->dmgtime = gpGlobals->time;
	pev->nextthink = gpGlobals->time;
}


void CLaser::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	int active = IsOn();

	if ( !ShouldToggle( useType, active ) )
		return;
	if ( active )
	{
		TurnOff();
	}
	else
	{
		TurnOn();
	}
}


void CLaser::FireAtPoint( TraceResult &tr )
{
	SetEndPos( tr.vecEndPos );
	if ( m_pSprite )
		UTIL_SetOrigin( m_pSprite->pev, tr.vecEndPos );

	BeamDamage( &tr );
	DoSparks( GetStartPos(), tr.vecEndPos );
}

void CLaser::StrikeThink( void )
{
	CBaseEntity *pEnd = RandomTargetname( STRING(pev->message) );

	if ( pEnd )
		m_firePosition = pEnd->pev->origin;

	TraceResult tr;

	UTIL_TraceLine( pev->origin, m_firePosition, dont_ignore_monsters, NULL, &tr );
	FireAtPoint( tr );
	pev->nextthink = gpGlobals->time + 0.1;
}



class CGlow : public CPointEntity
{
public:
	void Spawn( void );
	void Think( void );
	void Animate( float frames );
	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	float		m_lastTime;
	float		m_maxFrame;
};

LINK_ENTITY_TO_CLASS( env_glow, CGlow );

TYPEDESCRIPTION	CGlow::m_SaveData[] = 
{
	DEFINE_FIELD( CGlow, m_lastTime, FIELD_TIME ),
	DEFINE_FIELD( CGlow, m_maxFrame, FIELD_FLOAT ),
};

IMPLEMENT_SAVERESTORE( CGlow, CPointEntity );

void CGlow::Spawn( void )
{
	pev->solid			= SOLID_NOT;
	pev->movetype		= MOVETYPE_NONE;
	pev->effects		= 0;
	pev->frame			= 0;

	PRECACHE_MODEL( (char *)STRING(pev->model) );
	SET_MODEL( ENT(pev), STRING(pev->model) );

	m_maxFrame = (float) MODEL_FRAMES( pev->modelindex ) - 1;
	if ( m_maxFrame > 1.0 && pev->framerate != 0 )
		pev->nextthink	= gpGlobals->time + 0.1;

	m_lastTime = gpGlobals->time;
}


void CGlow::Think( void )
{
	Animate( pev->framerate * (gpGlobals->time - m_lastTime) );

	pev->nextthink		= gpGlobals->time + 0.1;
	m_lastTime			= gpGlobals->time;
}


void CGlow::Animate( float frames )
{ 
	if ( m_maxFrame > 0 )
		pev->frame = fmod( pev->frame + frames, m_maxFrame );
}


LINK_ENTITY_TO_CLASS( env_sprite, CSprite );

TYPEDESCRIPTION	CSprite::m_SaveData[] = 
{
	DEFINE_FIELD( CSprite, m_lastTime, FIELD_TIME ),
	DEFINE_FIELD( CSprite, m_maxFrame, FIELD_FLOAT ),
};

IMPLEMENT_SAVERESTORE( CSprite, CPointEntity );

void CSprite::Spawn( void )
{
	pev->solid			= SOLID_NOT;
	pev->movetype		= MOVETYPE_NONE;
	pev->effects		= 0;
	pev->frame			= 0;

	Precache();
	SET_MODEL( ENT(pev), STRING(pev->model) );

	m_maxFrame = (float) MODEL_FRAMES( pev->modelindex ) - 1;
	if ( pev->targetname && !(pev->spawnflags & SF_SPRITE_STARTON) )
		TurnOff();
	else
		TurnOn();
	
	// Worldcraft only sets y rotation, copy to Z
	if ( pev->angles.y != 0 && pev->angles.z == 0 )
	{
		pev->angles.z = pev->angles.y;
		pev->angles.y = 0;
	}
}


void CSprite::Precache( void )
{
	PRECACHE_MODEL( (char *)STRING(pev->model) );

	// Reset attachment after save/restore
	if ( pev->aiment )
		SetAttachment( pev->aiment, pev->body );
	else
	{
		// Clear attachment
		pev->skin = 0;
		pev->body = 0;
	}
}


void CSprite::SpriteInit( const char *pSpriteName, const Vector &origin )
{
	pev->model = MAKE_STRING(pSpriteName);
	pev->origin = origin;
	Spawn();
}

CSprite *CSprite::SpriteCreate( const char *pSpriteName, const Vector &origin, BOOL animate )
{
	CSprite *pSprite = GetClassPtr( (CSprite *)NULL );
	pSprite->SpriteInit( pSpriteName, origin );
	pSprite->pev->classname = MAKE_STRING("env_sprite");
	pSprite->pev->solid = SOLID_NOT;
	pSprite->pev->movetype = MOVETYPE_NOCLIP;
	if ( animate )
		pSprite->TurnOn();

	return pSprite;
}


void CSprite::AnimateThink( void )
{
	Animate( pev->framerate * (gpGlobals->time - m_lastTime) );

	pev->nextthink		= gpGlobals->time + 0.1;
	m_lastTime			= gpGlobals->time;
}

void CSprite::AnimateUntilDead( void )
{
	if ( gpGlobals->time > pev->dmgtime )
		UTIL_Remove(this);
	else
	{
		AnimateThink();
		pev->nextthink = gpGlobals->time;
	}
}

void CSprite::Expand( float scaleSpeed, float fadeSpeed )
{
	pev->speed = scaleSpeed;
	pev->health = fadeSpeed;
	SetThink( &CSprite::ExpandThink );

	pev->nextthink	= gpGlobals->time;
	m_lastTime		= gpGlobals->time;
}


void CSprite::ExpandThink( void )
{
	float frametime = gpGlobals->time - m_lastTime;
	pev->scale += pev->speed * frametime;
	pev->renderamt -= pev->health * frametime;
	if ( pev->renderamt <= 0 )
	{
		pev->renderamt = 0;
		UTIL_Remove( this );
	}
	else
	{
		pev->nextthink		= gpGlobals->time + 0.1;
		m_lastTime			= gpGlobals->time;
	}
}


void CSprite::Animate( float frames )
{ 
	pev->frame += frames;
	if ( pev->frame > m_maxFrame )
	{
		if ( pev->spawnflags & SF_SPRITE_ONCE )
		{
			TurnOff();
		}
		else
		{
			if ( m_maxFrame > 0 )
				pev->frame = fmod( pev->frame, m_maxFrame );
		}
	}
}


void CSprite::TurnOff( void )
{
	pev->effects = EF_NODRAW;
	pev->nextthink = 0;
}


void CSprite::TurnOn( void )
{
	pev->effects = 0;
	if ( (pev->framerate && m_maxFrame > 1.0) || (pev->spawnflags & SF_SPRITE_ONCE) )
	{
		SetThink( &CSprite::AnimateThink );
		pev->nextthink = gpGlobals->time;
		m_lastTime = gpGlobals->time;
	}
	pev->frame = 0;
}


void CSprite::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	int on = pev->effects != EF_NODRAW;
	if ( ShouldToggle( useType, on ) )
	{
		if ( on )
		{
			TurnOff();
		}
		else
		{
			TurnOn();
		}
	}
}


class CGibShooter : public CBaseDelay
{
public:
	void	Spawn( void );
	void	Precache( void );
	void	KeyValue( KeyValueData *pkvd );
	void EXPORT ShootThink( void );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	virtual CGib *CreateGib( void );

	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	int	m_iGibs;
	int m_iGibCapacity;
	int m_iGibMaterial;
	int m_iGibModelIndex;
	float m_flGibVelocity;
	float m_flVariance;
	float m_flGibLife;
};

TYPEDESCRIPTION CGibShooter::m_SaveData[] =
{
	DEFINE_FIELD( CGibShooter, m_iGibs, FIELD_INTEGER ),
	DEFINE_FIELD( CGibShooter, m_iGibCapacity, FIELD_INTEGER ),
	DEFINE_FIELD( CGibShooter, m_iGibMaterial, FIELD_INTEGER ),
	DEFINE_FIELD( CGibShooter, m_iGibModelIndex, FIELD_INTEGER ),
	DEFINE_FIELD( CGibShooter, m_flGibVelocity, FIELD_FLOAT ),
	DEFINE_FIELD( CGibShooter, m_flVariance, FIELD_FLOAT ),
	DEFINE_FIELD( CGibShooter, m_flGibLife, FIELD_FLOAT ),
};

IMPLEMENT_SAVERESTORE( CGibShooter, CBaseDelay );
LINK_ENTITY_TO_CLASS( gibshooter, CGibShooter );


void CGibShooter :: Precache ( void )
{
	if ( g_Language == LANGUAGE_GERMAN )
	{
		m_iGibModelIndex = PRECACHE_MODEL ("models/germanygibs.mdl");
	}
	else
	{
		m_iGibModelIndex = PRECACHE_MODEL ("models/hgibs.mdl");
	}
}


void CGibShooter::KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "m_iGibs"))
	{
		m_iGibs = m_iGibCapacity = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_flVelocity"))
	{
		m_flGibVelocity = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_flVariance"))
	{
		m_flVariance = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_flGibLife"))
	{
		m_flGibLife = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
	{
		CBaseDelay::KeyValue( pkvd );
	}
}

void CGibShooter::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	SetThink( &CGibShooter::ShootThink );
	pev->nextthink = gpGlobals->time;
}

void CGibShooter::Spawn( void )
{
	Precache();

	pev->solid = SOLID_NOT;
	pev->effects = EF_NODRAW;

	if ( m_flDelay == 0 )
	{
		m_flDelay = 0.1;
	}

	if ( m_flGibLife == 0 )
	{
		m_flGibLife = 25;
	}

	SetMovedir ( pev );
	pev->body = MODEL_FRAMES( m_iGibModelIndex );
}


CGib *CGibShooter :: CreateGib ( void )
{
	if ( CVAR_GET_FLOAT("violence_hgibs") == 0 )
		return NULL;

	CGib *pGib = GetClassPtr( (CGib *)NULL );
	pGib->Spawn( "models/hgibs.mdl" );
	pGib->m_bloodColor = BLOOD_COLOR_RED;

	if ( pev->body <= 1 )
	{
		ALERT ( at_aiconsole, "GibShooter Body is <= 1!\n" );
	}

	pGib->pev->body = RANDOM_LONG ( 1, pev->body - 1 );// avoid throwing random amounts of the 0th gib. (skull).

	return pGib;
}


void CGibShooter :: ShootThink ( void )
{
	pev->nextthink = gpGlobals->time + m_flDelay;

	Vector vecShootDir;

	vecShootDir = pev->movedir;

	vecShootDir = vecShootDir + gpGlobals->v_right * RANDOM_FLOAT( -1, 1) * m_flVariance;;
	vecShootDir = vecShootDir + gpGlobals->v_forward * RANDOM_FLOAT( -1, 1) * m_flVariance;;
	vecShootDir = vecShootDir + gpGlobals->v_up * RANDOM_FLOAT( -1, 1) * m_flVariance;;

	vecShootDir = vecShootDir.Normalize();
	CGib *pGib = CreateGib();
	
	if ( pGib )
	{
		pGib->pev->origin = pev->origin;
		pGib->pev->velocity = vecShootDir * m_flGibVelocity;
	
		pGib->pev->avelocity.x = RANDOM_FLOAT ( 100, 200 );
		pGib->pev->avelocity.y = RANDOM_FLOAT ( 100, 300 );

		float thinkTime = pGib->pev->nextthink - gpGlobals->time;

		pGib->m_lifeTime = (m_flGibLife * RANDOM_FLOAT( 0.95, 1.05 ));	// +/- 5%
		if ( pGib->m_lifeTime < thinkTime )
		{
			pGib->pev->nextthink = gpGlobals->time + pGib->m_lifeTime;
			pGib->m_lifeTime = 0;
		}
		
	}

	if ( --m_iGibs <= 0 )
	{
		if ( pev->spawnflags & SF_GIBSHOOTER_REPEATABLE )
		{
			m_iGibs = m_iGibCapacity;
			SetThink ( NULL );
			pev->nextthink = gpGlobals->time;
		}
		else
		{
			SetThink ( &CGibShooter::SUB_Remove );
			pev->nextthink = gpGlobals->time;
		}
	}
}


class CEnvShooter : public CGibShooter
{
	void		Precache( void );
	void		KeyValue( KeyValueData *pkvd );

	CGib		*CreateGib( void );
};

LINK_ENTITY_TO_CLASS( env_shooter, CEnvShooter );

void CEnvShooter :: KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "shootmodel"))
	{
		pev->model = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "shootsounds"))
	{
		int iNoise = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
		switch( iNoise )
		{
		case 0:
			m_iGibMaterial = matGlass;
			break;
		case 1:
			m_iGibMaterial = matWood;
			break;
		case 2:
			m_iGibMaterial = matMetal;
			break;
		case 3:
			m_iGibMaterial = matFlesh;
			break;
		case 4:
			m_iGibMaterial = matRocks;
			break;
		
		default:
		case -1:
			m_iGibMaterial = matNone;
			break;
		}
	}
	else
	{
		CGibShooter::KeyValue( pkvd );
	}
}


void CEnvShooter :: Precache ( void )
{
	m_iGibModelIndex = PRECACHE_MODEL( (char *)STRING(pev->model) );
	CBreakable::MaterialSoundPrecache( (Materials)m_iGibMaterial );
}


CGib *CEnvShooter :: CreateGib ( void )
{
	CGib *pGib = GetClassPtr( (CGib *)NULL );

	pGib->Spawn( STRING(pev->model) );
	
	int bodyPart = 0;

	if ( pev->body > 1 )
		bodyPart = RANDOM_LONG( 0, pev->body-1 );

	pGib->pev->body = bodyPart;
	pGib->m_bloodColor = DONT_BLEED;
	pGib->m_material = m_iGibMaterial;

	pGib->pev->rendermode = pev->rendermode;
	pGib->pev->renderamt = pev->renderamt;
	pGib->pev->rendercolor = pev->rendercolor;
	pGib->pev->renderfx = pev->renderfx;
	pGib->pev->scale = pev->scale;
	pGib->pev->skin = pev->skin;

	return pGib;
}




class CTestEffect : public CBaseDelay
{
public:
	void	Spawn( void );
	void	Precache( void );
	// void	KeyValue( KeyValueData *pkvd );
	void EXPORT TestThink( void );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	int		m_iLoop;
	int		m_iBeam;
	CBeam	*m_pBeam[24];
	float	m_flBeamTime[24];
	float	m_flStartTime;
};


LINK_ENTITY_TO_CLASS( test_effect, CTestEffect );

void CTestEffect::Spawn( void )
{
	Precache( );
}

void CTestEffect::Precache( void )
{
	PRECACHE_MODEL( "sprites/lgtning.spr" );
}

void CTestEffect::TestThink( void )
{
	int i;
	float t = (gpGlobals->time - m_flStartTime);

	if (m_iBeam < 24)
	{
		CBeam *pbeam = CBeam::BeamCreate( "sprites/lgtning.spr", 100 );

		TraceResult		tr;

		Vector vecSrc = pev->origin;
		Vector vecDir = Vector( RANDOM_FLOAT( -1.0, 1.0 ), RANDOM_FLOAT( -1.0, 1.0 ),RANDOM_FLOAT( -1.0, 1.0 ) );
		vecDir = vecDir.Normalize();
		UTIL_TraceLine( vecSrc, vecSrc + vecDir * 128, ignore_monsters, ENT(pev), &tr);

		pbeam->PointsInit( vecSrc, tr.vecEndPos );
		// pbeam->SetColor( 80, 100, 255 );
		pbeam->SetColor( 255, 180, 100 );
		pbeam->SetWidth( 100 );
		pbeam->SetScrollRate( 12 );
		
		m_flBeamTime[m_iBeam] = gpGlobals->time;
		m_pBeam[m_iBeam] = pbeam;
		m_iBeam++;

#if 0
		Vector vecMid = (vecSrc + tr.vecEndPos) * 0.5;
		MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
			WRITE_BYTE(TE_DLIGHT);
			WRITE_COORD(vecMid.x);	// X
			WRITE_COORD(vecMid.y);	// Y
			WRITE_COORD(vecMid.z);	// Z
			WRITE_BYTE( 20 );		// radius * 0.1
			WRITE_BYTE( 255 );		// r
			WRITE_BYTE( 180 );		// g
			WRITE_BYTE( 100 );		// b
			WRITE_BYTE( 20 );		// time * 10
			WRITE_BYTE( 0 );		// decay * 0.1
		MESSAGE_END( );
#endif
	}

	if (t < 3.0)
	{
		for (i = 0; i < m_iBeam; i++)
		{
			t = (gpGlobals->time - m_flBeamTime[i]) / ( 3 + m_flStartTime - m_flBeamTime[i]);
			m_pBeam[i]->SetBrightness( 255 * t );
			// m_pBeam[i]->SetScrollRate( 20 * t );
		}
		pev->nextthink = gpGlobals->time + 0.1;
	}
	else
	{
		for (i = 0; i < m_iBeam; i++)
		{
			UTIL_Remove( m_pBeam[i] );
		}
		m_flStartTime = gpGlobals->time;
		m_iBeam = 0;
		// pev->nextthink = gpGlobals->time;
		SetThink( NULL );
	}
}


void CTestEffect::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	SetThink( &CTestEffect::TestThink );
	pev->nextthink = gpGlobals->time + 0.1;
	m_flStartTime = gpGlobals->time;
}



// Blood effects
class CBlood : public CPointEntity
{
public:
	void	Spawn( void );
	void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void	KeyValue( KeyValueData *pkvd );

	inline	int		Color( void ) { return pev->impulse; }
	inline	float 	BloodAmount( void ) { return pev->dmg; }

	inline	void SetColor( int color ) { pev->impulse = color; }
	inline	void SetBloodAmount( float amount ) { pev->dmg = amount; }
	
	Vector	Direction( void );
	Vector	BloodPosition( CBaseEntity *pActivator );

private:
};

LINK_ENTITY_TO_CLASS( env_blood, CBlood );



#define SF_BLOOD_RANDOM		0x0001
#define SF_BLOOD_STREAM		0x0002
#define SF_BLOOD_PLAYER		0x0004
#define SF_BLOOD_DECAL		0x0008

void CBlood::Spawn( void )
{
	pev->solid			= SOLID_NOT;
	pev->movetype		= MOVETYPE_NONE;
	pev->effects		= 0;
	pev->frame			= 0;
	SetMovedir( pev );
}


void CBlood::KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "color"))
	{
		int color = atoi(pkvd->szValue);
		switch( color )
		{
		case 1:
			SetColor( BLOOD_COLOR_YELLOW );
			break;
		default:
			SetColor( BLOOD_COLOR_RED );
			break;
		}

		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "amount"))
	{
		SetBloodAmount( atof(pkvd->szValue) );
		pkvd->fHandled = TRUE;
	}
	else
		CPointEntity::KeyValue( pkvd );
}


Vector CBlood::Direction( void )
{
	if ( pev->spawnflags & SF_BLOOD_RANDOM )
		return UTIL_RandomBloodVector();
	
	return pev->movedir;
}


Vector CBlood::BloodPosition( CBaseEntity *pActivator )
{
	if ( pev->spawnflags & SF_BLOOD_PLAYER )
	{
		edict_t *pPlayer;

		if ( pActivator && pActivator->IsPlayer() )
		{
			pPlayer = pActivator->edict();
		}
		else
			pPlayer = g_engfuncs.pfnPEntityOfEntIndex( 1 );
		if ( pPlayer )
			return (pPlayer->v.origin + pPlayer->v.view_ofs) + Vector( RANDOM_FLOAT(-10,10), RANDOM_FLOAT(-10,10), RANDOM_FLOAT(-10,10) );
	}

	return pev->origin;
}


void CBlood::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( pev->spawnflags & SF_BLOOD_STREAM )
		UTIL_BloodStream( BloodPosition(pActivator), Direction(), (Color() == BLOOD_COLOR_RED) ? 70 : Color(), BloodAmount() );
	else
		UTIL_BloodDrips( BloodPosition(pActivator), Direction(), Color(), BloodAmount() );

	if ( pev->spawnflags & SF_BLOOD_DECAL )
	{
		Vector forward = Direction();
		Vector start = BloodPosition( pActivator );
		TraceResult tr;

		UTIL_TraceLine( start, start + forward * BloodAmount() * 2, ignore_monsters, NULL, &tr );
		if ( tr.flFraction != 1.0 )
			UTIL_BloodDecalTrace( &tr, Color() );
	}
}



// Screen shake
class CShake : public CPointEntity
{
public:
	void	Spawn( void );
	void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void	KeyValue( KeyValueData *pkvd );

	inline	float	Amplitude( void ) { return pev->scale; }
	inline	float	Frequency( void ) { return pev->dmg_save; }
	inline	float	Duration( void ) { return pev->dmg_take; }
	inline	float	Radius( void ) { return pev->dmg; }

	inline	void	SetAmplitude( float amplitude ) { pev->scale = amplitude; }
	inline	void	SetFrequency( float frequency ) { pev->dmg_save = frequency; }
	inline	void	SetDuration( float duration ) { pev->dmg_take = duration; }
	inline	void	SetRadius( float radius ) { pev->dmg = radius; }
private:
};

LINK_ENTITY_TO_CLASS( env_shake, CShake );

// pev->scale is amplitude
// pev->dmg_save is frequency
// pev->dmg_take is duration
// pev->dmg is radius
// radius of 0 means all players
// NOTE: UTIL_ScreenShake() will only shake players who are on the ground

#define SF_SHAKE_EVERYONE	0x0001		// Don't check radius
// UNDONE: These don't work yet
#define SF_SHAKE_DISRUPT	0x0002		// Disrupt controls
#define SF_SHAKE_INAIR		0x0004		// Shake players in air

void CShake::Spawn( void )
{
	pev->solid			= SOLID_NOT;
	pev->movetype		= MOVETYPE_NONE;
	pev->effects		= 0;
	pev->frame			= 0;
	
	if ( pev->spawnflags & SF_SHAKE_EVERYONE )
		pev->dmg = 0;
}


void CShake::KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "amplitude"))
	{
		SetAmplitude( atof(pkvd->szValue) );
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "frequency"))
	{
		SetFrequency( atof(pkvd->szValue) );
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "duration"))
	{
		SetDuration( atof(pkvd->szValue) );
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "radius"))
	{
		SetRadius( atof(pkvd->szValue) );
		pkvd->fHandled = TRUE;
	}
	else
		CPointEntity::KeyValue( pkvd );
}


void CShake::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	UTIL_ScreenShake( pev->origin, Amplitude(), Frequency(), Duration(), Radius() );
}


class CFade : public CPointEntity
{
public:
	void	Spawn( void );
	void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void	KeyValue( KeyValueData *pkvd );

	inline	float	Duration( void ) { return pev->dmg_take; }
	inline	float	HoldTime( void ) { return pev->dmg_save; }

	inline	void	SetDuration( float duration ) { pev->dmg_take = duration; }
	inline	void	SetHoldTime( float hold ) { pev->dmg_save = hold; }
private:
};

LINK_ENTITY_TO_CLASS( env_fade, CFade );

// pev->dmg_take is duration
// pev->dmg_save is hold duration
#define SF_FADE_IN				0x0001		// Fade in, not out
#define SF_FADE_MODULATE		0x0002		// Modulate, don't blend
#define SF_FADE_ONLYONE			0x0004

void CFade::Spawn( void )
{
	pev->solid			= SOLID_NOT;
	pev->movetype		= MOVETYPE_NONE;
	pev->effects		= 0;
	pev->frame			= 0;
}


void CFade::KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "duration"))
	{
		SetDuration( atof(pkvd->szValue) );
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "holdtime"))
	{
		SetHoldTime( atof(pkvd->szValue) );
		pkvd->fHandled = TRUE;
	}
	else
		CPointEntity::KeyValue( pkvd );
}


void CFade::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	int fadeFlags = 0;
	
	if ( !(pev->spawnflags & SF_FADE_IN) )
		fadeFlags |= FFADE_OUT;

	if ( pev->spawnflags & SF_FADE_MODULATE )
		fadeFlags |= FFADE_MODULATE;

	if ( pev->spawnflags & SF_FADE_ONLYONE )
	{
		if ( pActivator->IsNetClient() )
		{
			UTIL_ScreenFade( pActivator, pev->rendercolor, Duration(), HoldTime(), pev->renderamt, fadeFlags );
		}
	}
	else
	{
		UTIL_ScreenFadeAll( pev->rendercolor, Duration(), HoldTime(), pev->renderamt, fadeFlags );
	}
	SUB_UseTargets( this, USE_TOGGLE, 0 );
}


class CMessage : public CPointEntity
{
public:
	void	Spawn( void );
	void	Precache( void );
	void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void	KeyValue( KeyValueData *pkvd );
private:
};

LINK_ENTITY_TO_CLASS( env_message, CMessage );


void CMessage::Spawn( void )
{
	Precache();

	pev->solid			= SOLID_NOT;
	pev->movetype		= MOVETYPE_NONE;

	switch( pev->impulse )
	{
	case 1: // Medium radius
		pev->speed = ATTN_STATIC;
		break;
	
	case 2:	// Large radius
		pev->speed = ATTN_NORM;
		break;

	case 3:	//EVERYWHERE
		pev->speed = ATTN_NONE;
		break;
	
	default:
	case 0: // Small radius
		pev->speed = ATTN_IDLE;
		break;
	}
	pev->impulse = 0;

	// No volume, use normal
	if ( pev->scale <= 0 )
		pev->scale = 1.0;
}


void CMessage::Precache( void )
{
	if ( pev->noise )
		PRECACHE_SOUND( (char *)STRING(pev->noise) );
}

void CMessage::KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "messagesound"))
	{
		pev->noise = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "messagevolume"))
	{
		pev->scale = atof(pkvd->szValue) * 0.1;
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "messageattenuation"))
	{
		pev->impulse = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CPointEntity::KeyValue( pkvd );
}


void CMessage::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	CBaseEntity *pPlayer = NULL;

	if ( pev->spawnflags & SF_MESSAGE_ALL )
		UTIL_ShowMessageAll( STRING(pev->message) );
	else
	{
		if ( pActivator && pActivator->IsPlayer() )
			pPlayer = pActivator;
		else
		{
			pPlayer = CBaseEntity::Instance( g_engfuncs.pfnPEntityOfEntIndex( 1 ) );
		}
		if ( pPlayer )
			UTIL_ShowMessage( STRING(pev->message), pPlayer );
	}
	if ( pev->noise )
	{
		EMIT_SOUND( edict(), CHAN_BODY, STRING(pev->noise), pev->scale, pev->speed );
	}
	if ( pev->spawnflags & SF_MESSAGE_ONCE )
		UTIL_Remove( this );

	SUB_UseTargets( this, USE_TOGGLE, 0 );
}



//=========================================================
// FunnelEffect
//=========================================================
class CEnvFunnel : public CBaseDelay
{
public:
	void	Spawn( void );
	void	Precache( void );
	void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	int		m_iSprite;	// Don't save, precache
};

void CEnvFunnel :: Precache ( void )
{
	m_iSprite = PRECACHE_MODEL ( "sprites/flare6.spr" );
}

LINK_ENTITY_TO_CLASS( env_funnel, CEnvFunnel );

void CEnvFunnel::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_LARGEFUNNEL );
		WRITE_COORD( pev->origin.x );
		WRITE_COORD( pev->origin.y );
		WRITE_COORD( pev->origin.z );
		WRITE_SHORT( m_iSprite );

		if ( pev->spawnflags & SF_FUNNEL_REVERSE )// funnel flows in reverse?
		{
			WRITE_SHORT( 1 );
		}
		else
		{
			WRITE_SHORT( 0 );
		}


	MESSAGE_END();

	SetThink( &CEnvFunnel::SUB_Remove );
	pev->nextthink = gpGlobals->time;
}

void CEnvFunnel::Spawn( void )
{
	Precache();
	pev->solid = SOLID_NOT;
	pev->effects = EF_NODRAW;
}

//=========================================================
// Beverage Dispenser
// overloaded pev->frags, is now a flag for whether or not a can is stuck in the dispenser. 
// overloaded pev->health, is now how many cans remain in the machine.
//=========================================================
class CEnvBeverage : public CBaseDelay
{
public:
	void	Spawn( void );
	void	Precache( void );
	void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
};

void CEnvBeverage :: Precache ( void )
{
	PRECACHE_MODEL( "models/can.mdl" );
	PRECACHE_SOUND( "weapons/g_bounce3.wav" );
}

LINK_ENTITY_TO_CLASS( env_beverage, CEnvBeverage );

void CEnvBeverage::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( pev->frags != 0 || pev->health <= 0 )
	{
		// no more cans while one is waiting in the dispenser, or if I'm out of cans.
		return;
	}

	CBaseEntity *pCan = CBaseEntity::Create( "item_sodacan", pev->origin, pev->angles, edict() );	

	if ( pev->skin == 6 )
	{
		// random
		pCan->pev->skin = RANDOM_LONG( 0, 5 );
	}
	else
	{
		pCan->pev->skin = pev->skin;
	}

	pev->frags = 1;
	pev->health--;

	//SetThink (SUB_Remove);
	//pev->nextthink = gpGlobals->time;
}

void CEnvBeverage::Spawn( void )
{
	Precache();
	pev->solid = SOLID_NOT;
	pev->effects = EF_NODRAW;
	pev->frags = 0;

	if ( pev->health == 0 )
	{
		pev->health = 10;
	}
}

//=========================================================
// Soda can
//=========================================================
class CItemSoda : public CBaseEntity
{
public:
	void	Spawn( void );
	void	Precache( void );
	void	EXPORT CanThink ( void );
	void	EXPORT CanTouch ( CBaseEntity *pOther );
};

void CItemSoda :: Precache ( void )
{
}

LINK_ENTITY_TO_CLASS( item_sodacan, CItemSoda );

void CItemSoda::Spawn( void )
{
	Precache();
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_TOSS;

	SET_MODEL ( ENT(pev), "models/can.mdl" );
	UTIL_SetSize ( pev, Vector ( 0, 0, 0 ), Vector ( 0, 0, 0 ) );
	
	SetThink (&CItemSoda::CanThink);
	pev->nextthink = gpGlobals->time + 0.5;
}

void CItemSoda::CanThink ( void )
{
	EMIT_SOUND (ENT(pev), CHAN_WEAPON, "weapons/g_bounce3.wav", 1, ATTN_NORM );

	pev->solid = SOLID_TRIGGER;
	UTIL_SetSize ( pev, Vector ( -8, -8, 0 ), Vector ( 8, 8, 8 ) );
	SetThink ( NULL );
	SetTouch ( &CItemSoda::CanTouch );
}

void CItemSoda::CanTouch ( CBaseEntity *pOther )
{
	if ( !pOther->IsPlayer() )
	{
		return;
	}

	// spoit sound here

	pOther->TakeHealth( 1, DMG_GENERIC );// a bit of health.

	if ( !FNullEnt( pev->owner ) )
	{
		// tell the machine the can was taken
		pev->owner->v.frags = 0;
	}

	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	pev->effects = EF_NODRAW;
	SetTouch ( NULL );
	SetThink ( &CItemSoda::SUB_Remove );
	pev->nextthink = gpGlobals->time;
}


// buz: dynamic light entity
#define SF_DYNLIGHT_STARTOFF 1

class CDynamicLight : public CPointEntity
{
public:
	void Spawn()
	{
		pev->solid = SOLID_NOT;
		pev->movetype = MOVETYPE_NONE;
		pev->renderfx = 71; // dynamic light

		PRECACHE_MODEL("sprites/null.spr");
		SET_MODEL(ENT(pev), "sprites/null.spr"); // should be visible to send to client
		UTIL_SetSize(pev, g_vecZero, g_vecZero);

		if (pev->spawnflags & SF_DYNLIGHT_STARTOFF)
			pev->effects |= EF_NODRAW;

		pev->renderamt = pev->renderamt / 8;
	}

	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
	{
		//	if (!ShouldToggle( useType ))
		//		return;

		if (useType == USE_ON)
			pev->effects &= ~EF_NODRAW;
		else if (useType == USE_OFF)
			pev->effects |= EF_NODRAW;
		else if (useType == USE_TOGGLE)
		{
			if (pev->effects & EF_NODRAW)
				pev->effects &= ~EF_NODRAW;
			else
				pev->effects |= EF_NODRAW;
		}
	}
};

LINK_ENTITY_TO_CLASS(env_spotlight, CDynamicLight);

class CCubicMaps : public CPointEntity
{
public:
	void Spawn()
	{
		pev->solid = SOLID_NOT;
		pev->movetype = MOVETYPE_NONE;
		pev->renderfx = 77; //cubemap?

		PRECACHE_MODEL("sprites/null.spr");
		SET_MODEL(ENT(pev), "sprites/null.spr"); // should be visible to send to client
		UTIL_SetSize(pev, g_vecZero, g_vecZero);

		int cubeNum = pev->origin.x + pev->origin.y + pev->origin.z;
		cubeNum = abs(cubeNum);

		ALERT(at_console, "CCubicMaps Spawn\n");

	} 

	void KeyValue(KeyValueData *pkvd)
	{
		CPointEntity::KeyValue(pkvd);

		int cubeNum = pev->origin.x + pev->origin.y + pev->origin.z;
		cubeNum = abs(cubeNum);
		pev->sequence = cubeNum;
	}
};

LINK_ENTITY_TO_CLASS(env_cubemap, CCubicMaps);

//=================================================================
// env_model: like env_sprite, except you can specify a sequence.
//=================================================================
#define SF_ENVMODEL_OFF			1
#define SF_ENVMODEL_DROPTOFLOOR	2
#define SF_ENVMODEL_SOLID		4

#define SF_DONTUSELOD			8

#define SF_USELOD_HIGH			16
#define SF_USELOD_LOW			32
#define SF_DONTCHECK_FOV		64

//#define SF_CREATELIGHT			128
//#define SF_USE_CUSTOM_DISTANCE 128//not used

#define SF_USE_REALLOD			256

#define SF_BREAKEABLE			128

#define SF_COLLISION			512

#define SF_RANDOM_Y			1024

//#define SF_ONSIGHT_STATECHECK	0x00004 //AJH

#define BODY_GROUP				0
#define HEAD_GROUP				1
#define OTHER_GROUP				2

class CEnvModel : public CBaseAnimating
{

public://move it to public, so that it can be access by another class
	virtual void Spawn(void);
	void Precache(void);
	void EXPORT Think(void);
	void KeyValue(KeyValueData *pkvd);
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	virtual int	ObjectCaps(void) { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	virtual int		Save(CSave &save);
	virtual int		Restore(CRestore &restore);
	static	TYPEDESCRIPTION m_SaveData[];

	void SetSequence(void);

	string_t m_iszSequence_On;
	string_t m_iszSequence_Off;
	int m_iAction_On;
	int m_iAction_Off;

	//SysOp- New
	BOOL VisionCheck(void);
	BOOL CanSee(CBaseEntity *pLooker, CBaseEntity *pSeen);

	BOOL m_CeIsRope;
	BOOL m_bDoRealLOD;
	int m_iMaxRenderingDistance;

	float	m_fNextBushSound;
	bool	m_bPlayerIsTouchingProp;

	char* str1;
	char* str2;
	/*const*/ char* result;
	BOOL bHL2ModelFound;
	BOOL bNoModel;




	void DamageSound(void);

	// breakables use an overridden takedamage
	virtual int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType);
	// To spark when hit
	void TraceAttack(entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType);

	BOOL IsBreakable(void);
	BOOL SparkWhenHit(void);

	void EXPORT		Die(void);

	inline BOOL		Explodable(void) { return ExplosionMagnitude() > 0; }
	inline int		ExplosionMagnitude(void) { return pev->impulse; }
	inline void		ExplosionSetMagnitude(int magnitude) { pev->impulse = magnitude; }

	static void MaterialSoundPrecache(Materials precacheMaterial);
	static void MaterialSoundRandom(edict_t *pEdict, Materials soundMaterial, float volume);
	static const char **MaterialSoundList(Materials precacheMaterial, int &soundCount);

	static const char *pSoundsWood[];
	static const char *pSoundsFlesh[];
	static const char *pSoundsGlass[];
	static const char *pSoundsMetal[];
	static const char *pSoundsConcrete[];
	static const char *pSpawnObjects[];

	Materials	m_Material;
	Explosions	m_Explosion;
	int			m_idShard;
	float		m_angle;
	int			m_iszGibModel;
	int			m_iszSpawnObject;
};

TYPEDESCRIPTION CEnvModel::m_SaveData[] =
{
	DEFINE_FIELD(CEnvModel, m_iszSequence_On, FIELD_STRING),
	DEFINE_FIELD(CEnvModel, m_iszSequence_Off, FIELD_STRING),
	DEFINE_FIELD(CEnvModel, m_iAction_On, FIELD_INTEGER),
	DEFINE_FIELD(CEnvModel, m_iAction_Off, FIELD_INTEGER),
	DEFINE_FIELD(CEnvModel, m_CeIsRope, FIELD_BOOLEAN),
	DEFINE_FIELD(CEnvModel, m_bDoRealLOD, FIELD_BOOLEAN),
	DEFINE_FIELD(CEnvModel, m_iMaxRenderingDistance, FIELD_INTEGER),

	DEFINE_FIELD(CEnvModel, m_Material, FIELD_INTEGER),
	DEFINE_FIELD(CEnvModel, m_Explosion, FIELD_INTEGER),
	DEFINE_FIELD(CEnvModel, m_angle, FIELD_FLOAT),
	DEFINE_FIELD(CEnvModel, m_iszGibModel, FIELD_STRING),
	DEFINE_FIELD(CEnvModel, m_iszSpawnObject, FIELD_STRING),
};

IMPLEMENT_SAVERESTORE(CEnvModel, CBaseAnimating);

LINK_ENTITY_TO_CLASS(env_model, CEnvModel);
LINK_ENTITY_TO_CLASS(cine_camera, CEnvModel);


LINK_ENTITY_TO_CLASS(env_custom_model, CEnvModel);//new, for the custom options

LINK_ENTITY_TO_CLASS(mi_model_static, CEnvModel);//for movein maps
LINK_ENTITY_TO_CLASS(mi_model, CEnvModel);//for movein maps

LINK_ENTITY_TO_CLASS(prop_palm01, CEnvModel);//new, for the custom options
LINK_ENTITY_TO_CLASS(prop_palm02, CEnvModel);//for movein maps
LINK_ENTITY_TO_CLASS(prop_bush01, CEnvModel);//for movein maps
LINK_ENTITY_TO_CLASS(prop_bush02, CEnvModel);//for movein maps
LINK_ENTITY_TO_CLASS(prop_fern01, CEnvModel);//for movein maps
LINK_ENTITY_TO_CLASS(prop_palmC01, CEnvModel);//new, for the custom options

//pipes
LINK_ENTITY_TO_CLASS(prop_pipe, CEnvModel);//custom model
LINK_ENTITY_TO_CLASS(prop_pipe_long, CEnvModel);//custom model
LINK_ENTITY_TO_CLASS(prop_pipeT, CEnvModel);//custom model

const char *CEnvModel::pSpawnObjects[] =
{
	NULL,				// 0
	"item_battery",		// 1
	"item_healthkit",	// 2
	"weapon_1911",		// 3
	"ammo_45cal",		// 4
	"weapon_mp5",		// 5
	"ammo_mp5clip",		// 6
	"ammo_ARgrenades",	// 7
	"weapon_spas12",	// 8
	"ammo_buckshot",	// 9
	"weapon_m14",		// 10
	"ammo_m14clip",		// 11
	"weapon_anaconda",	// 12
	"ammo_44",			// 13
	"weapon_rpg",		// 14
	"ammo_rpgclip",		// 15
	"ammo_famasclip",	// 16
	"item_grenade",		// 17
	"weapon_tripmine",	// 18
	"weapon_semtex",	// 19
	"weapon_usas",		// 20
	"weapon_ak47",		// 21
};

void CEnvModel::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "m_iszSequence_On"))
	{
		m_iszSequence_On = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszSequence_Off"))
	{
		m_iszSequence_Off = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iAction_On"))
	{
		m_iAction_On = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iAction_Off"))
	{
		m_iAction_Off = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iMax_rendering"))
	{
		m_iMaxRenderingDistance = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	//BREAK START
	else if (FStrEq(pkvd->szKeyName, "explosion"))
	{
		if (!stricmp(pkvd->szValue, "directed"))
			m_Explosion = expDirected;
		else if (!stricmp(pkvd->szValue, "random"))
			m_Explosion = expRandom;
		else
			m_Explosion = expRandom;

		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "material"))
	{
		int i = atoi(pkvd->szValue);

		// 0:glass, 1:metal, 2:flesh, 3:wood

		if ((i < 0) || (i >= matLastMaterial))
			m_Material = matWood;
		else
			m_Material = (Materials)i;

		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "deadmodel"))
	{
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "shards"))
	{
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "gibmodel"))
	{
		m_iszGibModel = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "spawnobject"))
	{
		int object = atoi(pkvd->szValue);
		if (object > 0 && object < ARRAYSIZE(pSpawnObjects))
			m_iszSpawnObject = MAKE_STRING(pSpawnObjects[object]);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "explodemagnitude"))
	{
		ExplosionSetMagnitude(atoi(pkvd->szValue));
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "lip"))
		pkvd->fHandled = TRUE;
	else
	{
		CBaseAnimating::KeyValue(pkvd);
	}
}

void CEnvModel::Spawn(void)
{
	Precache();

	if (pev->health == 0)
		pev->health = 100; //creo q algo de 100 o 70...

	if (FClassnameIs(pev, "cine_camera"))
		SET_MODEL(ENT(pev), "models/npccamera_test.mdl");


	if (FClassnameIs(pev, "prop_cable") || FClassnameIs(pev, "prop_cable_small") || FClassnameIs(pev, "prop_bugs"))
		SET_MODEL(ENT(pev), STRING(pev->model));
	else
	{
		if (bHL2ModelFound || bNoModel)
		{
			ALERT(at_console, "Spawn() SET_MODEL: 'models/PROPS/ERROR.MDL'\n");
			SET_MODEL(ENT(pev), "models/PROPS/ERROR.MDL");
		}
		else
		{
			ALERT(at_console, "'%s' loaded sucessfully\n", result);
			SET_MODEL(ENT(pev), result);
		}
	}

	UTIL_SetOrigin(pev, pev->origin);
	m_angle = pev->angles.y;//moved up because of solidify problems

	if (FClassnameIs(pev, "prop_palm01") || FClassnameIs(pev, "prop_palm02") || FClassnameIs(pev, "prop_palmC01") || FClassnameIs(pev, "prop_fern01") || FClassnameIs(pev, "prop_bush01") || FClassnameIs(pev, "prop_bush02"))
		pev->solid = SOLID_SLIDEBOX;

	if (pev->spawnflags & SF_ENVMODEL_DROPTOFLOOR)
	{
		pev->origin.z += 1;
		DROP_TO_FLOOR(ENT(pev));
	}

	SetBoneController(0, 0);
	SetBoneController(1, 0);
	SetBoneController(2, 0);
	SetBoneController(3, 0);

	SetSequence();

	if (pev->spawnflags & SF_USE_REALLOD)
	{
		m_bDoRealLOD = TRUE;

		SetBodygroup(0, 2);//1100 polys
	}

	m_fNextBushSound = 2.0;//wait a sec
	m_bPlayerIsTouchingProp = FALSE;

	if (pev->spawnflags & SF_RANDOM_Y)
		pev->angles.y = RANDOM_LONG(0, 360);

	if (pev->spawnflags & SF_USELOD_HIGH)//demasiado LOD
		m_iMaxRenderingDistance = 256;

	if (pev->spawnflags & SF_USELOD_LOW)//poco LOD
		m_iMaxRenderingDistance = 2048;

	if (FStringNull(m_iMaxRenderingDistance))//max rendering hasn't been set, let's set a default distance
	{
		m_iMaxRenderingDistance = 512;
	}

	if (m_iMaxRenderingDistance < 255)//if it's zero, or the mapper just forgot to set a value...
		m_iMaxRenderingDistance = 255;

	if (m_bDoRealLOD)
		m_iMaxRenderingDistance = 9999;//probabily not the correct way to do this but I'm lazy now.

	if (pev->spawnflags & SF_BREAKEABLE)//if breakeable, actually
	{
		pev->takedamage = DAMAGE_YES;

		pev->solid = SOLID_SLIDEBOX;
		pev->movetype = MOVETYPE_PUSHSTEP;

		SetModelCollisionBox();
	}
	else
	{
		if (pev->spawnflags & SF_ENVMODEL_SOLID)//if breakeable, actually
		{
			pev->solid = SOLID_SLIDEBOX;
			pev->movetype = MOVETYPE_PUSHSTEP;

			SetModelCollisionBox();
		}
		else
		{
			pev->solid = SOLID_NOT;
			pev->takedamage = DAMAGE_NO;
		}
	}

	pev->nextthink = 0.1;
}



BOOL CEnvModel::IsBreakable(void)
{
	return m_Material != matUnbreakableGlass;
}



void CEnvModel::DamageSound(void)
{
	int pitch;
	float fvol;
	char *rgpsz[6];
	int i;
	int material = m_Material;

	//	if (RANDOM_LONG(0,1))
	//		return;

	if (RANDOM_LONG(0, 2))
		pitch = PITCH_NORM;
	else
		pitch = 95 + RANDOM_LONG(0, 34);

	fvol = RANDOM_FLOAT(0.75, 1.0);

	if (material == matComputer && RANDOM_LONG(0, 1))
		material = matMetal;

	switch (material)
	{
	case matComputer:
	case matGlass:
	case matUnbreakableGlass:
		rgpsz[0] = "debris/glass1.wav";
		rgpsz[1] = "debris/glass2.wav";
		rgpsz[2] = "debris/glass3.wav";
		i = 3;
		break;

	case matWood:
		rgpsz[0] = "debris/wood1.wav";
		rgpsz[1] = "debris/wood2.wav";
		rgpsz[2] = "debris/wood3.wav";
		i = 3;
		break;

	case matMetal:
		rgpsz[0] = "debris/metal1.wav";
		rgpsz[1] = "debris/metal3.wav";
		rgpsz[2] = "debris/metal2.wav";
		i = 2;
		break;

	case matFlesh:
		rgpsz[0] = "debris/flesh1.wav";
		rgpsz[1] = "debris/flesh2.wav";
		rgpsz[2] = "debris/flesh3.wav";
		rgpsz[3] = "debris/flesh5.wav";
		rgpsz[4] = "debris/flesh6.wav";
		rgpsz[5] = "debris/flesh7.wav";
		i = 6;
		break;

	case matRocks:
	case matCinderBlock:
		rgpsz[0] = "debris/concrete1.wav";
		rgpsz[1] = "debris/concrete2.wav";
		rgpsz[2] = "debris/concrete3.wav";
		i = 3;
		break;

	case matCeilingTile:
		// UNDONE: no ceiling tile shard sound yet
		i = 0;
		break;
	}

	if (i)
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, rgpsz[RANDOM_LONG(0, i - 1)], fvol, ATTN_NORM, 0, pitch);
}

//=========================================================
// Special takedamage for func_breakable. Allows us to make
// exceptions that are breakable-specific
// bitsDamageType indicates the type of damage sustained ie: DMG_CRUSH
//=========================================================
int CEnvModel::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	Vector	vecTemp;

	if (pevAttacker == pevInflictor)
	{
		vecTemp = pevInflictor->origin - (pev->absmin + (pev->size * 0.5));

		// if a client hit the breakable with melee, and breakable is melee-sensitive, break it now.
		// Shepard : Added DMG_SLASH because the knife use this instead of DMG_CLUB
		if (FBitSet(pevAttacker->flags, FL_CLIENT) &&
			FBitSet(pev->spawnflags, SF_BREAK_MELEE) && ((bitsDamageType & DMG_CLUB) || (bitsDamageType & DMG_SLASH)))
			flDamage = pev->health;
	}
	else
	{
		vecTemp = pevInflictor->origin - (pev->absmin + (pev->size * 0.5));
	}

	if (!IsBreakable())
		return 0;

	// Breakables take double damage from melee
	if ( (bitsDamageType & DMG_CLUB) || (bitsDamageType & DMG_SLASH) )
		flDamage *= 2;

	// Boxes / glass / etc. don't take much poison damage, just the impact of the dart - consider that 10%
	if (bitsDamageType & DMG_POISON)
		flDamage *= 0.1;

	// this global is still used for glass and other non-monster killables, along with decals.
	g_vecAttackDir = vecTemp.Normalize();

	// do the damage
	pev->health -= flDamage;

	if (pev->health <= 0)
	{
		Killed(pevAttacker, GIB_NORMAL);

		Die();
		return 0;
	}

	// Make a shard noise each time func breakable is hit.
	// Don't play shard noise if cbreakable actually died.
	DamageSound();

	return 1;
}

void ExplosionCreate(const Vector &center, const Vector &angles, edict_t *pOwner, int magnitude, BOOL doDamage);
void CEnvModel::Die(void)
{
	Vector vecSpot;// shard origin
	Vector vecVelocity;// shard velocity
	CBaseEntity *pEntity = NULL;
	char cFlag = 0;
	int pitch;
	float fvol;

	pitch = 95 + RANDOM_LONG(0, 29);

	if (pitch > 97 && pitch < 103)
		pitch = 100;

	// The more negative pev->health, the louder
	// the sound should be.

	fvol = RANDOM_FLOAT(0.85, 1.0) + (abs(pev->health) / 100.0);

	if (fvol > 1.0)
		fvol = 1.0;

	float fAmount = pev->size.x + pev->size.y + pev->size.z;
	float n;

	switch (m_Material)
	{
	case matGlass:
		switch (RANDOM_LONG(0, 1))
		{
		case 0:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustglass1.wav", fvol, ATTN_NORM, 0, pitch);
			break;
		case 1:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustglass2.wav", fvol, ATTN_NORM, 0, pitch);
			break;
		}
		cFlag = BREAK_GLASS;

		//fill it up

		for (n = 0.0; n <= (fAmount *0.05); n++)
		{
			//hawkfix
	/*		MESSAGE_BEGIN(MSG_ALL, gmsgParticles);
			WRITE_SHORT(0);
			WRITE_BYTE(0);
			WRITE_COORD(Center().x + RANDOM_FLOAT(-pev->size.x / 2, pev->size.x / 2));
			WRITE_COORD(Center().y + RANDOM_FLOAT(-pev->size.y / 2, pev->size.y / 2));
			WRITE_COORD(Center().z + RANDOM_FLOAT(-pev->size.z / 2, pev->size.z / 2));
			WRITE_COORD(0);
			WRITE_COORD(0);
			WRITE_COORD(0);
			WRITE_SHORT(iDefaultGlass);
			MESSAGE_END();*/
		}
		break;

	case matWood:
		switch (RANDOM_LONG(0, 1))
		{
		case 0:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustcrate1.wav", fvol, ATTN_NORM, 0, pitch);
			break;
		case 1:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustcrate2.wav", fvol, ATTN_NORM, 0, pitch);
			break;
		}
		cFlag = BREAK_WOOD;
		break;

	case matComputer:
	case matMetal:
		switch (RANDOM_LONG(0, 1))
		{
		case 0:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustmetal1.wav", fvol, ATTN_NORM, 0, pitch);
			break;
		case 1:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustmetal2.wav", fvol, ATTN_NORM, 0, pitch);
			break;
		}
		cFlag = BREAK_METAL;
		break;

	case matFlesh:
		switch (RANDOM_LONG(0, 1))
		{
		case 0:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustflesh1.wav", fvol, ATTN_NORM, 0, pitch);
			break;
		case 1:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustflesh2.wav", fvol, ATTN_NORM, 0, pitch);
			break;
		}
		cFlag = BREAK_FLESH;
		break;

	case matRocks:
	case matCinderBlock:
		switch (RANDOM_LONG(0, 1))
		{
		case 0:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustconcrete1.wav", fvol, ATTN_NORM, 0, pitch);
			break;
		case 1:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustconcrete2.wav", fvol, ATTN_NORM, 0, pitch);
			break;
		}
		cFlag = BREAK_CONCRETE;
		break;

	case matCeilingTile:
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustceiling.wav", fvol, ATTN_NORM, 0, pitch);
		break;
	}


	//of course it's directed!
	//	if (m_Explosion == expDirected)
	g_vecAttackDir = g_vecAttackDir *-1;

	vecVelocity = g_vecAttackDir * 200;

	vecSpot = pev->origin + (pev->mins + pev->maxs) * 0.5;

	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, vecSpot);
	WRITE_BYTE(TE_BREAKMODEL);

	// position
	WRITE_COORD(vecSpot.x);
	WRITE_COORD(vecSpot.y);
	WRITE_COORD(vecSpot.z);

	// size
	WRITE_COORD(pev->size.x);
	WRITE_COORD(pev->size.y);
	WRITE_COORD(pev->size.z);

	// velocity
	WRITE_COORD(vecVelocity.x);
	WRITE_COORD(vecVelocity.y);
	WRITE_COORD(vecVelocity.z);

	// randomization
	WRITE_BYTE(10);

	// Model
	WRITE_SHORT(m_idShard);	//model id#

	// # of shards
	WRITE_BYTE(0);	// let client decide

	// duration
	WRITE_BYTE(25);// 2.5 seconds

	// flags
	WRITE_BYTE(cFlag);
	MESSAGE_END();

	float size = pev->size.x;
	if (size < pev->size.y)
		size = pev->size.y;
	if (size < pev->size.z)
		size = pev->size.z;

	// !!! HACK  This should work!
	// Build a box above the entity that looks like an 8 pixel high sheet
	Vector mins = pev->absmin;
	Vector maxs = pev->absmax;
	mins.z = pev->absmax.z;
	maxs.z += 8;

	// BUGBUG -- can only find 256 entities on a breakable -- should be enough
	CBaseEntity *pList[256];
	int count = UTIL_EntitiesInBox(pList, 256, mins, maxs, FL_ONGROUND);
	if (count)
	{
		for (int i = 0; i < count; i++)
		{
			ClearBits(pList[i]->pev->flags, FL_ONGROUND);
			pList[i]->pev->groundentity = NULL;
		}
	}

	// Don't fire something that could fire myself
	pev->targetname = 0;
	pev->solid = SOLID_NOT;

	// Fire targets on break
	SUB_UseTargets(NULL, USE_TOGGLE, 0);

	SetThink(&CBaseEntity::SUB_Remove);
	pev->nextthink = pev->ltime + 0.1;

	if (m_iszSpawnObject)
		CBaseEntity::Create((char *)STRING(m_iszSpawnObject), VecBModelOrigin(pev), pev->angles, edict());

	if (Explodable())
	{
		ExplosionCreate(Center(), pev->angles, edict(), ExplosionMagnitude(), TRUE);
	}
}

void CEnvModel::TraceAttack(entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType)
{
	// random spark if this is a 'computer' object
	if (RANDOM_LONG(0, 1))
	{
		switch (m_Material)
		{
		case matComputer:
		{
							UTIL_Sparks(ptr->vecEndPos);

							float flVolume = RANDOM_FLOAT(0.7, 1.0);//random volume range
							switch (RANDOM_LONG(0, 1))
							{
							case 0: EMIT_SOUND(ENT(pev), CHAN_VOICE, "buttons/spark5.wav", flVolume, ATTN_NORM);	break;
							case 1: EMIT_SOUND(ENT(pev), CHAN_VOICE, "buttons/spark6.wav", flVolume, ATTN_NORM);	break;
							}
		}
			break;

		case matUnbreakableGlass:
			UTIL_Ricochet(ptr->vecEndPos, RANDOM_FLOAT(0.5, 1.5));
			break;
		}
	}

	CBaseDelay::TraceAttack(pevAttacker, flDamage, vecDir, ptr, bitsDamageType);
}


void CEnvModel::Think(void)
{
	int iTemp;

	StudioFrameAdvance(); // set m_fSequenceFinished if necessary

	if (m_fSequenceFinished && !m_fSequenceLoops)
	{
		if (pev->spawnflags & SF_ENVMODEL_OFF)
			iTemp = m_iAction_Off;
		else
			iTemp = m_iAction_On;

		switch (iTemp)
		{
		case 2: // change state
			if (pev->spawnflags & SF_ENVMODEL_OFF)
				pev->spawnflags &= ~SF_ENVMODEL_OFF;
			else
				pev->spawnflags |= SF_ENVMODEL_OFF;
			SetSequence();
			break;
		default: //remain frozen
			return;
		}
	}

	pev->nextthink = 0.5;//0.1
}


const char *CEnvModel::pSoundsWood[] =
{
	"debris/wood1.wav",
	"debris/wood2.wav",
	"debris/wood3.wav",
};

const char *CEnvModel::pSoundsFlesh[] =
{
	"debris/flesh1.wav",
	"debris/flesh2.wav",
	"debris/flesh3.wav",
	"debris/flesh5.wav",
	"debris/flesh6.wav",
	"debris/flesh7.wav",
};

const char *CEnvModel::pSoundsMetal[] =
{
	"debris/metal1.wav",
	"debris/metal2.wav",
	"debris/metal3.wav",
};

const char *CEnvModel::pSoundsConcrete[] =
{
	"debris/concrete1.wav",
	"debris/concrete2.wav",
	"debris/concrete3.wav",
};


const char *CEnvModel::pSoundsGlass[] =
{
	"debris/glass1.wav",
	"debris/glass2.wav",
	"debris/glass3.wav",
};

const char **CEnvModel::MaterialSoundList(Materials precacheMaterial, int &soundCount)
{
	const char	**pSoundList = NULL;

	switch (precacheMaterial)
	{
	case matWood:
		pSoundList = pSoundsWood;
		soundCount = ARRAYSIZE(pSoundsWood);
		break;
	case matFlesh:
		pSoundList = pSoundsFlesh;
		soundCount = ARRAYSIZE(pSoundsFlesh);
		break;
	case matComputer:
	case matUnbreakableGlass:
	case matGlass:
		pSoundList = pSoundsGlass;
		soundCount = ARRAYSIZE(pSoundsGlass);
		break;

	case matMetal:
		pSoundList = pSoundsMetal;
		soundCount = ARRAYSIZE(pSoundsMetal);
		break;

	case matCinderBlock:
	case matRocks:
		pSoundList = pSoundsConcrete;
		soundCount = ARRAYSIZE(pSoundsConcrete);
		break;


	case matCeilingTile:
	case matNone:
	default:
		soundCount = 0;
		break;
	}

	return pSoundList;
}

void CEnvModel::MaterialSoundPrecache(Materials precacheMaterial)
{
	const char	**pSoundList;
	int			i, soundCount = 0;

	pSoundList = MaterialSoundList(precacheMaterial, soundCount);

	for (i = 0; i < soundCount; i++)
	{
		PRECACHE_SOUND((char *)pSoundList[i]);
	}
}

void CEnvModel::MaterialSoundRandom(edict_t *pEdict, Materials soundMaterial, float volume)
{
	const char	**pSoundList;
	int			soundCount = 0;

	pSoundList = MaterialSoundList(soundMaterial, soundCount);

	if (soundCount)
		EMIT_SOUND(pEdict, CHAN_BODY, pSoundList[RANDOM_LONG(0, soundCount - 1)], volume, 1.0);
}

void CEnvModel::Precache(void)
{
	//	PRECACHE_MODEL((char*)STRING(pev->model)); //LRC
	//	return;

	str1 = (char*)STRING(pev->model);

	//I need to read the word 'models' as exactly it's in path's name.

	//EDIT: isn't needed since I've found the way but I'll keep it to let old maps be played

	for (int i = 0; str1[i]; i++)
	{
		str1[i] = tolower(str1[i]);
	}

	str2 = "models";

	result = strstr(str1, str2);
//	ALERT(at_console, "PRECACHE_MODEL: %s\n", str1);



	if (result == NULL)//this should never happen
	{
		ALERT(at_console, "Could not find '%s' in '%s'\n", str2, str1);
	}
//	else
//		ALERT(at_console, "Found a substring: '%s'\n", result);

//	ALERT(at_console, "PRECACHE_MODEL: changed to '%s'\n", result);

/*	if (FClassnameIs(pev, "prop_cable"))
		PRECACHE_MODEL("models/props/prop_cable.mdl");
	else if (FClassnameIs(pev, "prop_cable_small"))
		PRECACHE_MODEL("models/props/prop_cable_small.mdl");
	else if (FClassnameIs(pev, "prop_bugs"))
		PRECACHE_MODEL("models/props/prop_bugs.mdl");
	else
	{*/
		//new
		int     length;
		byte    *pMemFile;

		//uncomment if you want to try this out
		//		strcat ( result, "asd");

		pMemFile = LOAD_FILE_FOR_ME(result, &length);

		//if the file isn't present load a default one
		if (!pMemFile)
		{
			ALERT(at_console, "Can't find '%s', using NULL model\n", result);

			PRECACHE_MODEL("models/props/error.mdl");
			bNoModel = TRUE;
			return;
		}
		else
			PRECACHE_MODEL((char *)result);

		g_engfuncs.pfnFreeFile(pMemFile);
//	}


	const char *pGibName;

	switch (m_Material)
	{
	case matWood:
		pGibName = "models/woodgibs.mdl";

		PRECACHE_SOUND("debris/bustcrate1.wav");
		PRECACHE_SOUND("debris/bustcrate2.wav");
		break;
	case matFlesh:
		pGibName = "models/fleshgibs.mdl";

		PRECACHE_SOUND("debris/bustflesh1.wav");
		PRECACHE_SOUND("debris/bustflesh2.wav");
		break;
	case matComputer:
		PRECACHE_SOUND("buttons/spark5.wav");
		PRECACHE_SOUND("buttons/spark6.wav");
		pGibName = "models/computergibs.mdl";

		PRECACHE_SOUND("debris/bustmetal1.wav");
		PRECACHE_SOUND("debris/bustmetal2.wav");
		break;

	case matUnbreakableGlass:
	case matGlass:
		pGibName = "models/glassgibs.mdl";
		//	pGibName = "models/glassSmall.mdl";

		PRECACHE_SOUND("debris/bustglass1.wav");
		PRECACHE_SOUND("debris/bustglass2.wav");
		break;
	case matMetal:
		pGibName = "models/metalplategibs.mdl";

		PRECACHE_SOUND("debris/bustmetal1.wav");
		PRECACHE_SOUND("debris/bustmetal2.wav");
		break;
	case matCinderBlock:
		pGibName = "models/cindergibs.mdl";

		PRECACHE_SOUND("debris/bustconcrete1.wav");
		PRECACHE_SOUND("debris/bustconcrete2.wav");
		break;
	case matRocks:
		pGibName = "models/rockgibs.mdl";

		PRECACHE_SOUND("debris/bustconcrete1.wav");
		PRECACHE_SOUND("debris/bustconcrete2.wav");
		break;
	case matCeilingTile:
		pGibName = "models/ceilinggibs.mdl";

		PRECACHE_SOUND("debris/bustceiling.wav");
		break;
	}
	MaterialSoundPrecache(m_Material);
	if (m_iszGibModel)
		pGibName = STRING(m_iszGibModel);

	m_idShard = PRECACHE_MODEL((char *)pGibName);

	// Precache the spawn item's data
	if (m_iszSpawnObject)
		UTIL_PrecacheOther((char *)STRING(m_iszSpawnObject));
}

void CEnvModel::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if (ShouldToggle(useType, !(pev->spawnflags & SF_ENVMODEL_OFF)))
	{
		if (pev->spawnflags & SF_ENVMODEL_OFF)
			pev->spawnflags &= ~SF_ENVMODEL_OFF;
		else
			pev->spawnflags |= SF_ENVMODEL_OFF;

		SetSequence();
		//	SetNextThink( 0.1 );
		pev->nextthink = 0.1;
	}
}


void CEnvModel::SetSequence(void)
{
	int iszSeq;

	if (pev->spawnflags & SF_ENVMODEL_OFF)
		iszSeq = m_iszSequence_Off;
	else
		iszSeq = m_iszSequence_On;

	if (!iszSeq)
		return;
	pev->sequence = LookupSequence(STRING(iszSeq));

	if (pev->sequence == -1)
	{
		if (pev->targetname)
			ALERT(at_error, "env_model %s: unknown sequence \"%s\"\n", STRING(pev->targetname), STRING(iszSeq));
		else
			ALERT(at_error, "env_model: unknown sequence \"%s\"\n", STRING(pev->targetname), STRING(iszSeq));
		pev->sequence = 0;
	}

	pev->frame = 0;
	ResetSequenceInfo();

	if (pev->spawnflags & SF_ENVMODEL_OFF)
	{
		if (m_iAction_Off == 1)
			m_fSequenceLoops = 1;
		else
			m_fSequenceLoops = 0;
	}
	else
	{
		if (m_iAction_On == 1)
			m_fSequenceLoops = 1;
		else
			m_fSequenceLoops = 0;
	}
}

BOOL CEnvModel::CanSee(CBaseEntity *pLooker, CBaseEntity *pSeen)
{
	if (pev->frags && (pLooker->pev->origin - pSeen->pev->origin).Length() > pev->frags)
		return FALSE;

	if (pev->max_health < 360)
	{
		Vector2D	vec2LOS;
		float	flDot;
		float flComp = pev->health;
		UTIL_MakeVectors(pLooker->pev->angles);
		vec2LOS = (pSeen->pev->origin - pLooker->pev->origin).Make2D();
		vec2LOS = vec2LOS.Normalize();
		flDot = DotProduct(vec2LOS, gpGlobals->v_forward.Make2D());
		if (pev->max_health == -1)
		{
			CBaseMonster *pMonst = pLooker->MyMonsterPointer();
			if (pMonst)
				flComp = pMonst->m_flFieldOfView;
			else
				return FALSE; // not a monster, can't use M-M-M-MonsterVision
		}
		if (flDot <= flComp)
			return FALSE;
	}
	return TRUE;
}

class CEnvProp : public CEnvModel
{
	virtual void Spawn()
	{
		pev->controller[3] = 12;
		CEnvModel::Spawn();
	};

};

LINK_ENTITY_TO_CLASS(prop_static, CEnvProp);
//*************************

//=========================================================
//LRC- the long-awaited effect. (Rain, in the desert? :)
//
//FIXME: give designers a _lot_ more control.
//=========================================================
#define MAX_RAIN_BEAMS 32

#define AXIS_X 1
#define AXIS_Y 2
#define AXIS_Z 0

#define EXTENT_OBSTRUCTED 1
#define EXTENT_ARCING 2
#define EXTENT_OBSTRUCTED_REVERSE 3
#define EXTENT_ARCING_REVERSE 4
#define EXTENT_ARCING_THROUGH 5 //AJH

// Shepard : SoHL rain
#include "gamerules.h"
#include "player.h"
//extern CGameRules *g_pGameRules;

class CEnvRain : public CBaseEntity
{
public:
	void	Spawn( void );
	void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void	Think( void );
	void	Precache( void );
	void	KeyValue( KeyValueData *pkvd );
	virtual int	ObjectCaps( void ) { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	BOOL m_bState;
	//STATE	m_iState;
	int		m_spriteTexture;
	int		m_iszSpriteName; // have to saverestore this, the beams keep a link to it
	int		m_dripSize;
	int		m_minDripSpeed;
	int		m_maxDripSpeed;
	int		m_burstSize;
	int		m_brightness;
	int		m_pitch; // don't saverestore this
	float	m_flUpdateTime;
	float	m_flMaxUpdateTime;
	//	CBeam*	m_pBeams[MAX_RAIN_BEAMS];
	int m_axis;
	int m_iExtent;
	float m_fLifeTime;
	int m_iNoise;


	//virtual STATE GetState( void ) { return m_iState; };
};

LINK_ENTITY_TO_CLASS( env_rain, CEnvRain );

TYPEDESCRIPTION	CEnvRain::m_SaveData[] =
{
	DEFINE_FIELD( CEnvRain, m_bState, FIELD_BOOLEAN ),
	//DEFINE_FIELD( CEnvRain, m_iState, FIELD_INTEGER ),
	DEFINE_FIELD( CEnvRain, m_spriteTexture, FIELD_INTEGER ),
	DEFINE_FIELD( CEnvRain, m_dripSize, FIELD_INTEGER ),
	DEFINE_FIELD( CEnvRain, m_minDripSpeed, FIELD_INTEGER ),
	DEFINE_FIELD( CEnvRain, m_maxDripSpeed, FIELD_INTEGER ),
	DEFINE_FIELD( CEnvRain, m_burstSize, FIELD_INTEGER ),
	DEFINE_FIELD( CEnvRain, m_brightness, FIELD_INTEGER ),
	DEFINE_FIELD( CEnvRain, m_flUpdateTime, FIELD_FLOAT ),
	DEFINE_FIELD( CEnvRain, m_flMaxUpdateTime, FIELD_FLOAT ),
	DEFINE_FIELD( CEnvRain, m_iszSpriteName, FIELD_STRING ),
	DEFINE_FIELD( CEnvRain, m_axis, FIELD_INTEGER ),
	DEFINE_FIELD( CEnvRain, m_iExtent, FIELD_INTEGER ),
	DEFINE_FIELD( CEnvRain, m_fLifeTime, FIELD_FLOAT ),
	DEFINE_FIELD( CEnvRain, m_iNoise, FIELD_INTEGER ),
	//	DEFINE_FIELD( CEnvRain, m_pBeams, FIELD_CLASSPTR, MAX_RAIN_BEAMS ),
};

IMPLEMENT_SAVERESTORE( CEnvRain, CBaseEntity );

void CEnvRain::Precache( void )
{
	if ( pev->model )
		m_spriteTexture = PRECACHE_MODEL( (char *)STRING( m_iszSpriteName ) );
	else
		m_spriteTexture = PRECACHE_MODEL( "sprites/hi_rain.spr" );

}

void CEnvRain::KeyValue( KeyValueData *pkvd )
{
	if ( FStrEq( pkvd->szKeyName, "m_dripSize" ) )
	{
		m_dripSize = atoi( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "m_burstSize" ) )
	{
		m_burstSize = atoi( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "m_dripSpeed" ) )
	{
		int temp = atoi( pkvd->szValue );
		m_maxDripSpeed = temp + (temp / 4);
		m_minDripSpeed = temp - (temp / 4);
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "m_brightness" ) )
	{
		m_brightness = atoi( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "m_flUpdateTime" ) )
	{
		m_flUpdateTime = atof( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "m_flMaxUpdateTime" ) )
	{
		m_flMaxUpdateTime = atof( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "pitch" ) )
	{
		m_pitch = atoi( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "texture" ) )
	{
		m_iszSpriteName = ALLOC_STRING( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "m_axis" ) )
	{
		m_axis = atoi( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "m_iExtent" ) )
	{
		m_iExtent = atoi( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "m_fLifeTime" ) )
	{
		m_fLifeTime = atof( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "m_iNoise" ) )
	{
		m_iNoise = atoi( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue( pkvd );
}

void CEnvRain::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	//if ( !ShouldToggle( useType ) ) return;
	if ( !ShouldToggle( useType, m_bState ) )
		return;

	//if ( m_iState == STATE_ON )
	if ( m_bState == TRUE )
	{
		m_bState = FALSE;
		g_pGameRules->m_bWorldHasRain = false;
		//DontThink();
		pev->nextthink = -1; //SP FIX
	}
	else
	{
		m_bState = TRUE;
		//SetNextThink( 0.1 );
		pev->nextthink = 0.1; //SP FIX
		g_pGameRules->m_bWorldHasRain = true;
	}
}

#define SF_RAIN_START_OFF	1

void CEnvRain::Spawn( void )
{
	Precache();

	if ( pev->model )
		SET_MODEL( ENT( pev ), STRING( pev->model ) );		// Set size
	else
		SET_MODEL( ENT( pev ), "sprites/hi_rain.spr" );		// Set size

	pev->solid = SOLID_NOT;
	pev->effects = EF_NODRAW;

	if ( pev->rendercolor == g_vecZero )
		pev->rendercolor = Vector( 255, 255, 255 );

	if ( m_pitch )
		pev->angles.x = m_pitch;
	else if ( pev->angles.x == 0 ) // don't allow horizontal rain.  //AJH -Why not?
		pev->angles.x = 90;

	if ( m_burstSize == 0 ) // in case the level designer forgot to set it.
		m_burstSize = 2;

	if ( pev->spawnflags & SF_RAIN_START_OFF )
	{
		g_pGameRules->m_bWorldHasRain = false;
		//m_iState = STATE_OFF;
		m_bState = FALSE;
	}
	else
	{
		//m_iState = STATE_ON;
		m_bState = TRUE;
		pev->nextthink = 0.1; //SP 
		g_pGameRules->m_bWorldHasRain = true;
	}

	pev->spawnflags = EXTENT_OBSTRUCTED;
}

void CEnvRain::Think( void )
{
	ALERT(at_console,"RainThink %d %d %d %s\n",m_spriteTexture,m_dripSize,m_brightness,STRING(m_iszSpriteName));
	Vector vecSrc;
	Vector vecDest;

	UTIL_MakeVectors( pev->angles );
	Vector vecOffs = gpGlobals->v_forward;
	switch ( m_axis )
	{
	case AXIS_X:
		vecOffs = vecOffs * (pev->size.x / vecOffs.x);
		break;
	case AXIS_Y:
		vecOffs = vecOffs * (pev->size.y / vecOffs.y);
		break;
	case AXIS_Z:
		vecOffs = vecOffs * (pev->size.z / vecOffs.z);
		break;
	}

	ALERT(at_console,"RainThink offs.z = %f, size.z = %f\n",vecOffs.z,pev->size.z);

	int repeats;
	if ( !m_fLifeTime && !m_flUpdateTime && !m_flMaxUpdateTime )
		repeats = m_burstSize * 3;
	else
		repeats = m_burstSize;

	int drawn = 0;
	int tries = 0;
	TraceResult tr;
	BOOL bDraw;

	while ( drawn < repeats && tries < (repeats * 3) )
	{
		tries++;
		if ( m_axis == AXIS_X )
			vecSrc.x = pev->maxs.x;
		else
			vecSrc.x = pev->mins.x + RANDOM_LONG( 0, pev->size.x );
		if ( m_axis == AXIS_Y )
			vecSrc.y = pev->maxs.y;
		else
			vecSrc.y = pev->mins.y + RANDOM_LONG( 0, pev->size.y );
		if ( m_axis == AXIS_Z )
			vecSrc.z = pev->maxs.z;
		else
			vecSrc.z = pev->mins.z + RANDOM_LONG( 0, pev->size.z );
		vecDest = vecSrc - vecOffs;
		bDraw = TRUE;

		switch ( m_iExtent )
		{
		case EXTENT_OBSTRUCTED:
			UTIL_TraceLine( vecSrc, vecDest, ignore_monsters, NULL, &tr );
			vecDest = tr.vecEndPos;
			break;
		case EXTENT_OBSTRUCTED_REVERSE:
			UTIL_TraceLine( vecDest, vecSrc, ignore_monsters, NULL, &tr );
			vecSrc = tr.vecEndPos;
			break;
		case EXTENT_ARCING:
			UTIL_TraceLine( vecSrc, vecDest, ignore_monsters, NULL, &tr );
			if ( tr.flFraction == 1.0 ) bDraw = FALSE;
			vecDest = tr.vecEndPos;
			break;
		case EXTENT_ARCING_THROUGH:		//AJH - Arcs full length of brush only when blocked
			UTIL_TraceLine( vecDest, vecSrc, dont_ignore_monsters, NULL, &tr );
			if ( tr.flFraction == 1.0 ) bDraw = FALSE;
			break;
		case EXTENT_ARCING_REVERSE:
			UTIL_TraceLine( vecDest, vecSrc, ignore_monsters, NULL, &tr );
			if ( tr.flFraction == 1.0 ) bDraw = FALSE;
			vecSrc = tr.vecEndPos;
			break;
		}
		//		vecDest.z = pev->mins.z;
		if ( !bDraw ) continue;

		drawn++;

		MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_BEAMPOINTS );
		WRITE_COORD( vecDest.x );
		WRITE_COORD( vecDest.y );
		WRITE_COORD( vecDest.z );
		WRITE_COORD( vecSrc.x );
		WRITE_COORD( vecSrc.y );
		WRITE_COORD( vecSrc.z );
		WRITE_SHORT( m_spriteTexture );
		WRITE_BYTE( (int)0 ); // framestart
		WRITE_BYTE( (int)0 ); // framerate
		if ( m_fLifeTime ) // life
			WRITE_BYTE( (int)(m_fLifeTime * 10) );
		else if ( m_flMaxUpdateTime )
			WRITE_BYTE( (int)(RANDOM_FLOAT( m_flUpdateTime, m_flMaxUpdateTime ) * 30) );
		else
			WRITE_BYTE( (int)(m_flUpdateTime * 30) ); // life
		WRITE_BYTE( m_dripSize );  // width
		WRITE_BYTE( m_iNoise );   // noise
		WRITE_BYTE( (int)pev->rendercolor.x );   // r,
		WRITE_BYTE( (int)pev->rendercolor.y );   //    g,
		WRITE_BYTE( (int)pev->rendercolor.z );   //       b
		WRITE_BYTE( m_brightness );	// brightness
		WRITE_BYTE( (int)RANDOM_LONG( m_minDripSpeed, m_maxDripSpeed ) );		// speed
		MESSAGE_END();
	}
	/*
	CSprite *pSprite = CSprite::SpriteCreate( "sprites/exp_end.spr", vecDest, TRUE );
	pSprite->SetTransparency( kRenderTransAlpha, 222, 222, 255, 88, kRenderFxNone );
	pSprite->SetScale( 5.0 );
	pSprite->Expand( 5 , RANDOM_FLOAT( 80.0, 90.0 )  );
	pSprite->pev->frame = 0;
	*/
	// drawn will be false if we didn't draw anything.
	if ( pev->target && drawn )
		FireTargets( STRING( pev->target ), this, this, USE_TOGGLE, 0 );

	if ( m_flMaxUpdateTime )
		pev->nextthink = (RANDOM_FLOAT( m_flMaxUpdateTime, m_flUpdateTime ));
	else if ( m_flUpdateTime )
		pev->nextthink = (m_flUpdateTime); //SP FIXs
}

//=========================================================
// G-Cont - env_rain, use triAPI
//=========================================================

void CRainSettings::Spawn()
{
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	pev->effects |= EF_NODRAW;
}

void CRainSettings::KeyValue( KeyValueData *pkvd )
{
	if ( FStrEq( pkvd->szKeyName, "m_flDistance" ) )
	{
		Rain_Distance = atof( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "m_iMode" ) )
	{
		Rain_Mode = atoi( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else
	{
		CBaseEntity::KeyValue( pkvd );
	}
}

LINK_ENTITY_TO_CLASS( rain_settings, CRainSettings );

TYPEDESCRIPTION	CRainSettings::m_SaveData[] =
{
	DEFINE_FIELD( CRainSettings, Rain_Distance, FIELD_FLOAT ),
	DEFINE_FIELD( CRainSettings, Rain_Mode, FIELD_INTEGER ),
};
IMPLEMENT_SAVERESTORE( CRainSettings, CBaseEntity );



void CRainModify::Spawn()
{
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	pev->effects |= EF_NODRAW;

	if ( FStringNull( pev->targetname ) )
		pev->spawnflags |= 1;
}

void CRainModify::KeyValue( KeyValueData *pkvd )
{
	if ( FStrEq( pkvd->szKeyName, "m_iDripsPerSecond" ) )
	{
		Rain_Drips = atoi( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "m_flWindX" ) )
	{
		Rain_windX = atof( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "m_flWindY" ) )
	{
		Rain_windY = atof( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "m_flRandX" ) )
	{
		Rain_randX = atof( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "m_flRandY" ) )
	{
		Rain_randY = atof( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "m_flTime" ) )
	{
		fadeTime = atof( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else
	{
		CBaseEntity::KeyValue( pkvd );
	}
}

void CRainModify::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( pev->spawnflags & 1 )
		return; // constant

	if ( gpGlobals->deathmatch )
	{
		ALERT( at_console, "Rain error: only static rain in multiplayer\n" );
		return; // not in multiplayer
	}

	CBasePlayer *pPlayer;
	pPlayer = (CBasePlayer *)CBaseEntity::Instance( g_engfuncs.pfnPEntityOfEntIndex( 1 ) );

	if ( fadeTime )
	{ // write to 'ideal' settings
		pPlayer->Rain_ideal_dripsPerSecond = Rain_Drips;
		pPlayer->Rain_ideal_randX = Rain_randX;
		pPlayer->Rain_ideal_randY = Rain_randY;
		pPlayer->Rain_ideal_windX = Rain_windX;
		pPlayer->Rain_ideal_windY = Rain_windY;

		pPlayer->Rain_endFade = gpGlobals->time + fadeTime;
		pPlayer->Rain_nextFadeUpdate = gpGlobals->time + 1;
	}
	else
	{
		pPlayer->Rain_dripsPerSecond = Rain_Drips;
		pPlayer->Rain_randX = Rain_randX;
		pPlayer->Rain_randY = Rain_randY;
		pPlayer->Rain_windX = Rain_windX;
		pPlayer->Rain_windY = Rain_windY;

		pPlayer->Rain_needsUpdate = 1;
	}
}

LINK_ENTITY_TO_CLASS( rain_modify, CRainModify );

TYPEDESCRIPTION	CRainModify::m_SaveData[] =
{
	DEFINE_FIELD( CRainModify, fadeTime, FIELD_FLOAT ),
	DEFINE_FIELD( CRainModify, Rain_Drips, FIELD_INTEGER ),
	DEFINE_FIELD( CRainModify, Rain_randX, FIELD_FLOAT ),
	DEFINE_FIELD( CRainModify, Rain_randY, FIELD_FLOAT ),
	DEFINE_FIELD( CRainModify, Rain_windX, FIELD_FLOAT ),
	DEFINE_FIELD( CRainModify, Rain_windY, FIELD_FLOAT ),
};
IMPLEMENT_SAVERESTORE( CRainModify, CBaseEntity );


class CDynamicLOD : public CPointEntity
{
public:
	void Spawn(void);
	void Precache(void);
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);

	void KeyValue(KeyValueData *pkvd);
	void EXPORT Think(void);

	virtual int		Save(CSave &save);
	virtual int		Restore(CRestore &restore);
	static	TYPEDESCRIPTION m_SaveData[];

	string_t m_iszWallLow;
	string_t m_iszWallNormal;
	string_t m_iszWallDetail;
};

LINK_ENTITY_TO_CLASS(env_dynamic_lod, CDynamicLOD);

TYPEDESCRIPTION CDynamicLOD::m_SaveData[] =
{
	DEFINE_FIELD(CDynamicLOD, m_iszWallLow, FIELD_STRING),
	DEFINE_FIELD(CDynamicLOD, m_iszWallNormal, FIELD_STRING),//letras
	DEFINE_FIELD(CDynamicLOD, m_iszWallDetail, FIELD_STRING),
};

IMPLEMENT_SAVERESTORE(CDynamicLOD, CPointEntity);

void CDynamicLOD::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "m_iszWallLow"))
	{
		m_iszWallLow = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
		pev->message = ALLOC_STRING(pkvd->szValue);
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszWallNormal"))
	{
		m_iszWallNormal = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
		pev->noise1 = ALLOC_STRING(pkvd->szValue);
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszWallDetail"))
	{
		m_iszWallDetail = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
		pev->noise2 = ALLOC_STRING(pkvd->szValue);
	}
	else
	{
		CDynamicLOD::KeyValue(pkvd);
	}
}

void CDynamicLOD::Spawn(void)
{
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;

	Precache();
	SET_MODEL(ENT(pev), "sprites/null.spr"); // should be visible to send to client
	UTIL_SetSize(pev, g_vecZero, g_vecZero);

	SetThink(&CDynamicLOD::Think);
	pev->nextthink = gpGlobals->time + 1;
}

void CDynamicLOD::Think(void)
{
	//MAKE FAKE Level Of Detail
	CBaseEntity *pEntity = NULL;

	while ((pEntity = UTIL_FindEntityByClassname(pEntity, "player")) != NULL)
	{
		if (pEntity->IsPlayer())
		{
			float flDist = (pEntity->Center() - pev->origin).Length();

			//	ALERT ( at_console, "flDist is: %f\n",flDist );

			if (flDist >= 1024)//800
			{
				//ALERT ( at_console, "m_iszWallNormal\n" );
				FireTargets(STRING(pev->message), this, this, USE_ON, 0);//LOW
				FireTargets(STRING(pev->noise1), this, this, USE_OFF, 0);
				FireTargets(STRING(pev->noise2), this, this, USE_OFF, 0);
			}
			else if (flDist >= 512)
			{
				//ALERT ( at_console, "m_iszWallLow\n" );
				FireTargets(STRING(pev->message), this, this, USE_OFF, 0);
				FireTargets(STRING(pev->noise1), this, this, USE_ON, 0);//NORMAL
				FireTargets(STRING(pev->noise2), this, this, USE_OFF, 0);
			}
			else//detallado
			{
				//ALERT ( at_console, "m_iszWallDetail\n" );
				FireTargets(STRING(pev->message), this, this, USE_OFF, 0);
				FireTargets(STRING(pev->noise1), this, this, USE_OFF, 0);
				FireTargets(STRING(pev->noise2), this, this, USE_ON, 0);//DETAILED
			}
		}
	}

	pev->nextthink = gpGlobals->time + 0.5;
}

void CDynamicLOD::Precache(void)
{
	PRECACHE_MODEL("sprites/null.spr");
}

void CDynamicLOD::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if (useType == USE_ON)
		pev->effects &= ~EF_NODRAW;
	else if (useType == USE_OFF)
		pev->effects |= EF_NODRAW;
	else if (useType == USE_TOGGLE)
	{
		if (pev->effects & EF_NODRAW)
			pev->effects &= ~EF_NODRAW;
		else
			pev->effects |= EF_NODRAW;
	}
};

