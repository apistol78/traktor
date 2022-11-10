/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#if defined(_WIN32)
#	include "Render/Vulkan/Win32/ApiLoader.h"
#elif defined(__LINUX__) || defined(__RPI__)
#	include "Render/Vulkan/Linux/ApiLoader.h"
#elif defined(__ANDROID__)
#	include "Render/Vulkan/Android/ApiLoader.h"
#elif defined(__MAC__)
#	include "Render/Vulkan/macOS/ApiLoader.h"
#elif defined(__IOS__)
#	include "Render/Vulkan/iOS/ApiLoader.h"
#endif

