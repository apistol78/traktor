/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <X11/Xlib.h>
#include "Core/Config.h"
#include "Ui/Enums.h"

namespace traktor::ui
{

/*! Translate X11 virtual key to our virtual key.
 * \ingroup UIW32
 *
 * \param keySym X11 virtual key symbols.
 * \param nkeySyms Number of key symbols
 * \return Virtual key.
 */
VirtualKey translateToVirtualKey(const KeySym* keySym, int nkeySyms);

}
