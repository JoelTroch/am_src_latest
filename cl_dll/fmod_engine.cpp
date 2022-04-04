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

// TODO
// - Check for possible errors, optimize code
// - See if it works on Linux (should be)

// TODO (but not really important)
// - Translate all errors/warnings messages

#include "hud.h"
#include "cl_util.h"
#include "../external/SDL2/SDL.h"
#include "fmod_engine.h"

#ifdef WIN32
// For the time being, this header must stay here, otherwise it will cause a conflict
// with the Rainbows fake OS (vgui_os.cpp) about "PlaySoundA".
#include <windows.h>
#else
#include <dlfcn.h>
#endif

//==================================================
//================ UTILITY METHODS =================
//==================================================

// IsVerboseDebuggingAllowed - Check if the engine is allowed to report everything it does
bool IsVerboseDebuggingAllowed( void )
{
#ifdef _DEBUG
	return bVerboseDebug;
#else
	return false;
#endif
}

// ReplaceStr - Replace a substring by another one in a string
void ReplaceStr( char *cString, const char *cSearch, const char *cReplace )
{
	char cBuffer[4096];
	char *cStrstrPointer;
	if ( !(cStrstrPointer = strstr( cString, cSearch )) )
		return;

	strncpy( cBuffer, cString, cStrstrPointer - cString );
	cBuffer[cStrstrPointer - cString] = NULL;
	sprintf( cBuffer + (cStrstrPointer - cString), "%s%s", cReplace, cStrstrPointer + strlen( cSearch ) );
	cString[0] = NULL;
	strcpy( cString, cBuffer );
	// Commented because it cause issues with the "subfolders" lookup system
	//return ReplaceStr( cString, cSearch, cReplace );
}

//==================================================
//================= ENGINE ITSELF ==================
//==================================================

// DoesEntityExists - Check if an entity exists in the entities array
bool CFMODEngine::DoesEntityExists( const cl_entity_t *pEntity )
{
	if ( !pEntity )
		return false;

	if ( m_iNumEntities == 0 )
		return false;

	for ( int i = 0; i < m_iNumEntities; i++ )
	{
		if ( m_pEntities[i] == pEntity )
			return true;
	}
	return false;
}

// AddEntity - Add an entity to the entities array
void CFMODEngine::AddEntity( cl_entity_t *pEntity )
{
	if ( m_iNumEntities == MAX_ENTITIES )
		return;

	m_pEntities[m_iNumEntities++] = pEntity;
}

//--------------------------------------------------
//--------------------------------------------------
//--------------------------------------------------

