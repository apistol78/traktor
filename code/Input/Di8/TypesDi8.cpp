/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <Windows.h>
#include "Input/Win32/TypesWin32.h"

namespace traktor::input
{

uint32_t translateFromVk(uint32_t vk)
{
	for (uint32_t i = 0; i < sizeof_array(c_vkControlKeys); ++i)
	{
		if (c_vkControlKeys[i] == vk)
			return i;
	}
	return 0;
}

}
