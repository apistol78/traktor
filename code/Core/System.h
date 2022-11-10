/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#if defined (_WIN32)
#	define NOMINMAX
#	include <windows.h>
#	include <tchar.h>
#elif defined(__ANDROID__)
#	include <android/native_window.h>
#endif
