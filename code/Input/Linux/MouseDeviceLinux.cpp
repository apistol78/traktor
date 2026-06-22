/*
 * TRAKTOR
 * Copyright (c) 2024-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <linux/input-event-codes.h>
#include "Core/Math/MathUtils.h"
#include "Input/Linux/MouseDeviceLinux.h"

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
const float c_mouseWheelScale = 1.0f / 10.0f;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.MouseDeviceLinux", MouseDeviceLinux, IInputDevice)

MouseDeviceLinux::MouseDeviceLinux(int32_t width, int32_t height)
:	m_connected(false)
,	m_exclusive(false)
,	m_focus(false)
,	m_wheel(0.0)
,	m_haveLast(false)
,	m_width(width > 0 ? width : 1)
,	m_height(height > 0 ? height : 1)
{
	resetState();
}

std::wstring MouseDeviceLinux::getName() const
{
	return L"Standard Mouse";
}

InputCategory MouseDeviceLinux::getCategory() const
{
	return InputCategory::Mouse;
}

bool MouseDeviceLinux::isConnected() const
{
	return m_connected;
}

int32_t MouseDeviceLinux::getControlCount()
{
	return sizeof_array(c_mouseControlMap);
}

std::wstring MouseDeviceLinux::getControlName(int32_t control)
{
	return c_mouseControlMap[control].name;
}

bool MouseDeviceLinux::isControlAnalogue(int32_t control) const
{
	return c_mouseControlMap[control].analogue;
}

bool MouseDeviceLinux::isControlStable(int32_t control) const
{
	return c_mouseControlMap[control].stable;
}

float MouseDeviceLinux::getControlValue(int32_t control)
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
		return m_button[1];
	else if (mc.controlType == DefaultControl::Button3)
		return m_button[2];
	else
		return 0.0f;
}

bool MouseDeviceLinux::getControlRange(int32_t control, float& outMin, float& outMax) const
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

bool MouseDeviceLinux::getDefaultControl(DefaultControl controlType, bool analogue, int32_t& control) const
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

bool MouseDeviceLinux::getKeyEvent(KeyEvent& outEvent)
{
	return false;
}

void MouseDeviceLinux::resetState()
{
	m_raw[0] = 0;
	m_raw[1] = 0;
	m_wheel = 0.0;

	m_axis[0] = 0.0f;
	m_axis[1] = 0.0f;
	m_axis[2] = 0.0f;

	m_position[0] = 0;
	m_position[1] = 0;

	m_last[0] = 0.0;
	m_last[1] = 0.0;
	m_haveLast = false;

	m_button[0] = 0.0f;
	m_button[1] = 0.0f;
	m_button[2] = 0.0f;
}

void MouseDeviceLinux::readState()
{
	if (!m_connected)
	{
		resetState();
		return;
	}

	// Convert accumulated raw motion into per-frame axis deltas.
	m_axis[0] = clamp(float(m_raw[0]) * c_mouseDeltaScale, -c_mouseDeltaLimit, c_mouseDeltaLimit);
	m_axis[1] = clamp(float(m_raw[1]) * c_mouseDeltaScale, -c_mouseDeltaLimit, c_mouseDeltaLimit);
	m_axis[2] = clamp(float(-m_wheel * c_mouseWheelScale), -c_mouseDeltaLimit, c_mouseDeltaLimit);

	m_raw[0] = 0;
	m_raw[1] = 0;
	m_wheel = 0.0;
}

bool MouseDeviceLinux::supportRumble() const
{
	return false;
}

void MouseDeviceLinux::setRumble(const InputRumble& /*rumble*/)
{
}

void MouseDeviceLinux::setExclusive(bool exclusive)
{
	if (exclusive == m_exclusive)
		return;

	m_exclusive = exclusive;

	// Drop any accumulated motion and reset the delta baseline so the mode
	// switch doesn't inject a spurious jump.
	m_raw[0] = 0;
	m_raw[1] = 0;
	m_haveLast = false;
}

void MouseDeviceLinux::setFocus(bool focus)
{
	m_focus = focus;
	m_connected = focus;
	if (!focus)
	{
		// Release held buttons and motion baseline on focus loss.
		m_button[0] = m_button[1] = m_button[2] = 0.0f;
		m_haveLast = false;
	}
}

void MouseDeviceLinux::setSize(int32_t width, int32_t height)
{
	if (width > 0)
		m_width = width;
	if (height > 0)
		m_height = height;
}

void MouseDeviceLinux::onPointerEnter(double x, double y)
{
	m_position[0] = clamp((int32_t)x, 0, m_width - 1);
	m_position[1] = clamp((int32_t)y, 0, m_height - 1);
	m_last[0] = x;
	m_last[1] = y;
	m_haveLast = true;
}

void MouseDeviceLinux::onPointerMotion(double x, double y)
{
	if (m_exclusive)
		return;

	if (m_haveLast)
	{
		m_raw[0] += (int32_t)(x - m_last[0]);
		m_raw[1] += (int32_t)(y - m_last[1]);
	}

	m_last[0] = x;
	m_last[1] = y;
	m_haveLast = true;

	m_position[0] = clamp((int32_t)x, 0, m_width - 1);
	m_position[1] = clamp((int32_t)y, 0, m_height - 1);
}

void MouseDeviceLinux::onRelativeMotion(double dx, double dy)
{
	if (!m_exclusive)
		return;

	m_raw[0] += (int32_t)dx;
	m_raw[1] += (int32_t)dy;
}

void MouseDeviceLinux::onButton(uint32_t button, bool down)
{
	const float v = down ? 1.0f : 0.0f;
	switch (button)
	{
	case BTN_LEFT:
		m_button[0] = v;
		break;
	case BTN_RIGHT:
		m_button[1] = v;
		break;
	case BTN_MIDDLE:
		m_button[2] = v;
		break;
	default:
		break;
	}
}

void MouseDeviceLinux::onWheel(double value)
{
	m_wheel += value;
}

}
