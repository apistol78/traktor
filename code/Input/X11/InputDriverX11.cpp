/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Input/X11/InputDriverX11.h"
#include "Input/X11/KeyboardDeviceX11.h"
#include "Input/X11/MouseDeviceX11.h"

namespace traktor
{
    namespace input
    {

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.InputDriverX11", 0, InputDriverX11, IInputDriver)

InputDriverX11::InputDriverX11()
:	m_display(0)
,	m_window(None)
,	m_opcode(0)
{
}

bool InputDriverX11::create(const SystemApplication& sysapp, const SystemWindow& syswin, uint32_t inputCategories)
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
	XEvent evt;
	while (XCheckTypedEvent(m_display, GenericEvent, &evt))
	{
		if (evt.xcookie.type == GenericEvent && evt.xcookie.extension == m_opcode)
		{
			if (XGetEventData(m_display, &evt.xcookie))
			{
				for (RefArray< InputDeviceX11 >::iterator i = m_devices.begin(); i != m_devices.end(); ++i)
					(*i)->consumeEvent(evt);

				XFreeEventData(m_display, &evt.xcookie);
			}
		}
	}

	// Check so our window still has input focus; release exlusive if not in focus.
	::Window focusWindow;
	int focusState;

	XGetInputFocus(m_display, &focusWindow, &focusState);

	for (RefArray< InputDeviceX11 >::iterator i = m_devices.begin(); i != m_devices.end(); ++i)
		(*i)->setFocus(focusWindow == m_window);

	return UrOk;
}

    }
}
