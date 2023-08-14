/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Input/InputTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::input
{

class IInputDriver;
class IInputDevice;

/*! Input system.
 * \ingroup Input
 */
class T_DLLCLASS InputSystem : public Object
{
	T_RTTI_CLASS;

public:
	void addDriver(IInputDriver* inputDriver);

	void removeDriver(IInputDriver* inputDriver);

	void addDevice(IInputDevice* inputDevice);

	void removeDevice(IInputDevice* inputDevice);

	int32_t getDeviceCount() const;

	IInputDevice* getDevice(int32_t index);

	int32_t getDeviceCount(InputCategory category, bool connected) const;

	IInputDevice* getDevice(InputCategory category, int32_t index, bool connected);

	void setExclusive(bool exclusive);

	bool update();

private:
	RefArray< IInputDriver > m_drivers;
	RefArray< IInputDevice > m_devices;

	void updateDevices();
};

}
