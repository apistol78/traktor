/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Math/MathUtils.h"
#include "Input/X11/MouseDeviceX11.h"

namespace traktor::input
{
	namespace
	{

const struct MouseControlMap
{
	const wchar_t* name;
	DefaultControl controlType;
	bool analogue;
	bool stable;
}
c_mouseControlMap[] =
{
	{ L"Left mouse button", DefaultControl::Button1, false, true },
	{ L"Right mouse button", DefaultControl::Button2, false, true },
	{ L"Middle mouse button", DefaultControl::Button3, false, true },
	{ L"Mouse X axis", DefaultControl::AxisX, true, true },
	{ L"Mouse Y axis", DefaultControl::AxisY, true, true },
	{ L"Mouse Z axis", DefaultControl::AxisZ, true, true },
	{ L"Mouse X axis", DefaultControl::PositionX, true, false },
	{ L"Mouse Y axis", DefaultControl::PositionY, true, false }
};

const float c_mouseDeltaScale = 0.5f;
const float c_mouseDeltaLimit = 200.0f;
const float c_mouseWheelDelta = 1.0f;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.MouseDeviceX11", MouseDeviceX11, InputDeviceX11)

MouseDeviceX11::MouseDeviceX11(Display* display, Window window, int deviceId)
:	m_display(display)
,	m_window(window)
,	m_deviceId(deviceId)
,	m_connected(true)
,	m_exclusive(false)
,	m_focus(false)
,	m_haveGrab(false)
,	m_width(0)
,	m_height(0)
{
	uint8_t mask[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	XIEventMask evmask;

	// Select motion from user window.
	evmask.mask = mask;
	evmask.mask_len = sizeof(mask);
	evmask.deviceid = m_deviceId;

	XISetMask(mask, XI_Motion);
	XISetMask(mask, XI_ButtonPress);
	XISetMask(mask, XI_ButtonRelease);
	XISetMask(mask, XI_FocusIn);
	XISetMask(mask, XI_FocusOut);
	XISelectEvents(m_display, m_window, &evmask, 1);

	// Get current size of window.
	XWindowAttributes attr;
	XGetWindowAttributes(m_display, m_window, &attr);
	m_width = attr.width;
	m_height = attr.height;

	resetState();
}

MouseDeviceX11::~MouseDeviceX11()
{
	setExclusive(false);
}

std::wstring MouseDeviceX11::getName() const
{
	return L"Standard Mouse";
}

InputCategory MouseDeviceX11::getCategory() const
{
	return InputCategory::Mouse;
}

bool MouseDeviceX11::isConnected() const
{
	return m_connected;
}

int32_t MouseDeviceX11::getControlCount()
{
	return sizeof_array(c_mouseControlMap);
}

std::wstring MouseDeviceX11::getControlName(int32_t control)
{
	return c_mouseControlMap[control].name;
}

bool MouseDeviceX11::isControlAnalogue(int32_t control) const
{
	return c_mouseControlMap[control].analogue;
}

bool MouseDeviceX11::isControlStable(int32_t control) const
{
	return c_mouseControlMap[control].stable;
}

float MouseDeviceX11::getControlValue(int32_t control)
{
	if (!m_connected || control < 0)
		return 0.0f;

	const MouseControlMap& mc = c_mouseControlMap[control];
	if (mc.controlType == DefaultControl::AxisX)
		return m_axis[0];
	else if (mc.controlType == DefaultControl::AxisY)
		return m_axis[1];
	else if (mc.controlType == DefaultControl::AxisZ)
		return m_axis[2];
	else if (mc.controlType == DefaultControl::PositionX)
		return float(m_position[0]);
	else if (mc.controlType == DefaultControl::PositionY)
		return float(m_position[1]);
	else if (mc.controlType == DefaultControl::Button1)
		return m_button[0];
	else if (mc.controlType == DefaultControl::Button2)
		return m_button[2];
	else if (mc.controlType == DefaultControl::Button3)
		return m_button[1];
	else
		return 0.0f;
}

bool MouseDeviceX11::getControlRange(int32_t control, float& outMin, float& outMax) const
{
	if (!m_connected || control < 0)
		return false;

	const MouseControlMap& mc = c_mouseControlMap[control];
	if (mc.controlType == DefaultControl::PositionX)
	{
		outMin = 0;
		outMax = float(m_width);
	}
	else if (mc.controlType == DefaultControl::PositionY)
	{
		outMin = 0;
		outMax = float(m_height);
	}
	else
		return false;

	return true;
}

bool MouseDeviceX11::getDefaultControl(DefaultControl controlType, bool analogue, int32_t& control) const
{
	for (uint32_t i = 0; i < sizeof_array(c_mouseControlMap); ++i)
	{
		const MouseControlMap& mc = c_mouseControlMap[i];
		if (mc.controlType == controlType && mc.analogue == analogue)
		{
			control = i;
			return true;
		}
	}
	control = -1;
	return false;
}

bool MouseDeviceX11::getKeyEvent(KeyEvent& outEvent)
{
	return false;
}

void MouseDeviceX11::resetState()
{
	m_raw[0] = 0;
	m_raw[1] = 0;
	m_raw[2] = 0;

	m_axis[0] = 0.0f;
	m_axis[1] = 0.0f;
	m_axis[2] = 0.0f;

	m_position[0] = 0;
	m_position[1] = 0;

	m_warped[0] = -1;
	m_warped[1] = -1;

	m_button[0] = 0.0f;
	m_button[1] = 0.0f;
	m_button[2] = 0.0f;
}

void MouseDeviceX11::readState()
{
	if (!m_connected)
		resetState();

	// Update virtual position of pointer.
	if (m_exclusive && m_focus)
	{
		m_position[0] = clamp(m_position[0] + m_raw[0], 0, m_width - 1);
		m_position[1] = clamp(m_position[1] + m_raw[1], 0, m_height - 1);
	}

	// Calculate axis deltas from raw deltas.
	for (int i = 0; i < 3; ++i)
	{
		m_axis[i] = clamp(float(m_raw[i] * c_mouseDeltaScale), -c_mouseDeltaLimit, c_mouseDeltaLimit);
		m_raw[i] = 0;
	}

	if (m_exclusive && m_focus)
	{
		// Warp pointer to center of window.
		m_warped[0] = m_width / 2;
		m_warped[1] = m_height / 2;

		XWarpPointer(
			m_display,
			None,
			m_window,
			0,
			0,
			0,
			0,
			m_warped[0],
			m_warped[1]
		);
	}
}

bool MouseDeviceX11::supportRumble() const
{
	return false;
}

void MouseDeviceX11::setRumble(const InputRumble& /*rumble*/)
{
}

void MouseDeviceX11::setExclusive(bool exclusive)
{
	m_exclusive = exclusive;

	const bool shouldGrab = (exclusive && m_focus);
	if (shouldGrab == m_haveGrab)
		return;

	if (shouldGrab)
	{
		uint8_t mask[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
		XIEventMask evmask;

		evmask.mask = mask;
		evmask.mask_len = sizeof(mask);
		evmask.deviceid = XIAllDevices;

		XISetMask(mask, XI_Motion);
		XISetMask(mask, XI_ButtonPress);
		XISetMask(mask, XI_ButtonRelease);
		XISetMask(mask, XI_FocusIn);
		XISetMask(mask, XI_FocusOut);

#if !defined(_DEBUG)
		XIGrabDevice(
			m_display,
			m_deviceId,
			m_window,
			CurrentTime,
			None,
			GrabModeAsync,
			GrabModeAsync,
			False,
			&evmask
		);
#endif
	}
	else
	{
#if !defined(_DEBUG)
		XIUngrabDevice(m_display, m_deviceId, CurrentTime);
#endif
	}

	XFlush(m_display);
	m_haveGrab = shouldGrab;

	// Also update current size of window here; size is only
	// used when in exclusive mode and we assume window size
	// doesn't change after entering exclusive mode.
	// We do this because it's too expensive to query window
	// size frequently and we don't own the message loop.
	XWindowAttributes attr;
	XGetWindowAttributes(m_display, m_window, &attr);
	m_width = attr.width;
	m_height = attr.height;	
}

void MouseDeviceX11::consumeEvent(XEvent& evt)
{
	XIEvent* xi = (XIEvent*)evt.xcookie.data;
	switch (xi->evtype)
	{
	case XI_Motion:
		{
			XIDeviceEvent* event = (XIDeviceEvent*)evt.xcookie.data;
			if (event->deviceid != m_deviceId)
				return;

			// Need to check if event's position differ from warped position.
			if (event->event_x != m_warped[0] || event->event_y != m_warped[1])
			{
				if (!m_exclusive || !m_focus)
				{
					const int32_t dx = event->event_x - m_position[0];
					const int32_t dy = event->event_y - m_position[1];

					m_raw[0] += dx;
					m_raw[1] += dy;
				}
				else
				{
					const int32_t cx = m_width / 2;
					const int32_t cy = m_height / 2;

					// In exclusive mode the pointer is continiously warped back to center,
					// thus the motion is relative center.
					const int32_t dx = event->event_x - cx;
					const int32_t dy = event->event_y - cy;

					m_raw[0] += dx;
					m_raw[1] += dy;
				}
			}

			// In exclusive mode the position is accumulated in readState.
			if (!m_exclusive || !m_focus)
			{
				m_position[0] = event->event_x;
				m_position[1] = event->event_y;
			}

			m_warped[0] = -1;
			m_warped[1] = -1;
		}
		break;

	case XI_ButtonPress:
	case XI_ButtonRelease:
		{
			XIDeviceEvent* event = (XIDeviceEvent*)evt.xcookie.data;
			if (event->deviceid != m_deviceId)
				return;

			if (event->detail >= 1 && event->detail <= 3)
				m_button[event->detail - 1] = (xi->evtype == XI_ButtonPress) ? 1.0f : 0.0f;
			else if (xi->evtype == XI_ButtonPress && (event->detail == 4 || event->detail == 5))
				m_raw[2] += (event->detail == 4) ? c_mouseWheelDelta : -c_mouseWheelDelta;
		}
		break;

	case XI_FocusIn:
		setFocus(true);
		break;

	case XI_FocusOut:
		setFocus(false);
		break;		

	default:
		break;
	}
}

void MouseDeviceX11::setFocus(bool focus)
{
	m_focus = focus;
	m_connected = focus;
	setExclusive(m_exclusive);
}

}