// Init - Initialize FMOD
void CFMODEngine::Init( void )
{
	// Don't initialize twice !
	if ( m_bInitialized )
	{
		if ( IsVerboseDebuggingAllowed() )
			gEngfuncs.Con_Printf( "[CFMODEngine::Init] Attempting to initialize again! This attempt has been denied!\n" );

		return;
	}

	m_bInitialized = false;

	// Load the library
	if ( IsVerboseDebuggingAllowed() )
		gEngfuncs.Con_Printf( "[CFMODEngine::Init] Loading FMOD library...\n" );

#ifdef WIN32
	char cFMODLibPath[256];
	sprintf( cFMODLibPath, "%s\\cl_dlls\\fmod.dll", gEngfuncs.pfnGetGameDirectory() );
	m_pFMODLib = LoadLibrary( cFMODLibPath );
#else
	char cFMODLibPath[256];
	sprintf( cFMODLibPath, "%s/cl_dlls/libfmod.so", gEngfuncs.pfnGetGameDirectory() );
	m_pFMODLib = dlopen( cFMODLibPath, RTLD_LAZY );
#endif

	// Woops!!!
	if ( !m_pFMODLib )
	{
#ifdef WIN32
		gEngfuncs.Con_Printf( "[CFMODEngine::Init] Error loading FMOD library! (%d)\n", GetLastError() );
#else
		gEngfuncs.Con_Printf( "[CFMODEngine::Init] Error loading FMOD library! (%s)\n", dlerror() );
#endif
		return;
	}

#ifdef WIN32
	(FARPROC &)m_pFuncChannelIsPlaying				= GetProcAddress( (HMODULE)m_pFMODLib, "_FMOD5_Channel_IsPlaying@8" );
	(FARPROC &)m_pFuncChannelSet3DAttributes		= GetProcAddress( (HMODULE)m_pFMODLib, "_FMOD5_Channel_Set3DAttributes@16" );
	(FARPROC &)m_pFuncChannelSet3DSpread			= GetProcAddress( (HMODULE)m_pFMODLib, "_FMOD5_Channel_Set3DSpread@8" );
	(FARPROC &)m_pFuncChannelSetPaused				= GetProcAddress( (HMODULE)m_pFMODLib, "_FMOD5_Channel_SetPaused@8" );
	(FARPROC &)m_pFuncChannelSetPitch				= GetProcAddress( (HMODULE)m_pFMODLib, "_FMOD5_Channel_SetPitch@8" );
	(FARPROC &)m_pFuncChannelSetVolume				= GetProcAddress( (HMODULE)m_pFMODLib, "_FMOD5_Channel_SetVolume@8" );
	(FARPROC &)m_pFuncChannelStop					= GetProcAddress( (HMODULE)m_pFMODLib, "_FMOD5_Channel_Stop@4" );
	(FARPROC &)m_pFuncSoundSet3DMinMaxDistance		= GetProcAddress( (HMODULE)m_pFMODLib, "_FMOD5_Sound_Set3DMinMaxDistance@12" );
	(FARPROC &)m_pFuncSoundRelease					= GetProcAddress( (HMODULE)m_pFMODLib, "_FMOD5_Sound_Release@4" );
	(FARPROC &)m_pFuncSystemCreate					= GetProcAddress( (HMODULE)m_pFMODLib, "_FMOD5_System_Create@4" );
	(FARPROC &)m_pFuncSystemCreateSound				= GetProcAddress( (HMODULE)m_pFMODLib, "_FMOD5_System_CreateSound@20" );
	(FARPROC &)m_pFuncSystemCreateStream			= GetProcAddress( (HMODULE)m_pFMODLib, "_FMOD5_System_CreateStream@20" );
	(FARPROC &)m_pFuncSystemGetCPUUsage				= GetProcAddress( (HMODULE)m_pFMODLib, "_FMOD5_System_GetCPUUsage@24" );
	(FARPROC &)m_pFuncSystemGetSoftwareFormat		= GetProcAddress( (HMODULE)m_pFMODLib, "_FMOD5_System_GetSoftwareFormat@16" );
	(FARPROC &)m_pFuncSystemGetVersion				= GetProcAddress( (HMODULE)m_pFMODLib, "_FMOD5_System_GetVersion@8" );
	(FARPROC &)m_pFuncSystemInit					= GetProcAddress( (HMODULE)m_pFMODLib, "_FMOD5_System_Init@16" );
	(FARPROC &)m_pFuncSystemPlaySound				= GetProcAddress( (HMODULE)m_pFMODLib, "_FMOD5_System_PlaySound@20" );
	(FARPROC &)m_pFuncSystemRelease					= GetProcAddress( (HMODULE)m_pFMODLib, "_FMOD5_System_Release@4" );
	(FARPROC &)m_pFuncSystemSet3DListenerAttributes	= GetProcAddress( (HMODULE)m_pFMODLib, "_FMOD5_System_Set3DListenerAttributes@24" );
	(FARPROC &)m_pFuncSystemSet3DSettings			= GetProcAddress( (HMODULE)m_pFMODLib, "_FMOD5_System_Set3DSettings@16" );
	(FARPROC &)m_pFuncSystemSetOutput				= GetProcAddress( (HMODULE)m_pFMODLib, "_FMOD5_System_SetOutput@8" );
	(FARPROC &)m_pFuncSystemSetReverbProperties		= GetProcAddress( (HMODULE)m_pFMODLib, "_FMOD5_System_SetReverbProperties@12" );
	(FARPROC &)m_pFuncSystemSetSoftwareFormat		= GetProcAddress( (HMODULE)m_pFMODLib, "_FMOD5_System_SetSoftwareFormat@16" );
	(FARPROC &)m_pFuncSystemUpdate					= GetProcAddress( (HMODULE)m_pFMODLib, "_FMOD5_System_Update@4" );
#else
	// TODO
#endif

	if ( !m_pFuncChannelIsPlaying || !m_pFuncChannelSet3DAttributes || !m_pFuncChannelSet3DSpread || !m_pFuncChannelSetPaused || !m_pFuncChannelSetPitch ||
		!m_pFuncChannelSetPaused || !m_pFuncChannelSetVolume || !m_pFuncChannelStop || !m_pFuncSoundSet3DMinMaxDistance || !m_pFuncSoundRelease ||
		!m_pFuncSystemCreate || !m_pFuncSystemCreateSound || !m_pFuncSystemCreateStream || !m_pFuncSystemGetCPUUsage || !m_pFuncSystemGetSoftwareFormat ||
		!m_pFuncSystemGetVersion || !m_pFuncSystemInit || !m_pFuncSystemPlaySound || !m_pFuncSystemRelease || !m_pFuncSystemSet3DListenerAttributes ||
		!m_pFuncSystemSet3DSettings || !m_pFuncSystemSetOutput || !m_pFuncSystemSetReverbProperties || !m_pFuncSystemSetSoftwareFormat || !m_pFuncSystemUpdate )
	{
#ifdef WIN32
		gEngfuncs.Con_Printf( "[CFMODEngine::Init] Error loading FMOD functions! (%d)\n", GetLastError() );
#else
		gEngfuncs.Con_Printf( "[CFMODEngine::Init] Error loading FMOD functions! (%s)\n", dlerror() );
#endif
		return;
	}

	// Set some stuff first
	if ( IsVerboseDebuggingAllowed() )
		gEngfuncs.Con_Printf( "[CFMODEngine::Init] Creating sounds system...\n" );

	// Create the system dedicated to sounds
	m_pResult = m_pFuncSystemCreate( &m_pSystemSounds );
	if ( m_pResult == FMOD_OK )
	{
		// Check the version we are using
		if ( IsVerboseDebuggingAllowed() )
			gEngfuncs.Con_Printf( "[CFMODEngine::Init] Checking FMOD\'s version...\n" );

		unsigned int uiVersion = 0;
		m_pResult = m_pFuncSystemGetVersion( m_pSystemSounds, &uiVersion );
		if ( m_pResult == FMOD_OK )
		{
			if ( IsVerboseDebuggingAllowed() )
				gEngfuncs.Con_Printf( "[CFMODEngine::Init] User is running version %u, required version is %u...\n", uiVersion, FMOD_VERSION );

			if ( uiVersion == FMOD_VERSION )
				gEngfuncs.Con_Printf( "[CFMODEngine::Init] FMOD version is the same as the one used to build ARRANGEMENT (%u)\n", FMOD_VERSION );
			else if ( uiVersion > FMOD_VERSION )
				gEngfuncs.Con_Printf( "[CFMODEngine::Init] WARNING - FMOD version is higher than the one used to build ARRANGEMENT (%u > %u)\n", uiVersion, FMOD_VERSION );
			else
				gEngfuncs.Con_Printf( "[CFMODEngine::Init] WARNING - FMOD version is lower than the one used to build ARRANGEMENT (%u < %u)\n", uiVersion, FMOD_VERSION );

			// Set speaker mode for the sounds system
			if ( IsVerboseDebuggingAllowed() )
				gEngfuncs.Con_Printf( "[CFMODEngine::Init] Getting sampling rate of sounds system to set speaker mode...\n" );

			int iSampleRateForSounds;
			m_pResult = m_pFuncSystemGetSoftwareFormat( m_pSystemSounds, &iSampleRateForSounds, NULL, NULL );
			if ( m_pResult == FMOD_OK )
			{
				// Set speaker mode for the sounds system
				if ( IsVerboseDebuggingAllowed() )
					gEngfuncs.Con_Printf( "[CFMODEngine::Init] Sampling rate of sounds system found (%d), setting speaker mode on it...\n", iSampleRateForSounds );

				m_pResult = m_pFuncSystemSetSoftwareFormat( m_pSystemSounds, iSampleRateForSounds, GetSpeakerMode(), 0 );
				if ( m_pResult != FMOD_OK )
					ShowMessageBox( "Failed to set speaker mode for sounds!", "FMOD wasn\'t able to set the desired speaker mode for the sounds and this can cause issues with the sounds in-game.\n You may want to choose the \"auto select\" option to let FMOD select the best speaker mode for you.", false );
			}
			else
				ShowMessageBox( "Failed to retrieve sample rate of sounds system!", "FMOD wasn\'t able to retrieve the sample rate of your sound card from the sounds system.\nIf you asked FMOD to use a specific speaker mode, it will not be applied and the \"auto detect\" option will be used instead.", false );

			// Initialize the system dedicated to sounds
			if ( IsVerboseDebuggingAllowed() )
				gEngfuncs.Con_Printf( "[CFMODEngine::Init] Initializing sounds system...\n" );

			m_pResult = m_pFuncSystemInit( m_pSystemSounds, MAX_CHANNELS, FMOD_INIT_NORMAL, 0 );
			if ( m_pResult == FMOD_OK )
			{
				// Create the system dedicated to the music
				if ( IsVerboseDebuggingAllowed() )
					gEngfuncs.Con_Printf( "[CFMODEngine::Init] Creating musics system...\n" );

				m_pResult = m_pFuncSystemCreate( &m_pSystemMusics );
				if ( m_pResult == FMOD_OK )
				{
					// Set speaker mode for the musics system
					if ( IsVerboseDebuggingAllowed() )
						gEngfuncs.Con_Printf( "[CFMODEngine::Init] Getting sampling rate of musics system to set speaker mode...\n" );

					int iSampleRateForMusics;
					m_pResult = m_pFuncSystemGetSoftwareFormat( m_pSystemMusics, &iSampleRateForMusics, NULL, NULL );
					if ( m_pResult == FMOD_OK )
					{
						if ( IsVerboseDebuggingAllowed() )
							gEngfuncs.Con_Printf( "[CFMODEngine::Init] Sampling rate of musics system found (%d), setting speaker mode on it...\n", iSampleRateForMusics );

						m_pResult = m_pFuncSystemSetSoftwareFormat( m_pSystemMusics, iSampleRateForMusics, GetSpeakerMode(), 0 );
						if ( m_pResult != FMOD_OK )
							ShowMessageBox( "Failed to set speaker mode for musics!", "FMOD wasn\'t able to set the desired speaker mode for the musics and this can cause issues with the musics in-game.\nYou may want to choose the \"auto select\" option to let FMOD select the best speaker mode for you.", false );
					}
					else
						ShowMessageBox( "Failed to retrieve sample rate of musics system!", "FMOD wasn\'t able to retrieve the sample rate of your sound card from the musics system.\nIf you asked FMOD to use a specific speaker mode, it will not be applied and the \"auto detect\" option will be used instead.", false );

					// Initialize the system dedicated to the music
					if ( IsVerboseDebuggingAllowed() )
						gEngfuncs.Con_Printf( "[CFMODEngine::Init] Initializing musics system...\n" );

					m_pResult = m_pFuncSystemInit( m_pSystemMusics, 2, FMOD_INIT_NORMAL, 0 );
					if ( m_pResult == FMOD_OK )
					{
						// Set 3D settings
						if ( IsVerboseDebuggingAllowed() )
							gEngfuncs.Con_Printf( "[CFMODEngine::Init] Setting 3D parameters...\n" );

						m_pResult = m_pFuncSystemSet3DSettings( m_pSystemSounds, 1.0f, 1.0f, 1.0f );
						if ( m_pResult == FMOD_OK )
						{
							// Setup variables
							if ( IsVerboseDebuggingAllowed() )
								gEngfuncs.Con_Printf( "[CFMODEngine::Init] Final configuration and reading Half-Life\'s configuration in OS\'s registry for folders lookup...\n" );

							m_bAction = false;
							m_bPathsLookAtAddonsFolder = (ReadRegistry( FOLDER_ADDONS ) == 1) ? true : false;
							m_bPathsLookAtHDFolder = (ReadRegistry( FOLDER_HD ) == 1) ? true : false;
							m_flCurrentActionMP3Volume = 0.0f;
							m_flCurrentStealthMP3Volume = 0.0f;

							// Get ready to parse Half-Life's Steam manifest file for language detection
							// Did you know that this could detect if people uses Non-Steam version of Half-Life ? ^^
							/*if ( IsVerboseDebuggingAllowed() )
								gEngfuncs.Con_Printf( "[CFMODEngine::Init] Parsing Half-Life\'s Steam manifest for language detection...\n" );

							int iManifestLength = 0;
							const char *cManifestFile = (const char *)AM_LoadFile( "..\\..\\..\\appmanifest_70.acf", 5, &iManifestLength );

							// We managed to read the file
							if ( cManifestFile && iManifestLength > 0 )
							{
								// Copy the manifest's content to the buffer
								char cManifest[1024];
								sprintf( cManifest, "%s", cManifestFile );
								AM_FreeFile( (byte *)cManifestFile );

								// Get the language line
								char *cLine = strstr( cManifest, "\"language\"" );
								if ( cLine != NULL )
								{
									// We don't need the key name and the first quote of the value
									ReplaceStr( cLine, "\"language\"\t\t\"", "" );

									// Try to get the position of the last quote
									char *cPtr = strchr( cLine, '\"' );
									if ( cPtr != NULL )
									{
										// Remove that quote
										int iQuoteIndex = cPtr - cLine;
										cLine[iQuoteIndex] = '\0';
										strcpy( m_cLanguage, cLine );
										if ( IsVerboseDebuggingAllowed() )
											gEngfuncs.Con_Printf( "[CFMODEngine::Init] Detected language \"%s\"...\n", m_cLanguage );
									}
									else
										gEngfuncs.Con_Printf( "[CFMODEngine::Init] Unable to read Half-Life\'s settings! This can cause configurations mismatch between Half-Life and FMOD\n" );
								}
								else
									gEngfuncs.Con_Printf( "[CFMODEngine::Init] Unable to read Half-Life\'s settings! This can cause configurations mismatch between Half-Life and FMOD\n" );
							}
							else
							{
								gEngfuncs.Con_Printf( "[CFMODEngine::Init] Unable to read Half-Life\'s settings! This can cause configurations mismatch between Half-Life and FMOD\n" );
								AM_FreeFile( (byte *)cManifestFile );
							}*/

							// Get the pointer to the debug show CPU usage CVAR
							if ( IsVerboseDebuggingAllowed() )
								gEngfuncs.Con_Printf( "[CFMODEngine::Init] Getting CVARs pointers...\n" );

							m_cvDebugShowCPUUsage = gEngfuncs.pfnGetCvarPointer( "fmod_debug_show_cpu_usage" );
							if ( !m_cvDebugShowCPUUsage )
							{
								if ( IsVerboseDebuggingAllowed() )
									gEngfuncs.Con_Printf( "[CFMODEngine::Init] Failed to get pointer on CVAR \"fmod_debug_show_cpu_usage\"!\n" );
							}

							// Get the pointer to the SFX volume CVAR
							m_cvVolume = gEngfuncs.pfnGetCvarPointer( "volume" );
							if ( m_cvVolume )
								m_flVolume = m_cvVolume->value * 5.0f;
							else
							{
								if ( IsVerboseDebuggingAllowed() )
									gEngfuncs.Con_Printf( "[CFMODEngine::Init] Failed to get pointer on CVAR \"volume\"!\n" );
							}

							// Get the pointer to the music volume CVAR
							m_cvMP3Volume = gEngfuncs.pfnGetCvarPointer( "MP3Volume" );
							if ( m_cvMP3Volume )
								m_flDesiredMP3Volume = m_flCurrentStealthMP3Volume = m_cvMP3Volume->value;
							else
							{
								if ( IsVerboseDebuggingAllowed() )
									gEngfuncs.Con_Printf( "[CFMODEngine::Init] Failed to get pointer on CVAR \"MP3Volume\"!\n" );
							}

							// Set output mode
							if ( IsVerboseDebuggingAllowed() )
								gEngfuncs.Con_Printf( "[CFMODEngine::Init] Setting output mode on systems...\n" );

							m_pResult = m_pFuncSystemSetOutput( m_pSystemSounds, GetOutputMode() );
							if ( m_pResult != FMOD_OK )
								ShowMessageBox( "Failed to set output mode for sounds!", "FMOD wasn\'t able to set the desired output mode for the sounds and this can cause issues with the sounds in-game.\n You may want to choose the \"auto select\" option to let FMOD select the best output mode for you.", false );

							m_pResult = m_pFuncSystemSetOutput( m_pSystemMusics, GetOutputMode() );
							if ( m_pResult != FMOD_OK )
								ShowMessageBox( "Failed to set output mode for musics!", "FMOD wasn\'t able to set the desired output mode for the musics and this can cause issues with the musics in-game.\n You may want to choose the \"auto select\" option to let FMOD select the best output mode for you.", false );

							// Everything is good, we can proceed!
							if ( IsVerboseDebuggingAllowed() )
								gEngfuncs.Con_Printf( "[CFMODEngine::Init] Everything seems to be in order!\n" );

							m_bInitialized = true;
						}
						else
							ShowMessageBox( "Failed to set 3D parameters for FMOD!", "FMOD wasn\'t able to set the 3D parameters.", true );
					}
					else
						ShowMessageBox( "Failed to initialize FMOD system for music!", "The FMOD system for music couldn\'t be initialized.", true );
				}
				else
					ShowMessageBox( "Creation of the FMOD system for the music has failed!", "The FMOD system for the music couldn\'t be created.", true );
			}
			else
				ShowMessageBox( "Failed to initialize FMOD system for sounds!", "The FMOD system for sounds couldn\'t be initialized.", true );
		}
		else
			ShowMessageBox( "Unable to retrieve FMOD\'s version!", "The engine failed to retrieve FMOD\'s version.", true );
	}
	else
		ShowMessageBox( "Creation of the FMOD system for sounds has failed!", "The FMOD system for sounds couldn\'t be created.", true );
}

