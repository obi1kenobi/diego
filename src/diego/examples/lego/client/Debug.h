#ifndef __SF_DEBUG_H__
#define __SF_DEBUG_H__

#include <cstdarg>

//
// Debug printf. It gets printed to standard error to force a flush.
// It only gets printed if debug level is set at or higher than 
// the provided level.
//
int SfDPrintf(int level, const char *fmt, ...);

//
// Everything at or below this level will get printed.
//
void SfSetDebugLevel(int level);

//
// Returns current debug level
//
int SfGetDebugLevel();

#endif // __SF_DEBUG_H__
