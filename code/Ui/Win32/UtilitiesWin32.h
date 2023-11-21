/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Config.h"
#include "Ui/Enums.h"
#include "Ui/Font.h"
#include "Ui/Win32/Window.h"

namespace traktor::ui
{

/*! Translate Win32 virtual key to our virtual key.
 * \ingroup UIW32
 *
 * \param keyCode Win32 virtual key.
 * \return Virtual key.
 */
VirtualKey translateToVirtualKey(uint32_t keyCode);

/*! Translate virtual key to Win32 virtual key.
 * \ingroup UIW32
 *
 * \param vk Virtual key.
 * \return Key code.
 */
uint32_t translateToKeyCode(VirtualKey vk);

/*!
 */
Font logFontToFont(const LOGFONT& lf);

/*!
 */
LOGFONT fontToLogFont(const Font& fnt);

}
