/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_Platform_H
#define traktor_Platform_H

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

namespace traktor
{

	/*! \brief
	 * \ingroup Core
	 */
	struct SystemApplication
	{
#if defined(__ANDROID__)
		class DelegateInstance* instance;

		SystemApplication()
		:	instance(0)
		{
		}

		explicit SystemApplication(class DelegateInstance* instance_)
		:	instance(instance_)
		{
		}

#elif defined(__PNACL__)
		pp::Instance* instance;

		SystemApplication()
		:	instance(0)
		{
		}

		explicit SystemApplication(pp::Instance* instance_)
		:	instance(instance_)
		{
		}

#else
		void* unk;

		SystemApplication()
		:	unk(0)
		{
		}

#endif
	};

	/*! \brief
	 * \ingroup Core
	 */
	struct SystemWindow
	{
#if defined(_WIN32) && !defined(_XBOX_ONE)
		HWND hWnd;

		SystemWindow()
		:	hWnd(0)
		{
		}

		explicit SystemWindow(HWND hWnd_)
		:	hWnd(hWnd_)
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

		explicit SystemWindow(void* display_, unsigned long window_)
		:	display(display_)
		,	window(window_)
		{
		}

#elif defined(__APPLE__)
		void* view;
		
		SystemWindow()
		:	view(0)
		{
		}

		explicit SystemWindow(void* view_)
		:	view(view_)
		{
		}

#elif defined(__ANDROID__)
		struct ANativeWindow** window;

		SystemWindow()
		:	window(0)
		{
		}

		explicit SystemWindow(struct ANativeWindow** window_)
		:	window(window_)
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
