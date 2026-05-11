/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <linux/input-event-codes.h>
#include <xkbcommon/xkbcommon.h>
#include "Core/Config.h"
#include "Ui/Enums.h"

namespace traktor::ui
{

/*! Translate XKB keysym to our virtual key.
 * \ingroup UI
 *
 * \param keySym XKB keysym.
 * \return Virtual key.
 */
VirtualKey translateToVirtualKey(xkb_keysym_t keySym);

/*! Translate a Linux input button code (BTN_LEFT/MIDDLE/RIGHT) to an MbtX mask.
 *  Returns 0 for unsupported buttons.
 */
inline int32_t translateMouseButton(uint32_t button)
{
	switch (button)
	{
	case BTN_LEFT:   return MbtLeft;
	case BTN_MIDDLE: return MbtMiddle;
	case BTN_RIGHT:  return MbtRight;
	default:         return 0;
	}
}

}
