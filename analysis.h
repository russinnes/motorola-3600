#ifndef _ANALYSIS_H
#define _ANALYSIS_H

class CMotorolaSystem ;

#include <windows.h>
#include <mmsystem.h>
void CALLBACK analysisProc(WAVEHDR *lpHdr, CMotorolaSystem *pSystem, FILE *pFile);

#endif
