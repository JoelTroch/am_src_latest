/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
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

/*
======================================================================
( == [ FMOD Studio Low level API in GoldSrc engine ] == )
======================================================================

Programmed by
Joël "Shepard62700FR, Shepard62FR" Troch

With help from
Tinithrari
Alex "HAWK0044" Bakanov
Claudio "SysOp" Ficara

This implementation is based on Joël "Shepard62700FR, Shepard62FR" Troch's
"FMODEx in GoldSrc engine" work, see credits below.

======================================================================
( == [ FMOD Studio Low level API in GoldSrc engine ] == )
======================================================================

--------------------------------------------------------------------------------

======================================================================
( == [ FMODEx in GoldSrc engine ] == )
======================================================================

Programmed by
Joël "Shepard62700FR, Shepard62FR" TROCH
With some contributions of
FROZENGREENPEACE
Richard "Mr Nameless" Rohác
Andrew "CastleBravo / Highlander" Lucas
Hunk Guerrius

======================================================================
( == [ FMODEx in GoldSrc engine ] == )
======================================================================
*/

#ifndef _CFMODENGINE_INCLUDED
#define _CFMODENGINE_INCLUDED

#include <vector>

#ifdef WIN32
#include "fmod_studio_lowlevel_windows/fmod.h"
#include "fmod_studio_lowlevel_windows/fmod_errors.h"
#else
#include "fmod_studio_lowlevel_linux/fmod.h"
#include "fmod_studio_lowlevel_linux/fmod_errors.h"
#endif

// In FMOD's documentation and examples, MAX_CHANNELS is set to 512.
// I think it doesn't fit the amount of sound played at a same time (yet) hence why I put 128.
#define MAX_CHANNELS 128
#define MAX_ENTITIES 1024

enum folders_e
{
	FOLDER_DOWNLOADS = 0,	// "am_downloads"
	FOLDER_ADDONS,			// "am_addon"
	FOLDER_HD,				// "am_hd"
	FOLDER_LANGUAGE,		// "am_french", "am_german", "am_spanish"...
	FOLDER_MOD				// "am" itself
};

#ifdef _DEBUG
const bool bVerboseDebug = false; // Setting this to true will allow the engine to report everything it does (watch out for console spam)
// Important note about verbose debugging :
// Don't panic if not every "m_pResult" changes returns "FMOD_OK".
// In some situations it is normal that the result isn't "FMOD_OK".
// 2 examples :
// 1) Bad handles : According to some research, FMOD seems to free automatically stuff (to ease the CPU/RAM), but sometimes, it doesn't.
// An extra check can't hurt right ?
// 2) File not found : You will likely get these when attempting to play sounds/musics.
// Because we have to check the "downloads", "addon", "hd", "_lang" and the mod folder itself (depending on player's configuration),
// You will likely get the "not found file" error. The only one that you should be aware of is when it checks for the mod folder itself.
#endif

// CFMOD2DSound - A sound that has no origin in the world, main usage would be for GUI or any other special sounds
class CFMOD2DSound
{
public:
	CFMOD2DSound( FMOD_SYSTEM *pSystem, const char *cSound, bool &bOk, float flPitch, bool bLookAtAddonsFolder, bool bLookAtHDFolder, char *cLanguage );
	bool IsPlaying( void );
	void Update( void );
	~CFMOD2DSound( void );

private:
	FMOD_BOOL m_bIsPlaying;

	FMOD_RESULT m_pResult;
	FMOD_CHANNEL *m_pChannel;
	FMOD_SOUND *m_pSound;
};

//==================================================
//==================================================
//==================================================

