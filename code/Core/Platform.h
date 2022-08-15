#pragma once

#if defined(__ANDROID__)
struct ANativeWindow;
#endif

namespace traktor
{

	/*!
	 * \ingroup Core
	 */
	struct SystemApplication
	{
#if defined(__ANDROID__)
		class DelegateInstance* instance;

		SystemApplication()
		:	instance(nullptr)
		{
		}

		explicit SystemApplication(class DelegateInstance* instance_)
		:	instance(instance_)
		{
		}
#else
		void* unk;

		SystemApplication()
		:	unk(nullptr)
		{
		}
#endif
	};

	/*!
	 * \ingroup Core
	 */
	struct SystemWindow
	{
#if defined(_WIN32)
		void* hWnd;

		SystemWindow()
		:	hWnd(nullptr)
		{
		}

		explicit SystemWindow(void* hWnd_)
		:	hWnd(hWnd_)
		{
		}

#elif defined(__LINUX__) || defined(__RPI__)
		void* display;
		unsigned long window;

		SystemWindow()
		:	display(nullptr)
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
		:	view(nullptr)
		{
		}

		explicit SystemWindow(void* view_)
		:	view(view_)
		{
		}

#elif defined(__ANDROID__)
		struct ::ANativeWindow** window;

		SystemWindow()
		:	window(nullptr)
		{
		}

		explicit SystemWindow(struct ::ANativeWindow** window_)
		:	window(window_)
		{
		}

#else
		void* unk;

		SystemWindow()
		:	unk(nullptr)
		{
		}
#endif
	};

}
