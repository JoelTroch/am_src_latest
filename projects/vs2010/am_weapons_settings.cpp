/*******************************************************
*	CBaseSpreadWeapon class implementation.
*
*	contains functions to calculate spread.
*	written by BUzer for Half-Life:Paranoia modification
*	Modified by SysOp
*******************************************************/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "soundent.h"
#include "gamerules.h"

wep_params_t	gWeaponSettings[32];
wep_params_t	gDefaultSettings;

int				gSCounter;

typedef struct name_params_s
{
	char szName[32];
} name_params_t;

name_params_s	gNames[2048];
int	nameCounter;

char* GetRandomNPCName(void)
{
	return gNames[RANDOM_LONG(0, nameCounter)].szName;
}

void LoadNames(void)
{
	nameCounter = 0;

	extern char com_token[1500];
	char *COM_Parse(char *data);

	int length;
	char *pFile;
	char *aFile = pFile = (char*)LOAD_FILE_FOR_ME("scripts/npc_names.db", &length);

	if (!pFile || !length)
	{
		ALERT(at_console, " *\n");
		ALERT(at_console, " * failed to load file %s\n", "scripts/npc_names.db");
		ALERT(at_console, " *\n");
		return;
	}

	// parse weapons
	while (1)
	{
		// get weapon name
		pFile = COM_Parse(pFile);
		if (strlen(com_token) <= 0)
			break; // end of file

		strcpy(gNames[nameCounter].szName, com_token);//first token is wep name

		nameCounter++;
	}

	ALERT(at_console, "random item from array: %s\n", gNames[RANDOM_LONG(0, nameCounter)].szName);

	FREE_FILE(aFile);
}

void DebugWeaponSettings(void)
{
	ALERT(at_console, "|== Listing spread table ==\n");
	ALERT(at_console, "| %d weapons total\n", gSCounter);
	ALERT(at_console, "|\n");

	for (int i = 0; i < gSCounter; i++)
	{
		ALERT(at_console, "|    weapon: %s\n", gWeaponSettings[i].szWeaponName);
		ALERT(at_console, "|    origin_x: %f\n", gWeaponSettings[i].origin_x);
		ALERT(at_console, "|    origin_y: %f\n", gWeaponSettings[i].origin_y);
		ALERT(at_console, "|    origin_z: %f\n", gWeaponSettings[i].origin_z);

		ALERT(at_console, "|\n");
	}

	ALERT(at_console, "|== spread table end ==\n");
}

extern char com_token[1500];//replace multiplay_gamerules.cpp:static char com_token[ 1500 ];
char *COM_Parse(char *data);

void LoadWeaponSettings(char *filename)
{
	gSCounter = 0;

	int length;
	char *pFile;
	char *aFile = pFile = (char*)LOAD_FILE_FOR_ME(filename, &length);

	if (!pFile || !length)
	{
		ALERT(at_console, " *\n");
		ALERT(at_console, " * LoadSpreadTable failed to load file %s\n", filename);
		ALERT(at_console, " *\n");
		return;
	}

	// parse weapons
	while (1)
	{
		// get weapon name
		pFile = COM_Parse(pFile);
		if (strlen(com_token) <= 0)
			break; // end of file

		strcpy(gWeaponSettings[gSCounter].szWeaponName, com_token);//first token is wep name

		// '{' expected
		pFile = COM_Parse(pFile);
		if (com_token[0] != '{')
		{
			ALERT(at_console, " *\n");
			ALERT(at_console, " * LoadSpreadTable parsing error: expecting { after weapon name\n");
			ALERT(at_console, " *\n");
			break;
		}

		// parse sections
		while (1)
		{
			// 'primary', 'secondary', or 'returntime'
			pFile = COM_Parse(pFile);
			if (com_token[0] == '}')
				break; // no more sections

			// primary settings section
			if (!strcmp("ironsight", com_token))
			{
				// '{' expected
				pFile = COM_Parse(pFile);
				if (com_token[0] != '{')
				{
					ALERT(at_console, " *\n");
					ALERT(at_console, " * LoadSpreadTable parsing error: expecting { after section name\n");
					ALERT(at_console, " *\n");
					break;
				}
				// parse parameters
				while (1)
				{
					pFile = COM_Parse(pFile);
					if (com_token[0] == '}')
						break; // section finished

					if (!strcmp("x", com_token))
					{
						pFile = COM_Parse(pFile); // read value
						gWeaponSettings[gSCounter].origin_x = atof(com_token);
						gWeaponSettings[gSCounter].origin_x = gWeaponSettings[gSCounter].origin_x * -1 -0.045;//this small number corrects discrepances between engine and hlmv
					}
					else if (!strcmp("y", com_token))
					{
						pFile = COM_Parse(pFile); // read value
						gWeaponSettings[gSCounter].origin_y = atof(com_token);
					}
					else if (!strcmp("z", com_token))
					{
						pFile = COM_Parse(pFile); // read value
						gWeaponSettings[gSCounter].origin_z = atof(com_token);
						gWeaponSettings[gSCounter].origin_z = (gWeaponSettings[gSCounter].origin_z * -1) -1;
					}
					else
					{
						ALERT(at_console, " *\n");
						ALERT(at_console, " * LoadSpreadTable parsing error: %s - unknown variable name\n", com_token);
						ALERT(at_console, " *\n");
					}
				}
			}
			// secondary settings section
			else if (!strcmp("vec_barrel", com_token))
			{
				// '{' expected
				pFile = COM_Parse(pFile);
				if (com_token[0] != '{')
				{
					ALERT(at_console, " *\n");
					ALERT(at_console, " * LoadSpreadTable parsing error: expecting { after section name\n");
					ALERT(at_console, " *\n");
					break;
				}
				// parse spread parameters
				while (1)
				{
					pFile = COM_Parse(pFile);
					if (com_token[0] == '}')
						break; // section finished

					if (!strcmp("x", com_token))
					{
						pFile = COM_Parse(pFile);
						CVAR_SET_FLOAT("barrel_x", atof(com_token));
					//	ALERT(at_console, " * barrel_x position loaded - %f\n", atof(com_token));
					}
					else if (!strcmp("y", com_token))
					{
						pFile = COM_Parse(pFile);
						CVAR_SET_FLOAT("barrel_y", atof(com_token));
					//	ALERT(at_console, " * barrel_y position loaded - %f\n", atof(com_token));
					}
					/*	else if (!strcmp("attachment", com_token))//not needed
					{
					pFile = COM_Parse( pFile );
					CVAR_SET_FLOAT( "barrel_attach", atoi(com_token) );//int
					ALERT(at_console, " * using attachment %i as reference\n", atoi(com_token));
					}*/
					else
					{
						ALERT(at_console, " *\n");
						ALERT(at_console, " * LoadSpreadTable parsing error: %s - unknown variable name\n", com_token);
						ALERT(at_console, " *\n");
					}
				}
			}
			else
			{
				ALERT(at_console, " *\n");
				ALERT(at_console, " * LoadSpreadTable parsing error: %s - unknown section name\n", com_token);
				ALERT(at_console, " *\n");
			}
		}

		gSCounter++;
	}

	ALERT(at_console, "%d spread weapons info loaded from %s\n", gSCounter, filename);

	FREE_FILE(aFile);

	// initialize default spread
	sprintf(gDefaultSettings.szWeaponName, "default\n");
	gDefaultSettings.origin_x = 0;
	gDefaultSettings.origin_y = 0;
	gDefaultSettings.origin_z = 0;
}