// Reset - Reset the engine
void CFMODEngine::Reset( void )
{
	if ( IsVerboseDebuggingAllowed() )
		gEngfuncs.Con_Printf( "[CFMODEngine::Reset] Reset in progress...\n" );

	m_iNumEntities = 0;
}

// Shutdown - Shutdown the system
void CFMODEngine::Shutdown( void )
{
	// Release all the 2D sounds inside the 2D sounds factory
	if ( IsVerboseDebuggingAllowed() )
		gEngfuncs.Con_Printf( "[CFMODEngine::Shutdown] Shutting down all 2D sounds...\n" );

	int i = 0;
	for ( i = 0; i < m_p2DSoundsFactory.size(); i++ )
	{
		CFMOD2DSound *pSoundToRelease = m_p2DSoundsFactory[i];
		m_p2DSoundsFactory.erase( m_p2DSoundsFactory.begin() + i );
		delete pSoundToRelease;
	}
	m_p2DSoundsFactory.empty();

	// Do the same for 3D sounds
	if ( IsVerboseDebuggingAllowed() )
		gEngfuncs.Con_Printf( "[CFMODEngine::Shutdown] Shutting down all 3D sounds...\n" );

	for ( i = 0; i < m_p3DSoundsFactory.size(); i++ )
	{
		CFMOD3DSound *pSoundToRelease = m_p3DSoundsFactory[i];
		m_p3DSoundsFactory.erase( m_p3DSoundsFactory.begin() + i );
		delete pSoundToRelease;
	}
	m_p3DSoundsFactory.empty();

	// Release the action music
	if ( m_pMusicAction )
	{
		if ( IsVerboseDebuggingAllowed() )
			gEngfuncs.Con_Printf( "[CFMODEngine::Shutdown] Shutting down action music...\n" );

		m_pResult = m_pFuncSoundRelease( m_pMusicAction );
		if ( (m_pResult != FMOD_OK) && IsVerboseDebuggingAllowed() )
			gEngfuncs.Con_Printf( "[CFMODEngine::Shutdown] Shutting down action music failed! (%s)\n", FMOD_ErrorString( m_pResult ) );
	}

	// Release the stealth music
	if ( m_pMusicStealth )
	{
		if ( IsVerboseDebuggingAllowed() )
			gEngfuncs.Con_Printf( "[CFMODEngine::Shutdown] Shutting down stealth music...\n" );

		m_pResult = m_pFuncSoundRelease( m_pMusicStealth );
		if ( (m_pResult != FMOD_OK) && IsVerboseDebuggingAllowed() )
			gEngfuncs.Con_Printf( "[CFMODEngine::Shutdown] Shutting down stealth music failed! (%s)\n", FMOD_ErrorString( m_pResult ) );
	}

	// Release the systems
	if ( IsVerboseDebuggingAllowed() )
		gEngfuncs.Con_Printf( "[CFMODEngine::Shutdown] Shutting down musics system...\n" );

	m_pResult = m_pFuncSystemRelease( m_pSystemMusics );
	if ( (m_pResult != FMOD_OK) && IsVerboseDebuggingAllowed() )
		gEngfuncs.Con_Printf( "[CFMODEngine::Shutdown] Shutting down musics system failed! (%s)\n", FMOD_ErrorString( m_pResult ) );

	if ( IsVerboseDebuggingAllowed() )
		gEngfuncs.Con_Printf( "[CFMODEngine::Shutdown] Shutting down sounds system...\n" );

	m_pResult = m_pFuncSystemRelease( m_pSystemSounds );
	if ( (m_pResult != FMOD_OK) && IsVerboseDebuggingAllowed() )
		gEngfuncs.Con_Printf( "[CFMODEngine::Shutdown] Shutting down sounds system failed! (%s)\n", FMOD_ErrorString( m_pResult ) );

	m_pFuncChannelIsPlaying = NULL;
	m_pFuncChannelSet3DAttributes = NULL;
	m_pFuncChannelSet3DSpread = NULL;
	m_pFuncChannelSetPaused = NULL;
	m_pFuncChannelSetPitch = NULL;
	m_pFuncChannelSetVolume = NULL;
	m_pFuncChannelStop = NULL;
	m_pFuncSoundSet3DMinMaxDistance = NULL;
	m_pFuncSoundRelease = NULL;
	m_pFuncSystemCreate = NULL;
	m_pFuncSystemCreateSound = NULL;
	m_pFuncSystemCreateStream = NULL;
	m_pFuncSystemGetCPUUsage = NULL;
	m_pFuncSystemGetSoftwareFormat = NULL;
	m_pFuncSystemGetVersion = NULL;
	m_pFuncSystemInit = NULL;
	m_pFuncSystemPlaySound = NULL;
	m_pFuncSystemRelease = NULL;
	m_pFuncSystemSet3DListenerAttributes = NULL;
	m_pFuncSystemSet3DSettings = NULL;
	m_pFuncSystemSetOutput = NULL;
	m_pFuncSystemSetReverbProperties = NULL;
	m_pFuncSystemSetSoftwareFormat = NULL;
	m_pFuncSystemUpdate = NULL;

#ifdef WIN32
	FreeLibrary( (HMODULE)m_pFMODLib );
#else
	dlclose( m_pFMODLib );
#endif

	m_bInitialized = false;
}

