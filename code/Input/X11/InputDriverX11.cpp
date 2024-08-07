/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Timer/Profiler.h"
#include "Input/X11/InputDriverX11.h"
#include "Input/X11/KeyboardDeviceX11.h"
#include "Input/X11/MouseDeviceX11.h"

namespace traktor::input
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.InputDriverX11", 0, InputDriverX11, IInputDriver)

InputDriverX11::InputDriverX11()
:	m_display(0)
,	m_window(None)
,	m_opcode(0)
{
}

bool InputDriverX11::create(const SystemApplication& sysapp, const SystemWindow& syswin, InputCategory inputCategories)
{
	m_display = (Display*)syswin.display;
	m_window = (Window)syswin.window;

	static int32_t opcode = 0, event = 0, error = 0;
	if (!XQueryExtension(m_display, "XInputExtension", &opcode, &event, &error))
	{
		log::error << L"X11 input failed; \"XInputExtension\" not supported" << Endl;
		return false;
	}

	int32_t major = 2, minor = 0;
	if (XIQueryVersion(m_display, &major, &minor) == BadRequest)
	{
		log::error << L"X11 input failed; Need atleast XInput 2.0" << Endl;
		return false;
	}

	m_opcode = opcode;

	log::info << L"Using XInput " << major << L"." << minor << L"; setting up devices..." << Endl;

	XIDeviceInfo* deviceInfo = 0;
	int deviceInfoCount = 0;

	deviceInfo = XIQueryDevice(m_display, XIAllMasterDevices, &deviceInfoCount);

	log::info << L"Found " << deviceInfoCount << L" X11 device(s)" << Endl;
	for (int i = 0; i < deviceInfoCount; ++i)
	{
		if (deviceInfo[i].use == XIMasterPointer || deviceInfo[i].use == XISlavePointer)
		{
			log::info << L"Mouse device " << i << L" \"" << mbstows(deviceInfo[i].name) << L"\"" << Endl;
			Ref< MouseDeviceX11 > mouseDevice = new MouseDeviceX11(m_display, m_window, deviceInfo[i].deviceid);
			m_devices.push_back(mouseDevice);
		}
		else if (deviceInfo[i].use == XIMasterKeyboard || deviceInfo[i].use == XISlaveKeyboard)
		{
			log::info << L"Keyboard device " << i << L" \"" << mbstows(deviceInfo[i].name) << L"\"" << Endl;
			Ref< KeyboardDeviceX11 > keyboardDevice = new KeyboardDeviceX11(m_display, m_window, deviceInfo[i].deviceid);
			m_devices.push_back(keyboardDevice);
		}
	}

	XIFreeDeviceInfo(deviceInfo);
	return true;
}

int InputDriverX11::getDeviceCount()
{
	return int(m_devices.size());
}

Ref< IInputDevice > InputDriverX11::getDevice(int index)
{
	return m_devices[index];
}

InputDriverX11::UpdateResult InputDriverX11::update()
{
	T_PROFILER_SCOPE(L"InputDriverX11 update");

	XEvent evt;
	while (XCheckTypedEvent(m_display, GenericEvent, &evt))
	{
		if (evt.xcookie.type == GenericEvent && evt.xcookie.extension == m_opcode)
		{
			if (XGetEventData(m_display, &evt.xcookie))
			{
				for (auto device : m_devices)
				{
					T_PROFILER_SCOPE(str(L"InputDriverX11 update - %s", type_name(device)));
					device->consumeEvent(evt);
				}
				XFreeEventData(m_display, &evt.xcookie);
			}
		}
	}

	return UrOk;
}

}
