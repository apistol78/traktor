/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Input/Linux/GamepadDeviceLinux.h"
#include "Input/Linux/InputDriverLinux.h"

namespace traktor::input
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.InputDriverLinux", 0, InputDriverLinux, IInputDriver)

bool InputDriverLinux::create(const SystemApplication& sysapp, const SystemWindow& syswin, InputCategory inputCategories)
{
	for (int32_t i = 0; i < 4; ++i)
	{
		int fd = open(wstombs(str(L"/dev/input/js%d", i)).c_str(), O_RDONLY | O_NONBLOCK);
		if (fd < 0)
			break;
		m_devices.push_back(new GamepadDeviceLinux(fd));
		log::info << L"Gamepad " << i << L" input device created." << Endl;
	}
	return true;
}

int InputDriverLinux::getDeviceCount()
{
	return (int)m_devices.size();
}

Ref< IInputDevice > InputDriverLinux::getDevice(int index)
{
	return m_devices[index];
}

InputDriverLinux::UpdateResult InputDriverLinux::update()
{
	return UrOk;
}

}
