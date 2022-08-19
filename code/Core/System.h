#pragma once

#if defined (_WIN32)
#	define NOMINMAX
#	include <windows.h>
#	include <tchar.h>
#elif defined(__ANDROID__)
#	include <android/native_window.h>
#endif
