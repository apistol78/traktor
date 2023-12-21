/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Input/IInputDriver.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_WIN32_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

struct SystemWindow;

}

namespace traktor::input
{

class KeyboardDeviceWin32;
class MouseDeviceWin32;

class T_DLLCLASS InputDriverWin32 : public IInputDriver
{
	T_RTTI_CLASS;

public:
	virtual bool create(const SystemApplication& sysapp, const SystemWindow& syswin, InputCategory inputCategories) override final;

	virtual int getDeviceCount() override final;

	virtual Ref< IInputDevice > getDevice(int index) override final;

	virtual UpdateResult update() override final;

private:
	Ref< KeyboardDeviceWin32 > m_keyboardDevice;
	Ref< MouseDeviceWin32 > m_mouseDevice;
	RefArray< IInputDevice > m_devices;
};

}
