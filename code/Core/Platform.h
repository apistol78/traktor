#ifndef traktor_Platform_H
#define traktor_Platform_H

#if defined (_WIN32)
#if defined (_XBOX)
#include <xtl.h>
#else
#define _WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <tchar.h>
#endif

#endif	// traktor_Platform_H
