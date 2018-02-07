#ifndef _MMERRORS_H
#define _MMERRORS_H

#include <windows.h>
#include <mmsystem.h>

extern void reportmmresult(const char *str, MMRESULT res);

#endif