#include "stdafx.h"
#include "analysis.h"
#include "motorola.h"

void CALLBACK analysisProc(WAVEHDR *lpHdr, CMotorolaSystem *pSystem, FILE *pFile)
{
  unsigned nSize = lpHdr->dwBytesRecorded / sizeof (short);
  short *lpData = (short *)lpHdr->lpData;
  unsigned nDex = 0;

  nDex = 0;
  long nCount = 0;
  while (nDex < nSize)
  {
    bool bBit = lpData[nDex++] < 0;
	nCount++;
	while ( nDex < nSize && bBit == (bool)(lpData[nDex] < 0) )
	{
	  nDex++;
	  nCount++;
	}
    pSystem->inputData(bBit, nCount);
    nCount = 0;
  }
}
