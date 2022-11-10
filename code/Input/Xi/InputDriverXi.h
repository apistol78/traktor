/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <windows.h>
#include <xinput.h>
#include "Core/RefArray.h"
#include "Input/IInputDriver.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_XI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace input
	{

class InputDeviceXi;

/*! XInput driver.
 * \ingroup Input
 */
class T_DLLCLASS InputDriverXi : public IInputDriver
{
	T_RTTI_CLASS;

public:
	InputDriverXi(DWORD deviceCount = 4);

	virtual bool create(const SystemApplication& sysapp, const SystemWindow& syswin, uint32_t inputCategories) override final;

	virtual int getDeviceCount() override final;

	virtual Ref< IInputDevice > getDevice(int index) override final;

	virtual UpdateResult update() override final;

private:
	RefArray< InputDeviceXi > m_devices;
};

	}
}

