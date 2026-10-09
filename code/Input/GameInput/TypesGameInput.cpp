/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Input/GameInput/TypesGameInput.h"

#include <windows.h>

namespace traktor::input
{

uint32_t translateFromScanCode(uint32_t scanCode, bool extended, uint32_t vk)
{
	// Pause and NumLock share scan code 0x45, only the virtual key tells them apart.
	uint32_t code = scanCode & 0x7f;
	if (vk == VK_PAUSE)
		code = 0xc5;
	else if (vk == VK_NUMLOCK)
		code = 0x45;
	else if (extended)
		code |= 0x80;

	for (uint32_t i = 0; i < sizeof_array(c_scanCodeControlKeys); ++i)
		if (c_scanCodeControlKeys[i] == code)
			return i;
	return 0;
}

}
