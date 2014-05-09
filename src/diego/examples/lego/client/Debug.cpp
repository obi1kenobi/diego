#include "Debug.h"

#include <cstdio>

// Only debug level messages 0 get printed by default
static int _level = 0;

int
SfDPrintf(int level, const char *fmt, ...)
{
    if (level >= _level) {
        va_list args;
        va_start(args, fmt);
        int result = vfprintf(stderr, fmt, args);
        va_end(args);
        return result;
    } else {
        return 0;
    }
}

void
SfSetDebugLevel(int level)
{
    _level = level;
}

int
SfGetDebugLevel()
{
    return _level;
}