// CFMOD3DSound - A sound that has an origin in the world, a minimum distance and a maximum distance which can also follow an entity
class CFMOD3DSound
{
public:
	CFMOD3DSound( FMOD_SYSTEM *pSystem, const Vector vecPosition, bool &bOk, const char *cSound, float flMinDistance, float flMaxDistance, int iEntityIndexToFollow, float flPitch, bool bLookAtAddonsFolder, bool bLookAtHDFolder, char *cLanguage );
	bool IsPlaying( void );
	void Update( void );
	~CFMOD3DSound( void );

private:
	cl_entity_t *m_pEntityToFollow;
	FMOD_BOOL m_bIsPlaying;

	FMOD_RESULT m_pResult;
	FMOD_CHANNEL *m_pChannel;
	FMOD_SOUND *m_pSound;
};

//==================================================
//==================================================
//==================================================

// CFMODEngine - The engine itself
class CFMODEngine
{
public:
	// Entity related stuff
	bool DoesEntityExists( const cl_entity_t *pEntity );
	void AddEntity( cl_entity_t *pEntity );

	// Engine related stuff
	void Init( void );
	void Reset( void );
	void Shutdown( void );
	void Update( void );

	// Sound related stuff
	void Play2DSound( const char *cSound, float flPitch = 1.0f );
	void Play3DSoundAtEntity( const int iIndex, const char *cSound, const float flMinDistance = 512.0f, float flMaxDistance = -1.0f, const bool bFollowEntity = false, const float flPitch = 1.0f );
	void Play3DSoundAtPosition( const Vector vecPosition, const char *cSound, const float flMinDistance = 512.0f, float flMaxDistance = -1.0f, const int iEntityIndexToFollow = -1, const float flPitch = 1.0f );
	void SetReverb( const int iReverbType );

	// Music related stuff
	void PlayMusic( const char *cStream, const bool bLooped = false, const bool bActionMusic = false );
	void StopMusic( const bool bActionMusic = false );

	bool m_bAction;
	bool m_bInitialized;

	cvar_t *m_cvDebugShowCPUUsage;
	cvar_t *m_cvMP3Volume;
	cvar_t *m_cvVolume;

	cl_entity_t *m_pEntities[MAX_ENTITIES];

	float m_flCurrentActionMP3Volume;
	float m_flCurrentStealthMP3Volume;
	float m_flDesiredMP3Volume;
	float m_flVolume;

	int m_iNumEntities;

	FMOD_RESULT (__stdcall * m_pFuncChannelIsPlaying)				( FMOD_CHANNEL *channel, FMOD_BOOL *isplaying );
	FMOD_RESULT (__stdcall * m_pFuncChannelSet3DAttributes)			( FMOD_CHANNEL *channel, const FMOD_VECTOR *pos, const FMOD_VECTOR *vel, const FMOD_VECTOR *alt_pan_pos );
	FMOD_RESULT (__stdcall * m_pFuncChannelSet3DSpread)				( FMOD_CHANNEL *channel, float angle );
	FMOD_RESULT (__stdcall * m_pFuncChannelSetPaused)				( FMOD_CHANNEL *channel, FMOD_BOOL paused );
	FMOD_RESULT (__stdcall * m_pFuncChannelSetPitch)				( FMOD_CHANNEL *channel, float pitch );
	FMOD_RESULT (__stdcall * m_pFuncChannelSetVolume)				( FMOD_CHANNEL *channel, float volume );
	FMOD_RESULT (__stdcall * m_pFuncChannelStop)					( FMOD_CHANNEL *channel );
	FMOD_RESULT (__stdcall * m_pFuncSoundSet3DMinMaxDistance)		( FMOD_SOUND *sound, float min, float max );
	FMOD_RESULT (__stdcall * m_pFuncSoundRelease)					( FMOD_SOUND *sound );
	FMOD_RESULT (__stdcall * m_pFuncSystemCreateSound)				( FMOD_SYSTEM *system, const char *name_or_data, FMOD_MODE mode, FMOD_CREATESOUNDEXINFO *exinfo, FMOD_SOUND **sound );
	FMOD_RESULT (__stdcall * m_pFuncSystemPlaySound)				( FMOD_SYSTEM *system, FMOD_SOUND *sound, FMOD_CHANNELGROUP *channelgroup, FMOD_BOOL paused, FMOD_CHANNEL **channel );

private:
	FMOD_OUTPUTTYPE GetOutputMode( void );
	FMOD_SPEAKERMODE GetSpeakerMode( void );

