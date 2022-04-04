#ifndef AM_FILE_LOADER_H
#define AM_FILE_LOADER_H

byte *AM_LoadFile(const char *name, int unused, int *len);
void AM_FreeFile(byte *buff);
void AM_FreeFile(char *buff);

#endif