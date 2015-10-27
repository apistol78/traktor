#ifndef traktor_Platform_H
#define traktor_Platform_H

#if defined (_WIN32)
#	if defined (_XBOX)
#		include <xtl.h>
#	else
#		define _WIN32_LEAN_AND_MEAN
#		include <windows.h>
#	endif
#	include <tchar.h>
#endif

#if defined(max)
#	undef max
#endif
#if defined(min)
#	undef min
#endif

namespace traktor
{

	struct SystemWindow
	{
#if defined(_WIN32)
		HWND hWnd;

		SystemWindow()
		:	hWnd(0)
		{
		}

#elif defined(__LINUX__)
		void* display;
		unsigned long window;

		SystemWindow()
		:	display(0)
		,	window(0)
		{
		}

#elif defined(__APPLE__)

		void* view;
		
		SystemWindow()
		:	view(0)
		{
		}

#elif defined(__ANDROID__)

		struct ANativeWindow* window;

		SystemWindow()
		:	window(0)
		{
		}

#else
		void* unk;

		SystemWindow()
		:	unk(0)
		{
		}
#endif
	};

}

#endif	// traktor_Platform_H