// Update - Called every frame to update the engine
void CFMODEngine::Update( void )
{
	// Update the systems
	m_pResult = m_pFuncSystemUpdate( m_pSystemMusics );
	if ( (m_pResult != FMOD_OK) && IsVerboseDebuggingAllowed() )
		gEngfuncs.Con_Printf( "[CFMODEngine::Update] Updating musics system failed! (%s)\n", FMOD_ErrorString( m_pResult ) );

	m_pResult = m_pFuncSystemUpdate( m_pSystemSounds );
	if ( (m_pResult != FMOD_OK) && IsVerboseDebuggingAllowed() )
		gEngfuncs.Con_Printf( "[CFMODEngine::Update] Updating sounds system failed! (%s)\n", FMOD_ErrorString( m_pResult ) );

	// Check if ARRANGEMENT is the focused window
	bool bIsFocused = true;
#ifdef WIN32
	if ( (int)gHUD.m_pCvarFMODMuteWhenNotFocused->value != 0 )
	{
		HWND hActiveWindow = GetForegroundWindow();
		if ( hActiveWindow != NULL )
		{
			char cWindowTitle[64] = "";
			LRESULT result = SendMessage( hActiveWindow, WM_GETTEXT, 64, reinterpret_cast<LPARAM>( cWindowTitle ) );
			bIsFocused = (strcmp( cWindowTitle, "ARRANGEMENT - NEW CLIENT" ) == 0); // This must be the same as "game" in liblist.gam
		}
	}
#endif

	// Get the volume for SFXs
	if ( m_cvVolume )
		m_flVolume = bIsFocused ? m_cvVolume->value * 5.0f : 0.0f;
	else
		m_flVolume = bIsFocused ? 0.2f : 0.0f;

	// Get the volume for musics
	if ( m_cvMP3Volume )
		m_flDesiredMP3Volume = bIsFocused ? m_cvMP3Volume->value : 0.0f;
	else
		m_flDesiredMP3Volume = bIsFocused ? 0.2f : 0.0f;

	// Fade the volumes when the state changes
	if ( m_bAction )
	{
		// Higher the action music volume
		if ( m_flCurrentActionMP3Volume < m_flDesiredMP3Volume )
			m_flCurrentActionMP3Volume += 0.01f;
		else
			m_flCurrentActionMP3Volume = m_flDesiredMP3Volume;

		// Lower the stealth music volume
		if ( m_flCurrentStealthMP3Volume > 0.0f )
			m_flCurrentStealthMP3Volume -= 0.01f;
		else
			m_flCurrentStealthMP3Volume = 0.0f;
	}
	else
	{
		// Higher the stealth music volume
		if ( m_flCurrentStealthMP3Volume < m_flDesiredMP3Volume )
			m_flCurrentStealthMP3Volume += 0.01f;
		else
			m_flCurrentStealthMP3Volume = m_flDesiredMP3Volume;

		// Lower the action music volume
		if ( m_flCurrentActionMP3Volume > 0.0f )
			m_flCurrentActionMP3Volume -= 0.01f;
		else
			m_flCurrentActionMP3Volume = 0.0f;
	}

	// Update the musics channels volumes
	if ( m_pChannelMusicAction )
	{
		m_pResult = m_pFuncChannelSetVolume( m_pChannelMusicAction, m_flCurrentActionMP3Volume );
		if ( (m_pResult != FMOD_OK) && IsVerboseDebuggingAllowed() )
			gEngfuncs.Con_Printf( "[CFMODEngine::Update] Updating action music volume failed! (%s)\n", FMOD_ErrorString( m_pResult ) );
	}

	if ( m_pChannelMusicStealth )
	{
		m_pResult = m_pFuncChannelSetVolume( m_pChannelMusicStealth, m_flCurrentStealthMP3Volume );
		if ( (m_pResult != FMOD_OK) && IsVerboseDebuggingAllowed() )
			gEngfuncs.Con_Printf( "[CFMODEngine::Update] Updating stealth music volume failed! (%s)\n", FMOD_ErrorString( m_pResult ) );
	}

	// Update all the playing 2D sounds (thanks Tinithrari for helping me with memory leaks)
	for ( int i = 0; i < m_p2DSoundsFactory.size(); i++ )
	{
		if ( m_p2DSoundsFactory[i] != NULL )
		{
			m_p2DSoundsFactory[i]->Update();
			// Sound has finished playing ? Release it
			if ( !m_p2DSoundsFactory[i]->IsPlaying() )
			{
				CFMOD2DSound *pSoundToRelease = m_p2DSoundsFactory[i];
				m_p2DSoundsFactory.erase( m_p2DSoundsFactory.begin() + i );
				delete pSoundToRelease;
			}
		}
	}

	// Update all the playing 3D sounds (thanks Tinithrari for helping me with memory leaks)
	for ( int i = 0; i < m_p3DSoundsFactory.size(); i++ )
	{
		if ( m_p3DSoundsFactory[i] != NULL )
		{
			m_p3DSoundsFactory[i]->Update();
			// Sound has finished playing ? Release it
			if ( !m_p3DSoundsFactory[i]->IsPlaying() )
			{
				CFMOD3DSound *pSoundToRelease = m_p3DSoundsFactory[i];
				m_p3DSoundsFactory.erase( m_p3DSoundsFactory.begin() + i );
				delete pSoundToRelease;
			}
		}
	}

	// TODO: Check if this can be moved in Reset
	// I think that task should be done by the server and not the client.
	// But is it really necessary to reset the reverberation ? Why I wrote that ?
	if ( strcmp( m_cLastMap, gEngfuncs.pfnGetLevelName() ) != 0 )
	{
		FMOD_REVERB_PROPERTIES pNoReverb = FMOD_PRESET_OFF;
		m_pResult = m_pFuncSystemSetReverbProperties( m_pSystemSounds, 0, &pNoReverb );
		if ( m_pResult != FMOD_OK )
			gEngfuncs.Con_Printf( "[CFMODEngine::Update] Reset reverberation failed! (%s)\n", FMOD_ErrorString( m_pResult ) );

		sprintf( m_cLastMap, "%s", gEngfuncs.pfnGetLevelName() );
	}

	// Show CPU usage if we asked to
	if ( m_cvDebugShowCPUUsage )
	{
		if ( (int)m_cvDebugShowCPUUsage->value != 0 )
		{
			float flMusicsCPU, flMusicsStream, flMusicsUpdate, flMusicsTotal;
			float flSoundsCPU, flSoundsStream, flSoundsUpdate, flSoundsTotal;
			m_pResult = m_pFuncSystemGetCPUUsage( m_pSystemMusics, &flMusicsCPU, &flMusicsStream, 0, &flMusicsUpdate, &flMusicsTotal );
			if ( (m_pResult != FMOD_OK) && IsVerboseDebuggingAllowed() )
				gEngfuncs.Con_Printf( "[CFMODEngine::Update] Getting CPU usage on musics system failed! (%s)\n", FMOD_ErrorString( m_pResult ) );

			m_pResult = m_pFuncSystemGetCPUUsage( m_pSystemSounds, &flSoundsCPU, &flSoundsStream, 0, &flSoundsUpdate, &flSoundsTotal );
			if ( (m_pResult != FMOD_OK) && IsVerboseDebuggingAllowed() )
				gEngfuncs.Con_Printf( "[CFMODEngine::Update] Getting CPU usage on sounds system failed! (%s)\n", FMOD_ErrorString( m_pResult ) );

			gEngfuncs.Con_Printf( "CPU : %.2f/%.2f, Stream : %.2f/%.2f, Update : %.2f/%.2f, Total : %.2f/%.2f\n", flMusicsCPU, flSoundsCPU, flMusicsStream, flSoundsStream, flMusicsUpdate, flSoundsUpdate, flMusicsTotal, flSoundsTotal );
		}
	}
	m_iNumEntities = 0;

	// Update the listener (player) position
	cl_entity_t *pPlayer = gEngfuncs.GetLocalPlayer();
	if ( !pPlayer )
	{
		if ( IsVerboseDebuggingAllowed() )
			gEngfuncs.Con_Printf( "[CFMODEngine::Update] pPlayer is NULL! Can\'t set the position in 3D space\n" );

		return;
	}
	FMOD_VECTOR vecPlayerPos = { pPlayer->origin.x, pPlayer->origin.y, pPlayer->origin.z };
	m_pResult = m_pFuncSystemSet3DListenerAttributes( m_pSystemSounds, 0, &vecPlayerPos, 0, 0, 0 );
	if ( (m_pResult != FMOD_OK) && IsVerboseDebuggingAllowed() )
		gEngfuncs.Con_Printf( "[CFMODEngine::Update] Updating player position failed! (%s)\n", FMOD_ErrorString( m_pResult ) );
}

//--------------------------------------------------
//--------------------------------------------------
//--------------------------------------------------

// Play2DSound - Play a 2D sound
void CFMODEngine::Play2DSound( const char *cSound, const float flPitch )
{
	bool bOk = false;
	CFMOD2DSound *pSound = new CFMOD2DSound( m_pSystemSounds, cSound, bOk, flPitch, m_bPathsLookAtAddonsFolder, m_bPathsLookAtHDFolder, m_cLanguage );
	if ( !bOk )
		return;

	m_p2DSoundsFactory.push_back( pSound );
}

// Play3DSoundAtEntity - Play a 3D sound where it's origin is based on an entity position
void CFMODEngine::Play3DSoundAtEntity( const int iIndex, const char *cSound, const float flMinDistance, float flMaxDistance, const bool bFollowEntity, const float flPitch )
{
	// Check if the entity is valid
	cl_entity_t *pEntity = gEngfuncs.GetEntityByIndex( iIndex );
	if ( !pEntity )
	{
		gEngfuncs.Con_Printf( "[CFMODEngine::Play3DSoundAtEntity] Attempting to play sound on a non-existing entity! This attempt has been denied!\n" );
		return;
	}
	Play3DSoundAtPosition( pEntity->origin, cSound, flMinDistance, flMaxDistance, (bFollowEntity) ? iIndex : -1, flPitch );
}

// Play3DSoundAtPosition - Play a 3D sound at a specified position
void CFMODEngine::Play3DSoundAtPosition( const Vector vecPosition, const char *cSound, const float flMinDistance, float flMaxDistance, const int iEntityIndexToFollow, const float flPitch )
{
	// Set the distance
	if ( flMaxDistance == -1.0f )
		flMaxDistance = flMinDistance * 1.5f;

	bool bOk = false;
	CFMOD3DSound *pSound = new CFMOD3DSound( m_pSystemSounds, vecPosition, bOk, cSound, flMinDistance, flMaxDistance, iEntityIndexToFollow, flPitch, m_bPathsLookAtAddonsFolder, m_bPathsLookAtHDFolder, m_cLanguage );
	if ( !bOk )
		return;

	m_p3DSoundsFactory.push_back( pSound );
}

