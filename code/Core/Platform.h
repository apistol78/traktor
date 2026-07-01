/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
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
		class DelegateInstance* instance = nullptr;

		SystemApplication() = default;

		explicit SystemApplication(class DelegateInstance* instance_)
		:	instance(instance_)
		{
		}

#else
		void* unk = nullptr;
#endif
	};

	/*!
	 * \ingroup Core
	 */
	struct SystemWindow
	{
#if defined(_WIN32)
		void* hWnd = nullptr;

		SystemWindow() = default;

		explicit SystemWindow(void* hWnd_)
		:	hWnd(hWnd_)
		{
		}

#elif defined(__LINUX__) || defined(__RPI__)
		void* x11_display = nullptr;
		unsigned long x11_window = 0;
		void* wl_display = nullptr;
		unsigned long wl_surface = 0;
		int32_t width = -1;
		int32_t height = -1;

		static SystemWindow fromX11(void* display_, unsigned long window_)
		{
			return { display_, window_, nullptr, 0, -1, -1 };
		}

		static SystemWindow fromWayland(void* display_, unsigned long surface_, int32_t width, int32_t height)
		{
			return { nullptr, 0, display_, surface_, width, height };
		}

#elif defined(__APPLE__)
		void* view = nullptr;

		SystemWindow() = default;

		explicit SystemWindow(void* view_)
		:	view(view_)
		{
		}

#elif defined(__ANDROID__)
		struct ::ANativeWindow** window = nullptr;

		SystemWindow() = default;

		explicit SystemWindow(struct ::ANativeWindow** window_)
		:	window(window_)
		{
		}

#else
		void* unk = nullptr;
#endif
	};

}
