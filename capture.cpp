// capture.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <mmsystem.h>
#include <conio.h>
#include "analysis.h"
#include "motorola.h"
#include "mmerrors.h"

#pragma comment(lib, "winmm.lib")

struct SBinding
{
  CMotorolaSystem *m_pSystem;
  FILE *m_pFile;
};

static void CALLBACK waveInProc(HWAVEIN hwi, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
  struct SBinding *pBind = NULL;
  WAVEHDR *lpHdr = NULL;
  switch (uMsg)
  {
    case WIM_OPEN:
      printf("open");
	  break;
    case WIM_CLOSE:
	  printf("close");
	  break;
    case WIM_DATA:
      pBind = (struct SBinding *)dwInstance;
	  lpHdr = (WAVEHDR *)dwParam1;
	  analysisProc(lpHdr, pBind->m_pSystem, pBind->m_pFile);
	  waveInUnprepareHeader(hwi, lpHdr, sizeof *lpHdr);
	  waveInPrepareHeader(hwi, lpHdr, sizeof *lpHdr);
	  waveInAddBuffer(hwi, lpHdr, sizeof *lpHdr);
      break;
    default:
	  break;
  }
}

int main(int argc, char* argv[])
{
	HWAVEIN hWaveIn = NULL;
	WAVEFORMATEX waveFormatEx = {0};
	MMRESULT res = 0;
	DWORD dwOpen = CALLBACK_FUNCTION;

	int keyPress = 'i';
	bool bInvert = argc > 1;

	printf("Hello World!\nStarting with %s.\n", bInvert ? "invert" : "normal");

    waveFormatEx.wFormatTag = WAVE_FORMAT_PCM;
    waveFormatEx.nChannels = 1;
    waveFormatEx.nSamplesPerSec = 22050;
    waveFormatEx.nAvgBytesPerSec = 44100;
    waveFormatEx.nBlockAlign = 2;
    waveFormatEx.wBitsPerSample = 16;
    waveFormatEx.cbSize = 0; 

	CMotorolaSystem system(3600L, false, waveFormatEx.nSamplesPerSec);
	system.setInvert(bInvert);

	struct SBinding binding;
	binding.m_pSystem = &system;
	binding.m_pFile = fopen("dump.raw", "w");

	res = waveInOpen(&hWaveIn, WAVE_MAPPER, &waveFormatEx, (DWORD)waveInProc, (DWORD)&binding, dwOpen);
	reportmmresult("waveInOpen", res);

	WAVEHDR waveHeaderA = {0};
	WAVEHDR waveHeaderB = {0};

	waveHeaderA.dwBufferLength = 1<<13;
	waveHeaderA.lpData = (char*)LocalAlloc(0, waveHeaderA.dwBufferLength);
	waveHeaderB.dwBufferLength = 1<<13;
	waveHeaderB.lpData = (char*)LocalAlloc(0, waveHeaderB.dwBufferLength);

	res = waveInPrepareHeader(hWaveIn, &waveHeaderA, sizeof waveHeaderA);
	reportmmresult("waveInPrepareHeader", res);

	res = waveInAddBuffer(hWaveIn, &waveHeaderA, sizeof waveHeaderA);
	reportmmresult("waveInAddBuffer", res);

	res = waveInPrepareHeader(hWaveIn, &waveHeaderB, sizeof waveHeaderB);
	reportmmresult("waveInPrepareHeader", res);

	res = waveInAddBuffer(hWaveIn, &waveHeaderB, sizeof waveHeaderB);
	reportmmresult("waveInAddBuffer", res);

	res = waveInStart(hWaveIn);
	reportmmresult("waveInStart", res);

	while (keyPress == 'i')
	{
      while (!kbhit())
		SleepEx(100L, TRUE);
	  keyPress = ::_getche();
	  if (keyPress == 'i') bInvert = !bInvert;
	  system.setInvert(bInvert);
	}

	res = waveInStop(hWaveIn);
	reportmmresult("waveInStop", res);

	res = waveInClose(hWaveIn);
	reportmmresult("waveInClose", res);

	if (binding.m_pFile) fclose(binding.m_pFile);

	return 0;
}