// SetReverb - Set the reverberation in the system dedicated to sounds
void CFMODEngine::SetReverb( const int iReverbType )
{
	// Something very funny to note here.
	// For Revenge of Love, I use VS2010 Ultimate SP1 and here I use VS2013 Ultimate Update 5
	// This code will fail on Revenge of Love because the compiler seems to dislike when I use
	// a single "FMOD_REVERB_PROPERTIES" to rule them all.
	// However, on VS2013, everything is working like a charm.
	// I'm not a huge fan of VS2013 and the GitHub HL SDK has been worked with VS2010 (the reason
	// of why I stick with VS2010 on RoL)
	// So if you are using VS2010 (but you shouldn't because Am has been converted for VS2013), you
	// will have to "if, else if and else" the crap out of this code.

	// Set the type
	FMOD_REVERB_PROPERTIES pProperties;
	switch ( iReverbType )
	{
	case 1: pProperties = FMOD_PRESET_GENERIC; break;
	case 2: pProperties = FMOD_PRESET_PADDEDCELL; break;
	case 3: pProperties = FMOD_PRESET_ROOM; break;
	case 4: pProperties = FMOD_PRESET_BATHROOM; break;
	case 5: pProperties = FMOD_PRESET_LIVINGROOM; break;
	case 6: pProperties = FMOD_PRESET_STONEROOM; break;
	case 7: pProperties = FMOD_PRESET_AUDITORIUM; break;
	case 8: pProperties = FMOD_PRESET_CONCERTHALL; break;
	case 9: pProperties = FMOD_PRESET_CAVE; break;
	case 10: pProperties = FMOD_PRESET_ARENA; break;
	case 11: pProperties = FMOD_PRESET_HANGAR; break;
	case 12: pProperties = FMOD_PRESET_CARPETTEDHALLWAY; break;
	case 13: pProperties = FMOD_PRESET_HALLWAY; break;
	case 14: pProperties = FMOD_PRESET_STONECORRIDOR; break;
	case 15: pProperties = FMOD_PRESET_ALLEY; break;
	case 16: pProperties = FMOD_PRESET_FOREST; break;
	case 17: pProperties = FMOD_PRESET_CITY; break;
	case 18: pProperties = FMOD_PRESET_MOUNTAINS; break;
	case 19: pProperties = FMOD_PRESET_QUARRY; break;
	case 20: pProperties = FMOD_PRESET_PLAIN; break;
	case 21: pProperties = FMOD_PRESET_PARKINGLOT; break;
	case 22: pProperties = FMOD_PRESET_SEWERPIPE; break;
	case 23: pProperties = FMOD_PRESET_UNDERWATER; break;
	default: pProperties = FMOD_PRESET_OFF;
	}
	m_pResult = m_pFuncSystemSetReverbProperties( m_pSystemSounds, 0, &pProperties );
	if ( (m_pResult != FMOD_OK) && IsVerboseDebuggingAllowed() )
		gEngfuncs.Con_Printf( "[CFMODEngine::SetReverb] Setting reverberation failed! (%s)\n", FMOD_ErrorString( m_pResult ) );
}

//--------------------------------------------------
//--------------------------------------------------
//--------------------------------------------------

// PlayMusic - Play the stealth and action musics or a specific one
// TODO : When bLooped is set to false, the music is looped which is bad
// I should have ported the "safer" version from Revenge of Love a long time ago
// instead of waiting this bad shit to happen.
void CFMODEngine::PlayMusic( const char *cStream, const bool bLooped, const bool bActionMusic )
{
	// Stop already playing music (if there is)
	StopMusic( (bActionMusic) ? true : false );

	// Get the "default" path to the stream
	char cPath[64];
	sprintf( cPath, "%s/sound/fmod/%s", gEngfuncs.pfnGetGameDirectory(), cStream );

	// Check in "am_downloads" folder
	ReplaceStr( cPath, "am", "am_downloads" );
	m_pResult = m_pFuncSystemCreateStream( m_pSystemMusics, cPath, FMOD_2D | (bLooped) ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF, 0, (bActionMusic) ? &m_pMusicAction : &m_pMusicStealth );
	// Not found? Check in "valve_downloads" then
	if ( m_pResult != FMOD_OK )
	{
		m_pResult = (bActionMusic) ? m_pFuncSoundRelease( m_pMusicAction ) : m_pFuncSoundRelease( m_pMusicStealth );
		ReplaceStr( cPath, "am_downloads", "valve_downloads" );
		m_pResult = m_pFuncSystemCreateStream( m_pSystemMusics, cPath, FMOD_2D | (bLooped) ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF, 0, (bActionMusic) ? &m_pMusicAction : &m_pMusicStealth );
		// Not found? Reset for addons check
		if ( m_pResult != FMOD_OK )
		{
			m_pResult = (bActionMusic) ? m_pFuncSoundRelease( m_pMusicAction ) : m_pFuncSoundRelease( m_pMusicStealth );
			sprintf( cPath, "%s/sound/fmod/%s", gEngfuncs.pfnGetGameDirectory(), cStream );
		}
	}

	// Check in "am_addon" folder
	if ( m_bPathsLookAtAddonsFolder && (m_pResult != FMOD_OK) )
	{
		ReplaceStr( cPath, "am", "am_addon" );
		m_pResult = m_pFuncSystemCreateStream( m_pSystemMusics, cPath, FMOD_2D | (bLooped) ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF, 0, (bActionMusic) ? &m_pMusicAction : &m_pMusicStealth );
		// Not found? Check in "valve_addon" then
		if ( m_pResult != FMOD_OK )
		{
			m_pResult = (bActionMusic) ? m_pFuncSoundRelease( m_pMusicAction ) : m_pFuncSoundRelease( m_pMusicStealth );
			ReplaceStr( cPath, "am_addon", "valve_addon" );
			m_pResult = m_pFuncSystemCreateStream( m_pSystemMusics, cPath, FMOD_2D | (bLooped) ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF, 0, (bActionMusic) ? &m_pMusicAction : &m_pMusicStealth );
			// Not found? Reset for HD check
			if ( m_pResult != FMOD_OK )
			{
				m_pResult = (bActionMusic) ? m_pFuncSoundRelease( m_pMusicAction ) : m_pFuncSoundRelease( m_pMusicStealth );
				sprintf( cPath, "%s/sound/fmod/%s", gEngfuncs.pfnGetGameDirectory(), cStream );
			}
		}
	}

	// Check in "am_hd" folder
	if ( m_bPathsLookAtHDFolder && (m_pResult != FMOD_OK) )
	{
		ReplaceStr( cPath, "am", "am_hd" );
		m_pResult = m_pFuncSystemCreateStream( m_pSystemMusics, cPath, FMOD_2D | (bLooped) ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF, 0, (bActionMusic) ? &m_pMusicAction : &m_pMusicStealth );
		// Not found? Check in "valve_hd" then
		if ( m_pResult != FMOD_OK )
		{
			m_pResult = (bActionMusic) ? m_pFuncSoundRelease( m_pMusicAction ) : m_pFuncSoundRelease( m_pMusicStealth );
			ReplaceStr( cPath, "am_hd", "valve_hd" );
			m_pResult = m_pFuncSystemCreateSound( m_pSystemMusics, cPath, FMOD_2D | (bLooped) ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF, 0, (bActionMusic) ? &m_pMusicAction : &m_pMusicStealth );
			// Not found? Reset for language check
			if ( m_pResult != FMOD_OK )
			{
				m_pResult = (bActionMusic) ? m_pFuncSoundRelease( m_pMusicAction ) : m_pFuncSoundRelease( m_pMusicStealth );
				sprintf( cPath, "%s/sound/fmod/%s", gEngfuncs.pfnGetGameDirectory(), cStream );
			}
		}
	}

	// Check in "am_lang" folder
	if ( (strcmp( m_cLanguage, "english" ) != 0) && (m_pResult != FMOD_OK) )
	{
		strcat( cPath, "_" );
		strcat( cPath, m_cLanguage );
		m_pResult = m_pFuncSystemCreateStream( m_pSystemMusics, cPath, FMOD_2D | (bLooped) ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF, 0, (bActionMusic) ? &m_pMusicAction : &m_pMusicStealth );
		// Not found? Check in "valve_addon" then
		if ( m_pResult != FMOD_OK )
		{
			m_pResult = (bActionMusic) ? m_pFuncSoundRelease( m_pMusicAction ) : m_pFuncSoundRelease( m_pMusicStealth );
			ReplaceStr( cPath, "am", "valve" );
			m_pResult = m_pFuncSystemCreateStream( m_pSystemMusics, cPath, FMOD_2D | (bLooped) ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF, 0, (bActionMusic) ? &m_pMusicAction : &m_pMusicStealth );
			// Not found? Reset for root check
			if ( m_pResult != FMOD_OK )
			{
				m_pResult = (bActionMusic) ? m_pFuncSoundRelease( m_pMusicAction ) : m_pFuncSoundRelease( m_pMusicStealth );
				sprintf( cPath, "%s/sound/fmod/%s", gEngfuncs.pfnGetGameDirectory(), cStream );
			}
		}
	}

	// Check in "am" folder
	m_pResult = m_pFuncSystemCreateStream( m_pSystemMusics, cPath, FMOD_2D | FMOD_LOOP_NORMAL, 0, (bActionMusic) ? &m_pMusicAction : &m_pMusicStealth );
	if ( m_pResult == FMOD_ERR_FILE_NOTFOUND )
	{
		m_pResult = (bActionMusic) ? m_pFuncSoundRelease( m_pMusicAction ) : m_pFuncSoundRelease( m_pMusicStealth );
		ReplaceStr( cPath, "am", "valve" );
		m_pResult = m_pFuncSystemCreateStream( m_pSystemMusics, cPath, FMOD_2D | (bLooped) ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF, 0, (bActionMusic) ? &m_pMusicAction : &m_pMusicStealth );
	}
	if ( m_pResult == FMOD_OK )
	{
		// Play the music
		m_pResult = m_pFuncSystemPlaySound( m_pSystemMusics, (bActionMusic) ? m_pMusicAction : m_pMusicStealth, 0, true, (bActionMusic) ? &m_pChannelMusicAction : &m_pChannelMusicStealth );
		if ( m_pResult == FMOD_OK )
		{
			if ( bActionMusic )
			{
				m_pResult = m_pFuncChannelSetVolume( m_pChannelMusicAction, m_flCurrentActionMP3Volume );
				if ( (m_pResult != FMOD_OK) && IsVerboseDebuggingAllowed() )
					gEngfuncs.Con_Printf( "[CFMODEngine::PlayMusic] Failed to set action music volume! (%s)\n", FMOD_ErrorString( m_pResult ) );

				m_pResult = m_pFuncChannelSetPaused( m_pChannelMusicAction, false );
				if ( (m_pResult != FMOD_OK) && IsVerboseDebuggingAllowed() )
					gEngfuncs.Con_Printf( "[CFMODEngine::PlayMusic] Failed to set action pause state! (%s)\n", FMOD_ErrorString( m_pResult ) );
			}
			else
			{
				m_pResult = m_pFuncChannelSetVolume( m_pChannelMusicStealth, m_flCurrentStealthMP3Volume );
				if ( (m_pResult != FMOD_OK) && IsVerboseDebuggingAllowed() )
					gEngfuncs.Con_Printf( "[CFMODEngine::PlayMusic] Failed to set stealth music volume! (%s)\n", FMOD_ErrorString( m_pResult ) );

				m_pResult = m_pFuncChannelSetPaused( m_pChannelMusicStealth, false );
				if ( (m_pResult != FMOD_OK) && IsVerboseDebuggingAllowed() )
					gEngfuncs.Con_Printf( "[CFMODEngine::PlayMusic] Failed to set stealth pause state! (%s)\n", FMOD_ErrorString( m_pResult ) );
			}
		}
		else
		{
			if ( IsVerboseDebuggingAllowed() )
				gEngfuncs.Con_Printf( "[CFMODEngine::PlayMusic] Failed to play %s music! (%s)\n", (bActionMusic ? "action" : "ambient"), FMOD_ErrorString( m_pResult ) );
		}
	}
	else
		gEngfuncs.Con_Printf( "[CFMODEngine::PlayMusic] Failed to play %s stream \"%s\"!\n", (bActionMusic) ? "action" : "stealth", cStream );
}

