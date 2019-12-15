#pragma once

#if defined (_WIN32)
#	if defined (_XBOX)
#		include <xtl.h>
#	else
#		define NOMINMAX
#		include <windows.h>
#	endif
#	include <tchar.h>
#elif defined(__ANDROID__)
#	include <android/native_window.h>
#endif
