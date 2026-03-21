/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

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

}