// StopMusic - Stop the specified music if it's playing
void CFMODEngine::StopMusic( bool bActionMusic )
{
	// Check if a music is playing first
	FMOD_BOOL bIsPlaying = false;
	if ( bActionMusic )
	{
		m_pResult = m_pFuncChannelIsPlaying( m_pChannelMusicAction, &bIsPlaying );
		if ( (m_pResult != FMOD_OK) && IsVerboseDebuggingAllowed() )
			gEngfuncs.Con_Printf( "[CFMODEngine::StopMusic] Getting action music playing status failed! (%s)\n", FMOD_ErrorString( m_pResult ) );
	}
	else
	{
		m_pResult = m_pFuncChannelIsPlaying( m_pChannelMusicStealth, &bIsPlaying );
		if ( (m_pResult != FMOD_OK) && IsVerboseDebuggingAllowed() )
			gEngfuncs.Con_Printf( "[CFMODEngine::StopMusic] Getting stealth music playing status failed! (%s)\n", FMOD_ErrorString( m_pResult ) );
	}

	// It's the case? Then stop it first
	if ( m_pResult == FMOD_OK )
	{
		if ( bIsPlaying )
		{
			if ( bActionMusic )
			{
				m_pResult = m_pFuncSoundRelease( m_pMusicAction );
				if ( (m_pResult != FMOD_OK) && IsVerboseDebuggingAllowed() )
					gEngfuncs.Con_Printf( "[CFMODEngine::StopMusic] Releasing action music failed! (%s)\n", FMOD_ErrorString( m_pResult ) );
			}
			else
			{
				m_pResult = m_pFuncSoundRelease( m_pMusicStealth );
				if ( (m_pResult != FMOD_OK) && IsVerboseDebuggingAllowed() )
					gEngfuncs.Con_Printf( "[CFMODEngine::StopMusic] Releasing stealth music failed! (%s)\n", FMOD_ErrorString( m_pResult ) );
			}
		}
	}
}

//--------------------------------------------------
//--------------------------------------------------
//--------------------------------------------------

// GetOutputMode - Read the "output mode"'s CVAR and apply it to FMOD
FMOD_OUTPUTTYPE CFMODEngine::GetOutputMode( void )
{
	if ( (int)gHUD.m_pCvarFMODOutput->value == 1 )
	{
#ifdef WIN32
		return FMOD_OUTPUTTYPE_DSOUND; // DirectSound (default on WinXP and below)
#else
		return FMOD_OUTPUTTYPE_PULSEAUDIO; // PulseAudio (default on Linux)
#endif
	}
	else if ( (int)gHUD.m_pCvarFMODOutput->value == 2 )
	{
#ifdef WIN32
		return FMOD_OUTPUTTYPE_WINMM; // Windows Multimedia
#else
		return FMOD_OUTPUTTYPE_ALSA; // Advanced Linux Sound Architecture (default on Linux if PulseAudio isn't available)
#endif
	}
#ifdef WIN32
	else if ( (int)gHUD.m_pCvarFMODOutput->value == 3 )
		return FMOD_OUTPUTTYPE_WASAPI; // Windows Audio Session API (default on WinVista and higher)
	else if ( (int)gHUD.m_pCvarFMODOutput->value == 4 )
		return FMOD_OUTPUTTYPE_ASIO; // ASIO 2.0
#endif
	else
		return FMOD_OUTPUTTYPE_AUTODETECT;
}

// GetSpeakerMode - Read the "speaker mode"'s CVAR and apply it to FMOD
FMOD_SPEAKERMODE CFMODEngine::GetSpeakerMode( void )
{
	switch ( (int)gHUD.m_pCvarFMODSpeaker->value )
	{
		case 1:		return FMOD_SPEAKERMODE_MONO;		break;	// Mono
		case 2:		return FMOD_SPEAKERMODE_STEREO;		break;	// Stereo
		case 3:		return FMOD_SPEAKERMODE_QUAD;		break;	// Quad
		case 4:		return FMOD_SPEAKERMODE_SURROUND;	break;	// Surround
		case 5:		return FMOD_SPEAKERMODE_5POINT1;	break;	// 5.1
		case 6:		return FMOD_SPEAKERMODE_7POINT1;	break;	// 7.1
		default:	return FMOD_SPEAKERMODE_DEFAULT;			// Use OS's config
	}
}

// ReadRegistry - Read the OS's "registry" to read Half-Life's configuration
int CFMODEngine::ReadRegistry( folders_e eSettingToLookAt )
{
#ifdef WIN32
	HKEY hRegistryKey;
	LONG lRes = RegOpenKeyExW( HKEY_CURRENT_USER, L"SOFTWARE\\Valve\\Half-Life\\Settings\\", 0, KEY_READ, &hRegistryKey );
	if ( lRes != ERROR_SUCCESS )
	{
		gEngfuncs.Con_Printf( "[CFMODEngine::ReadRegistry] Unable to read Half-Life\'s settings! This can cause configurations mismatch between Half-Life and FMOD\n" );
		return 0;
	}

	std::wstring sValueName;
	switch ( eSettingToLookAt )
	{
	case FOLDER_ADDONS: sValueName = L"addons_folder"; break;
	case FOLDER_HD: sValueName = L"hdmodels"; break;
	}

	DWORD dwBufferSize = sizeof( DWORD );
	DWORD dwResult = 0;
	LONG lnError = RegQueryValueExW( hRegistryKey, sValueName.c_str(), 0, NULL, reinterpret_cast<LPBYTE>(&dwResult), &dwBufferSize );
	if ( lnError != ERROR_SUCCESS )
	{
		gEngfuncs.Con_Printf( "[CFMODEngine::ReadRegistry] Unable to read Half-Life\'s settings! This can cause configurations mismatch between Half-Life and FMOD\n" );
		return 0;
	}
	return dwResult;
#else
	// Linux doesn't have a registry like Windows does.
	// Those settings are stored in "hl.conf" in the Half-Life folder.
	// It's much easier than Windows because I just need to look for those lines "hdmodels=X" and "addons_folder=X" and parse them.
	// Will do this later, Linux is not a priority at the moment.
	// Will also have to look how it's done on Mac OS (but I don't have a Mac)
	return 0;
#endif
}

// ShowMessageBox - Signal FMOD's critical errors/warnings and close the game (if it's an error)
void CFMODEngine::ShowMessageBox( const char* cTitle, const char* cMessage, const bool bIsError )
{
	gEngfuncs.Con_Printf( "[CFMODEngine::ShowMessageBox] %s\n", cMessage );
	if ( bIsError )
	{
		SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, cTitle, cMessage, NULL );
		gEngfuncs.pfnClientCmd( "quit\n" );
	}
	else
		SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_WARNING, cTitle, cMessage, NULL );
}

//==================================================
//==================== 2D SOUND ====================
//==================================================

