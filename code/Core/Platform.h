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
		void* display = nullptr;
		unsigned long window = 0;

		SystemWindow() = default;

		explicit SystemWindow(void* display_, unsigned long window_)
		:	display(display_)
		,	window(window_)
		{
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
