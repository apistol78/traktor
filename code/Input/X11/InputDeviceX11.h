/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <X11/Xlib.h>
#include "Input/IInputDevice.h"

namespace traktor
{
	namespace input
	{

class InputDeviceX11 : public IInputDevice
{
	T_RTTI_CLASS;

public:
	virtual void consumeEvent(XEvent& evt) = 0;

	virtual void setFocus(bool focus) = 0;
};

	}
}