// CFMOD2DSound - Create a 2D sound
CFMOD2DSound::CFMOD2DSound( FMOD_SYSTEM *pSystem, const char *cSound, bool &bOk, float flPitch, bool bLookAtAddonsFolder, bool bLookAtHDFolder, char *cLanguage )
{
	// There is no system to use
	if ( !pSystem )
		return;

	// Setup some stuff first
	m_bIsPlaying = false;

	// Get the "default" path to the sound
	char cPath[64];
	sprintf( cPath, "%s/sound/%s", gEngfuncs.pfnGetGameDirectory(), cSound );

	// Check in "am_downloads" folder
	ReplaceStr( cPath, "am", "am_downloads" );
	m_pResult = gFMOD.m_pFuncSystemCreateSound( pSystem, cPath, FMOD_3D | FMOD_3D_LINEARROLLOFF, 0, &m_pSound );
	// Not found? Check in "valve_downloads" then
	if ( m_pResult != FMOD_OK )
	{
		m_pResult = gFMOD.m_pFuncSoundRelease( m_pSound );
		ReplaceStr( cPath, "am_downloads", "valve_downloads" );
		m_pResult = gFMOD.m_pFuncSystemCreateSound( pSystem, cPath, FMOD_3D | FMOD_3D_LINEARROLLOFF, 0, &m_pSound );
		// Not found? Reset for addons check
		if ( m_pResult != FMOD_OK )
		{
			m_pResult = gFMOD.m_pFuncSoundRelease( m_pSound );
			sprintf( cPath, "%s/sound/%s", gEngfuncs.pfnGetGameDirectory(), cSound );
		}
	}

	// Check in "am_addon" folder
	if ( bLookAtAddonsFolder )
	{
		ReplaceStr( cPath, "am", "am_addon" );
		m_pResult = gFMOD.m_pFuncSystemCreateSound( pSystem, cPath, FMOD_DEFAULT, 0, &m_pSound );
		// Not found? Check in "valve_addon" then
		if ( m_pResult != FMOD_OK )
		{
			m_pResult = gFMOD.m_pFuncSoundRelease( m_pSound );
			ReplaceStr( cPath, "am_addon", "valve_addon" );
			m_pResult = gFMOD.m_pFuncSystemCreateSound( pSystem, cPath, FMOD_DEFAULT, 0, &m_pSound );
			// Not found? Reset for HD check
			if ( m_pResult != FMOD_OK )
			{
				m_pResult = gFMOD.m_pFuncSoundRelease( m_pSound );
				sprintf( cPath, "%s/sound/%s", gEngfuncs.pfnGetGameDirectory(), cSound );
			}
		}
	}

	// Check in "am_hd" folder
	if ( bLookAtHDFolder && (m_pResult != FMOD_OK) )
	{
		ReplaceStr( cPath, "am", "am_hd" );
		m_pResult = gFMOD.m_pFuncSystemCreateSound( pSystem, cPath, FMOD_DEFAULT, 0, &m_pSound );
		// Not found? Check in "valve_hd" then
		if ( m_pResult != FMOD_OK )
		{
			m_pResult = gFMOD.m_pFuncSoundRelease( m_pSound );
			ReplaceStr( cPath, "am_hd", "valve_hd" );
			m_pResult = gFMOD.m_pFuncSystemCreateSound( pSystem, cPath, FMOD_DEFAULT, 0, &m_pSound );
			// Not found? Reset for language check
			if ( m_pResult != FMOD_OK )
			{
				m_pResult = gFMOD.m_pFuncSoundRelease( m_pSound );
				sprintf( cPath, "%s/sound/%s", gEngfuncs.pfnGetGameDirectory(), cSound );
			}
		}
	}

	// Check in "am_lang" folder
	if ( (strcmp( cLanguage, "english" ) != 0) && (m_pResult != FMOD_OK) )
	{
		strcat( cPath, "_" );
		strcat( cPath, cLanguage );
		m_pResult = gFMOD.m_pFuncSystemCreateSound( pSystem, cPath, FMOD_DEFAULT, 0, &m_pSound );
		// Not found? Check in "valve_lang" then
		if ( m_pResult != FMOD_OK )
		{
			m_pResult = gFMOD.m_pFuncSoundRelease( m_pSound );
			ReplaceStr( cPath, "am", "valve" );
			m_pResult = gFMOD.m_pFuncSystemCreateSound( pSystem, cPath, FMOD_DEFAULT, 0, &m_pSound );
			// Not found? Reset for root check
			if ( m_pResult != FMOD_OK )
			{
				m_pResult = gFMOD.m_pFuncSoundRelease( m_pSound );
				sprintf( cPath, "%s/sound/%s", gEngfuncs.pfnGetGameDirectory(), cSound );
			}
		}
	}

	// Check in "am" folder
	m_pResult = gFMOD.m_pFuncSystemCreateSound( pSystem, cPath, FMOD_DEFAULT, 0, &m_pSound );
	if ( m_pResult == FMOD_ERR_FILE_NOTFOUND )
	{
		// Not found? Check in "valve" then
		m_pResult = gFMOD.m_pFuncSoundRelease( m_pSound );
		ReplaceStr( cPath, "am", "valve" );
		m_pResult = gFMOD.m_pFuncSystemCreateSound( pSystem, cPath, FMOD_DEFAULT, 0, &m_pSound );
	}
	if ( m_pResult == FMOD_OK )
	{
		// Set up for play
		m_pResult = gFMOD.m_pFuncSystemPlaySound( pSystem, m_pSound, 0, true, &m_pChannel );
		if ( (m_pResult != FMOD_OK) && IsVerboseDebuggingAllowed() )
			gEngfuncs.Con_Printf( "[CFMOD2DSound::CFMOD2DSound] Failed to play sound! (%s)\n", FMOD_ErrorString( m_pResult ) );

		// Set the volume
		m_pResult = gFMOD.m_pFuncChannelSetVolume( m_pChannel, gFMOD.m_flVolume );
		if ( (m_pResult != FMOD_OK) && IsVerboseDebuggingAllowed() )
			gEngfuncs.Con_Printf( "[CFMOD2DSound::CFMOD2DSound] Failed to set volume! (%s)\n", FMOD_ErrorString( m_pResult ) );

		// Set the pitch
		m_pResult = gFMOD.m_pFuncChannelSetPitch( m_pChannel, flPitch );
		if ( (m_pResult != FMOD_OK) && IsVerboseDebuggingAllowed() )
			gEngfuncs.Con_Printf( "[CFMOD2DSound::CFMOD2DSound] Failed to set pitch! (%s)\n", FMOD_ErrorString( m_pResult ) );

		// Play it !
		m_pResult = gFMOD.m_pFuncChannelSetPaused( m_pChannel, false );
		if ( (m_pResult != FMOD_OK) && IsVerboseDebuggingAllowed() )
			gEngfuncs.Con_Printf( "[CFMOD2DSound::CFMOD2DSound] Failed to unpause sound! (%s)\n", FMOD_ErrorString( m_pResult ) );

		m_bIsPlaying = true;
	}
	else
	{
		gEngfuncs.Con_Printf( "[CFMOD2DSound::CFMOD2DSound] Failed to play sound \"%s\"!\n", cSound );
		bOk = false;
		return;
	}

	bOk = true;
}

// IsPlaying - Check if the sound is currently being playing
bool CFMOD2DSound::IsPlaying( void )
{
	return m_bIsPlaying;
}

// Update - Update the volume of the sound
void CFMOD2DSound::Update( void )
{
	m_pResult = gFMOD.m_pFuncChannelIsPlaying( m_pChannel, &m_bIsPlaying );
	if ( m_pResult == FMOD_OK )
	{
		if ( m_bIsPlaying )
		{
			m_pResult = gFMOD.m_pFuncChannelSetVolume( m_pChannel, gFMOD.m_flVolume );
			if ( (m_pResult != FMOD_OK) && IsVerboseDebuggingAllowed() )
				gEngfuncs.Con_Printf( "[CFMOD2DSound::Update] Updating sound volume failed! (%s)\n", FMOD_ErrorString( m_pResult ) );
		}
	}
	else
	{
		if ( IsVerboseDebuggingAllowed() )
			gEngfuncs.Con_Printf( "[CFMOD2DSound::Update] Retrieving sound playing status failed! (%s)\n", FMOD_ErrorString( m_pResult ) );
	}
}

// ~CFMOD2DSound - Release the 2D sound
CFMOD2DSound::~CFMOD2DSound( void )
{
	m_pResult = gFMOD.m_pFuncSoundRelease( m_pSound );
	if ( (m_pResult != FMOD_OK) && IsVerboseDebuggingAllowed() )
		gEngfuncs.Con_Printf( "[CFMOD2DSound::~CFMOD2DSound] Releasing the sound failed! (%s)\n", FMOD_ErrorString( m_pResult ) );

	m_pResult = gFMOD.m_pFuncChannelStop( m_pChannel );
	if ( (m_pResult != FMOD_OK) && IsVerboseDebuggingAllowed() )
		gEngfuncs.Con_Printf( "[CFMOD2DSound::~CFMOD2DSound] Stopping the channel failed! (%s)\n", FMOD_ErrorString( m_pResult ) );
}

//==================================================
//==================== 3D SOUND ====================
//==================================================