	int ReadRegistry( const folders_e eSettingToLookAt );

	void ShowMessageBox( const char* cTitle, const char* cMessage, const bool bIsError );

	bool m_bPathsLookAtAddonsFolder;
	bool m_bPathsLookAtHDFolder;

	char m_cLastMap[64];
	char m_cLanguage[9];

	// Sound factories
	std::vector<CFMOD2DSound*> m_p2DSoundsFactory;
	std::vector<CFMOD3DSound*> m_p3DSoundsFactory;

	// FMOD related stuff
	FMOD_RESULT m_pResult;

	FMOD_RESULT (__stdcall * m_pFuncSystemCreate)					( FMOD_SYSTEM **system );
	FMOD_RESULT (__stdcall * m_pFuncSystemCreateStream)				( FMOD_SYSTEM *system, const char *name_or_data, FMOD_MODE mode, FMOD_CREATESOUNDEXINFO *exinfo, FMOD_SOUND **sound );
	FMOD_RESULT (__stdcall * m_pFuncSystemGetCPUUsage)				( FMOD_SYSTEM *system, float *dsp, float *stream, float *geometry, float *update, float *total );
	FMOD_RESULT (__stdcall * m_pFuncSystemGetSoftwareFormat)		( FMOD_SYSTEM *system, int *samplerate, FMOD_SPEAKERMODE *speakermode, int *numrawspeakers );
	FMOD_RESULT (__stdcall * m_pFuncSystemGetVersion)				( FMOD_SYSTEM *system, unsigned int *version );
	FMOD_RESULT (__stdcall * m_pFuncSystemInit)						( FMOD_SYSTEM *system, int maxchannels, FMOD_INITFLAGS flags, void *extradriverdata );
	FMOD_RESULT (__stdcall * m_pFuncSystemRelease)					( FMOD_SYSTEM *system );
	FMOD_RESULT (__stdcall * m_pFuncSystemSet3DListenerAttributes)	( FMOD_SYSTEM *system, int listener, const FMOD_VECTOR *pos, const FMOD_VECTOR *vel, const FMOD_VECTOR *forward, const FMOD_VECTOR *up );
	FMOD_RESULT (__stdcall * m_pFuncSystemSet3DSettings)			( FMOD_SYSTEM *system, float dopplerscale, float distancefactor, float rolloffscale );
	FMOD_RESULT (__stdcall * m_pFuncSystemSetOutput)				( FMOD_SYSTEM *system, FMOD_OUTPUTTYPE output );
	FMOD_RESULT (__stdcall * m_pFuncSystemSetReverbProperties)		( FMOD_SYSTEM *system, int instance, const FMOD_REVERB_PROPERTIES *prop );
	FMOD_RESULT (__stdcall * m_pFuncSystemSetSoftwareFormat)		( FMOD_SYSTEM *system, int samplerate, FMOD_SPEAKERMODE speakermode, int numrawspeakers );
	FMOD_RESULT (__stdcall * m_pFuncSystemUpdate)					( FMOD_SYSTEM *system );

	FMOD_CHANNEL *m_pChannelMusicAction;
	FMOD_CHANNEL *m_pChannelMusicStealth;
	FMOD_SOUND *m_pMusicAction;
	FMOD_SOUND *m_pMusicStealth;
	FMOD_SYSTEM *m_pSystemSounds;
	FMOD_SYSTEM *m_pSystemMusics;

	void *m_pFMODLib;
};

extern CFMODEngine gFMOD;

#endif // _CFMODENGINE_INCLUDED
