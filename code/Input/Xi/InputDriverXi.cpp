/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Input/Xi/InputDriverXi.h"
#include "Input/Xi/InputDeviceXi.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.InputDriverXi", 0, InputDriverXi, IInputDriver)

InputDriverXi::InputDriverXi(DWORD deviceCount)
{
	for (DWORD i = 0; i < deviceCount; ++i)
		m_devices.push_back(new InputDeviceXi(i));
}

bool InputDriverXi::create(const SystemApplication& sysapp, const SystemWindow& syswin, uint32_t inputCategories)
{
	return true;
}

int InputDriverXi::getDeviceCount()
{
	return int(m_devices.size());
}

Ref< IInputDevice > InputDriverXi::getDevice(int index)
{
	return m_devices[index];
}

IInputDriver::UpdateResult InputDriverXi::update()
{
	return UrOk;
}

	}
}
