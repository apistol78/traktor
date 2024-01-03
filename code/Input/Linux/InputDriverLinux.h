/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
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
#if defined(T_INPUT_LINUX_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::input
{

class GamepadDeviceLinux;

class T_DLLCLASS InputDriverLinux : public IInputDriver
{
	T_RTTI_CLASS;

public:
	virtual bool create(const SystemApplication& sysapp, const SystemWindow& syswin, InputCategory inputCategories) override;

	virtual int getDeviceCount() override;

	virtual Ref< IInputDevice > getDevice(int index) override;

	virtual UpdateResult update() override;

private:
	RefArray< GamepadDeviceLinux > m_devices;
};

}
