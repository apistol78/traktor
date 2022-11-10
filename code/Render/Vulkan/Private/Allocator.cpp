/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
// Vulkan Memory Allocator implementation file.

#include "Render/Vulkan/Private/ApiHeader.h"
#include "Render/Vulkan/Private/ApiLoader.h"

#undef min
#undef max

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
