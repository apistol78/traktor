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
#include <X11/Xutil.h>
#include <X11/extensions/XInput2.h>
#include "Core/Platform.h"
#include "Core/RefArray.h"
#include "Input/IInputDriver.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_X11_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::input
{

class InputDeviceX11;

class T_DLLCLASS InputDriverX11 : public IInputDriver
{
	T_RTTI_CLASS;

public:
	InputDriverX11();

	virtual bool create(const SystemApplication& sysapp, const SystemWindow& syswin, uint32_t inputCategories) override;

	virtual int getDeviceCount() override;

	virtual Ref< IInputDevice > getDevice(int index) override;

	virtual UpdateResult update() override;

private:
	Display* m_display;
	Window m_window;
	int32_t m_opcode;
	RefArray< InputDeviceX11 > m_devices;
};

}
