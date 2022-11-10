/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <IOKit/hid/IOHIDLib.h>
#include "Core/RefArray.h"
#include "Input/IInputDriver.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_OSX_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace input
	{

class IInputDevice;
class InputDeviceKeyboardOsX;
class InputDeviceMouseOsX;

class T_DLLCLASS InputDriverOsX : public IInputDriver
{
	T_RTTI_CLASS;

public:
	InputDriverOsX();

	virtual bool create(const SystemApplication& sysapp, const SystemWindow& syswin, uint32_t inputCategories) override final;

	virtual int getDeviceCount() override final;

	virtual Ref< IInputDevice > getDevice(int index) override final;

	virtual UpdateResult update() override final;

private:
	bool m_devicesChanged;
	RefArray< IInputDevice > m_devices;
	Ref< InputDeviceKeyboardOsX > m_keyboardDevice;
	Ref< InputDeviceMouseOsX > m_mouseDevice;

	static void callbackDeviceMatch(void* inContext, IOReturn inResult, void* inSender, IOHIDDeviceRef inIOHIDDeviceRef);
};

	}
}