// CFMOD3DSound - Create a 3D sound
CFMOD3DSound::CFMOD3DSound( FMOD_SYSTEM *pSystem, const Vector vecPosition, bool &bOk, const char *cSound, float flMinDistance, float flMaxDistance, int iEntityIndexToFollow, float flPitch, bool bLookAtAddonsFolder, bool bLookAtHDFolder, char *cLanguage )
{
	// There is no system to use
	if ( !pSystem )
		return;

	// Setup some stuff first
	m_bIsPlaying = false;
	m_pEntityToFollow = NULL;

	// Get the "default" path to the sound
	char cPath[64];
	sprintf( cPath, "%s/sound/%s", gEngfuncs.pfnGetGameDirectory(), cSound );

	// Check in "am_downloads" folder
	ReplaceStr( cPath, "am", "am_downloads" );
	m_pResult = gFMOD.m_pFuncSystemCreateSound( pSystem, cPath, FMOD_3D | FMOD_3D_LINEARROLLOFF, 0, &m_pSound );
	// Not found? Check in "valve_downloads" then
	if ( m_pResult != FMOD_OK )
	{
		m_pResult = gFMOD.m_pFuncSoundRelease( m_pSound );
		ReplaceStr( cPath, "am_downloads", "valve_downloads" );
		m_pResult = gFMOD.m_pFuncSystemCreateSound( pSystem, cPath, FMOD_3D | FMOD_3D_LINEARROLLOFF, 0, &m_pSound );
		// Not found? Reset for addons check
		if ( m_pResult != FMOD_OK )
		{
			m_pResult = gFMOD.m_pFuncSoundRelease( m_pSound );
			sprintf( cPath, "%s/sound/%s", gEngfuncs.pfnGetGameDirectory(), cSound );
		}
	}

	// Check in "am_addon" folder
	if ( bLookAtAddonsFolder && (m_pResult != FMOD_OK) )
	{
		ReplaceStr( cPath, "am", "am_addon" );
		m_pResult = gFMOD.m_pFuncSystemCreateSound( pSystem, cPath, FMOD_3D | FMOD_3D_LINEARROLLOFF, 0, &m_pSound );
		// Not found? Check in "valve_addon" then
		if ( m_pResult != FMOD_OK )
		{
			m_pResult = gFMOD.m_pFuncSoundRelease( m_pSound );
			ReplaceStr( cPath, "am_addon", "valve_addon" );
			m_pResult = gFMOD.m_pFuncSystemCreateSound( pSystem, cPath, FMOD_3D | FMOD_3D_LINEARROLLOFF, 0, &m_pSound );
			// Not found? Reset for HD check
			if ( m_pResult != FMOD_OK )
			{
				m_pResult = gFMOD.m_pFuncSoundRelease( m_pSound );
				sprintf( cPath, "%s/sound/%s", gEngfuncs.pfnGetGameDirectory(), cSound );
			}
		}
	}

	// Check in "am_hd" folder
	if ( bLookAtHDFolder && (m_pResult != FMOD_OK) )
	{
		ReplaceStr( cPath, "am", "am_hd" );
		m_pResult = gFMOD.m_pFuncSystemCreateSound( pSystem, cPath, FMOD_3D | FMOD_3D_LINEARROLLOFF, 0, &m_pSound );
		// Not found? Check in "valve_hd" then
		if ( m_pResult != FMOD_OK )
		{
			m_pResult = gFMOD.m_pFuncSoundRelease( m_pSound );
			ReplaceStr( cPath, "am_hd", "valve_hd" );
			m_pResult = gFMOD.m_pFuncSystemCreateSound( pSystem, cPath, FMOD_3D | FMOD_3D_LINEARROLLOFF, 0, &m_pSound );
			// Not found? Reset for language check
			if ( m_pResult != FMOD_OK )
			{
				m_pResult = gFMOD.m_pFuncSoundRelease( m_pSound );
				sprintf( cPath, "%s/sound/%s", gEngfuncs.pfnGetGameDirectory(), cSound );
			}
		}
	}

	// Check in "am_lang" folder
	if ( (strcmp( cLanguage, "english" ) != 0) && (m_pResult != FMOD_OK) )
	{
		strcat( cPath, "_" );
		strcat( cPath, cLanguage );
		m_pResult = gFMOD.m_pFuncSystemCreateSound( pSystem, cPath, FMOD_3D | FMOD_3D_LINEARROLLOFF, 0, &m_pSound );
		// Not found? Check in "valve_lang" then
		if ( m_pResult != FMOD_OK )
		{
			m_pResult = gFMOD.m_pFuncSoundRelease( m_pSound );
			ReplaceStr( cPath, "am", "valve" );
			m_pResult = gFMOD.m_pFuncSystemCreateSound( pSystem, cPath, FMOD_3D | FMOD_3D_LINEARROLLOFF, 0, &m_pSound );
			// Not found? Reset for root check
			if ( m_pResult != FMOD_OK )
			{
				m_pResult = gFMOD.m_pFuncSoundRelease( m_pSound );
				sprintf( cPath, "%s/sound/%s", gEngfuncs.pfnGetGameDirectory(), cSound );
			}
		}
	}

	// Check in "am" folder
	m_pResult = gFMOD.m_pFuncSystemCreateSound( pSystem, cPath, FMOD_3D | FMOD_3D_LINEARROLLOFF, 0, &m_pSound );
	if ( m_pResult == FMOD_ERR_FILE_NOTFOUND )
	{
		// Not found? Check in "valve" then
		m_pResult = gFMOD.m_pFuncSoundRelease( m_pSound );
		ReplaceStr( cPath, "am", "valve" );
		m_pResult = gFMOD.m_pFuncSystemCreateSound( pSystem, cPath, FMOD_3D | FMOD_3D_LINEARROLLOFF, 0, &m_pSound );
	}
	if ( m_pResult == FMOD_OK )
	{
		// Set the distance
		m_pResult = gFMOD.m_pFuncSoundSet3DMinMaxDistance( m_pSound, flMinDistance, flMaxDistance );
		if ( (m_pResult != FMOD_OK) && IsVerboseDebuggingAllowed() )
			gEngfuncs.Con_Printf( "[CFMOD3DSound::CFMOD3DSound] Failed to set min/max distance! (%s)\n", FMOD_ErrorString( m_pResult ) );

		// Set up for play
		m_pResult = gFMOD.m_pFuncSystemPlaySound( pSystem, m_pSound, 0, true, &m_pChannel );
		if ( (m_pResult != FMOD_OK) && IsVerboseDebuggingAllowed() )
			gEngfuncs.Con_Printf( "[CFMOD3DSound::CFMOD3DSound] Failed to play sound! (%s)\n", FMOD_ErrorString( m_pResult ) );

		// Set 3D parameters (position and spread)
		FMOD_VECTOR vecPos = { vecPosition.x, vecPosition.y, vecPosition.z };
		m_pResult = gFMOD.m_pFuncChannelSet3DAttributes( m_pChannel, &vecPos, 0, 0 );
		if ( (m_pResult != FMOD_OK) && IsVerboseDebuggingAllowed() )
			gEngfuncs.Con_Printf( "[CFMOD3DSound::CFMOD3DSound] Failed to set origin! (%s)\n", FMOD_ErrorString( m_pResult ) );

		m_pResult = gFMOD.m_pFuncChannelSet3DSpread( m_pChannel, 180 );
		if ( (m_pResult != FMOD_OK) && IsVerboseDebuggingAllowed() )
			gEngfuncs.Con_Printf( "[CFMOD3DSound::CFMOD3DSound] Failed to set spread! (%s)\n", FMOD_ErrorString( m_pResult ) );

		// Set the volume
		m_pResult = gFMOD.m_pFuncChannelSetVolume( m_pChannel, gFMOD.m_flVolume );
		if ( (m_pResult != FMOD_OK) && IsVerboseDebuggingAllowed() )
			gEngfuncs.Con_Printf( "[CFMOD3DSound::CFMOD3DSound] Failed to set volume! (%s)\n", FMOD_ErrorString( m_pResult ) );

		// Set pitch
		m_pResult = gFMOD.m_pFuncChannelSetPitch( m_pChannel, flPitch );
		if ( (m_pResult != FMOD_OK) && IsVerboseDebuggingAllowed() )
			gEngfuncs.Con_Printf( "[CFMOD3DSound::CFMOD3DSound] Failed to set pitch! (%s)\n", FMOD_ErrorString( m_pResult ) );

		// If we asked the sound to follow an entity, set it up
		if ( iEntityIndexToFollow != -1 )
		{
			m_pEntityToFollow = gEngfuncs.GetEntityByIndex( iEntityIndexToFollow );
			if ( !gFMOD.DoesEntityExists( m_pEntityToFollow ) )
				m_pEntityToFollow = NULL;
		}

		// Play it !
		m_pResult = gFMOD.m_pFuncChannelSetPaused( m_pChannel, false );
		if ( (m_pResult != FMOD_OK) && IsVerboseDebuggingAllowed() )
			gEngfuncs.Con_Printf( "[CFMOD3DSound::CFMOD3DSound] Failed to set paused state! (%s)\n", FMOD_ErrorString( m_pResult ) );

		m_bIsPlaying = true;
	}
	else
	{
		gEngfuncs.Con_Printf( "[CFMOD3DSound::CFMOD3DSound] Failed to play sound \"%s\"!\n", cSound );
		bOk = false;
		return;
	}

	bOk = true;
}

// IsPlaying - Check if the sound is currently being played
bool CFMOD3DSound::IsPlaying( void )
{
	return m_bIsPlaying;
}

// Update - Update the volume of the sound and it's position if it was meant to follow an entity
void CFMOD3DSound::Update( void )
{
	// Update the volume
	m_pResult = gFMOD.m_pFuncChannelIsPlaying( m_pChannel, &m_bIsPlaying );
	if ( m_pResult == FMOD_OK )
	{
		if ( m_bIsPlaying )
		{
			m_pResult = gFMOD.m_pFuncChannelSetVolume( m_pChannel, gFMOD.m_flVolume );
			if ( (m_pResult != FMOD_OK) && IsVerboseDebuggingAllowed() )
				gEngfuncs.Con_Printf( "[CFMOD3DSound::Update] Updating sound volume failed! (%s)\n", FMOD_ErrorString( m_pResult ) );
		}
	}
	else
	{
		if ( IsVerboseDebuggingAllowed() )
			gEngfuncs.Con_Printf( "[CFMOD3DSound::Update] Retrieving sound playing status failed! (%s)\n", FMOD_ErrorString( m_pResult ) );
	}

	// The sound is meant to follow an entity, then follow it unless it is no longer valid
	if ( m_pEntityToFollow && gFMOD.DoesEntityExists( m_pEntityToFollow ) )
	{
		FMOD_VECTOR vecPos = { m_pEntityToFollow->origin.x, m_pEntityToFollow->origin.y, m_pEntityToFollow->origin.z };
		m_pResult = gFMOD.m_pFuncChannelSet3DAttributes( m_pChannel, &vecPos, 0, 0 );
		if ( (m_pResult != FMOD_OK) && IsVerboseDebuggingAllowed() )
			gEngfuncs.Con_Printf( "[CFMOD3DSound::Update] Updating sound position failed! (%s)\n", FMOD_ErrorString( m_pResult ) );
	}
	else
	{
		if ( m_pEntityToFollow )
			m_pEntityToFollow = NULL;
	}
}

// ~CFMOD3DSound - Release the 3D sound
CFMOD3DSound::~CFMOD3DSound( void )
{
	m_pEntityToFollow = NULL;
	m_pResult = gFMOD.m_pFuncSoundRelease( m_pSound );
	if ( (m_pResult != FMOD_OK) && IsVerboseDebuggingAllowed() )
		gEngfuncs.Con_Printf( "[CFMOD3DSound::~CFMOD3DSound] Releasing the sound failed! (%s)\n", FMOD_ErrorString( m_pResult ) );

	m_pResult = gFMOD.m_pFuncChannelStop( m_pChannel );
	if ( (m_pResult != FMOD_OK) && IsVerboseDebuggingAllowed() )
		gEngfuncs.Con_Printf( "[CFMOD3DSound::~CFMOD3DSound] Stopping the channel failed! (%s)\n", FMOD_ErrorString( m_pResult ) );
}
