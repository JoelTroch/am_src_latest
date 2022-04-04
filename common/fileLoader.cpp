/*
	Written by HAWK (Alexander Bakanov)
	16.01.2015

	Custom file loader instead engine one.
*/

#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "com_model.h"

#include "fileLoader.h"
#include <windows.h>

//									for match HL
byte *AM_LoadFile(const char *name, int unused,  int *len)
{
	if (!name)	return NULL;
	char filePath[1024];
	sprintf(filePath, "am/%s", name);
	FILE *fp = fopen(filePath, "rb");
	
	if (!fp)
	{
		//gEngfuncs.Con_DPrintf("FILE: Unable to open file %s\n", filePath);
		return NULL;
	}

	int realLen = 0;
	fseek(fp, 0, SEEK_END);
	realLen = (int)ftell(fp);
	rewind(fp);
	if (realLen <= 0)
	{
		if (len) *len = 0;
		gEngfuncs.Con_DPrintf("FILE: No length %s\n", filePath);
		fclose(fp);
		return NULL;
	}

	byte *buff = (byte*)malloc(realLen);
	if (!buff)
	{
		#ifdef _DEBUG
		char text[256];
		sprintf(text,"FILE: No memory %s\n",filePath);
		MessageBox(NULL,text,NULL,NULL);
		exit(-1);
		#else
		gEngfuncs.Con_DPrintf("FILE: No memory %s\n", filePath);
		#endif

		if (len) *len = 0;
		fclose(fp);
		return NULL;
	}

	int read = (int)fread(buff, 1, realLen, fp);
	if (read != realLen)
	{
		gEngfuncs.Con_DPrintf("FILE: Unable to read file %s\n", filePath);
		if (len) *len = 0;

		if (buff)
			AM_FreeFile(buff);
		fclose(fp);
		return NULL;
	}

	*len = realLen;
	fclose(fp);
	return buff;
}

void AM_FreeFile(byte *buff)
{
	//gEngfuncs.Con_Printf("FILE: Free memory\n");
//	if (buff)
//		delete[]buff;
	if (!buff) return;

	free(buff);
}
void AM_FreeFile(char *buff)
{
	AM_FreeFile((byte*)buff);
}
