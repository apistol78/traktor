/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Input/GameInput/MouseDeviceGameInput.h"

#include "Core/Math/MathUtils.h"

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
} c_mouseControlMap[] = {
	{ L"Left mouse button", DefaultControl::Button1, false, true },
	{ L"Right mouse button", DefaultControl::Button2, false, true },
	{ L"Middle mouse button", DefaultControl::Button3, false, true },
	{ L"Mouse button 4", DefaultControl::Button4, false, true },
	{ L"Mouse button 5", DefaultControl::Button5, false, true },
	{ L"Mouse X axis", DefaultControl::AxisX, true, false },
	{ L"Mouse Y axis", DefaultControl::AxisY, true, false },
	{ L"Mouse Z axis", DefaultControl::AxisZ, true, false },
	{ L"Mouse X position", DefaultControl::PositionX, true, false },
	{ L"Mouse Y position", DefaultControl::PositionY, true, false }
};

const GameInputMouseButtons c_mouseButtons[] = {
	GameInputMouseLeftButton,
	GameInputMouseRightButton,
	GameInputMouseMiddleButton,
	GameInputMouseButton4,
	GameInputMouseButton5
};

const float c_mouseDeltaLimit = 100.0f;

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.MouseDeviceGameInput", MouseDeviceGameInput, IInputDevice)

MouseDeviceGameInput::MouseDeviceGameInput(IGameInput* gameInput, HWND hWnd)
	: m_gameInput(gameInput)
	, m_hWnd(hWnd)
{
}

void MouseDeviceGameInput::destroy()
{
	if (m_clipped)
	{
		ClipCursor(nullptr);
		m_clipped = false;
	}
	m_mice.clear();
	m_connected = false;
}

std::wstring MouseDeviceGameInput::getName() const
{
	return L"Mouse";
}

InputCategory MouseDeviceGameInput::getCategory() const
{
	return InputCategory::Mouse;
}

bool MouseDeviceGameInput::isConnected() const
{
	return m_connected;
}

int32_t MouseDeviceGameInput::getControlCount()
{
	return sizeof_array(c_mouseControlMap);
}

std::wstring MouseDeviceGameInput::getControlName(int32_t control)
{
	return c_mouseControlMap[control].name;
}

bool MouseDeviceGameInput::isControlAnalogue(int32_t control) const
{
	return c_mouseControlMap[control].analogue;
}

bool MouseDeviceGameInput::isControlStable(int32_t control) const
{
	return c_mouseControlMap[control].stable;
}

float MouseDeviceGameInput::getControlValue(int32_t control)
{
	if (!m_connected)
		return 0.0f;

	if (control < (int32_t)sizeof_array(c_mouseButtons))
		return (m_buttons & c_mouseButtons[control]) != 0 ? 1.0f : 0.0f;

	switch (c_mouseControlMap[control].controlType)
	{
	case DefaultControl::AxisX:
		return clamp(float(m_deltaX), -c_mouseDeltaLimit, c_mouseDeltaLimit);
	case DefaultControl::AxisY:
		return clamp(float(m_deltaY), -c_mouseDeltaLimit, c_mouseDeltaLimit);
	case DefaultControl::AxisZ:
		return float(m_deltaWheel) / WHEEL_DELTA;
	case DefaultControl::PositionX:
		return float(m_position.x);
	case DefaultControl::PositionY:
		return float(m_position.y);
	default:
		return 0.0f;
	}
}

bool MouseDeviceGameInput::getControlRange(int32_t control, float& outMin, float& outMax) const
{
	if (control < (int32_t)sizeof_array(c_mouseButtons))
	{
		outMin = 0.0f;
		outMax = 1.0f;
		return true;
	}

	switch (c_mouseControlMap[control].controlType)
	{
	case DefaultControl::AxisX:
	case DefaultControl::AxisY:
		outMin = -c_mouseDeltaLimit;
		outMax = c_mouseDeltaLimit;
		return true;
	case DefaultControl::PositionX:
		outMin = float(m_rect.left);
		outMax = float(m_rect.right);
		return true;
	case DefaultControl::PositionY:
		outMin = float(m_rect.top);
		outMax = float(m_rect.bottom);
		return true;
	default:
		return false;
	}
}

bool MouseDeviceGameInput::getDefaultControl(DefaultControl controlType, bool analogue, int32_t& control) const
{
	for (int32_t i = 0; i < (int32_t)sizeof_array(c_mouseControlMap); ++i)
	{
		const MouseControlMap& mc = c_mouseControlMap[i];
		if (mc.controlType == controlType && mc.analogue == analogue)
		{
			control = i;
			return true;
		}
	}
	return false;
}

bool MouseDeviceGameInput::getKeyEvent(KeyEvent& outEvent)
{
	return false;
}

void MouseDeviceGameInput::resetState()
{
	m_buttons = GameInputMouseNone;
	m_deltaX = 0;
	m_deltaY = 0;
	m_deltaWheel = 0;
	m_position = { 0, 0 };
}

void MouseDeviceGameInput::readState()
{
	resetState();

	// Readings from before focus was regained can hold buttons which has since been released.
	const bool connected = (GetForegroundWindow() == GetAncestor(m_hWnd, GA_ROOT));
	if (connected && !m_connected)
		m_connectedTimestamp = m_gameInput->GetCurrentTimestamp();
	m_connected = connected;

	GetClientRect(m_hWnd, &m_rect);

	// Keep cursor inside window while exclusive.
	const bool clip = m_connected && m_exclusive;
	if (clip)
	{
		RECT clipRect = m_rect;
		MapWindowPoints(m_hWnd, nullptr, (POINT*)&clipRect, 2);
		ClipCursor(&clipRect);
	}
	else if (m_clipped)
		ClipCursor(nullptr);
	m_clipped = clip;

	if (!m_connected)
		return;

	// Positions are accumulated deltas, so movement is the difference from the previous reading.
	for (auto& mouse : m_mice)
	{
		ComRef< IGameInputReading > reading;
		if (FAILED(m_gameInput->GetCurrentReading(GameInputKindMouse, mouse.device, &reading.getAssign())))
			continue;

		GameInputMouseState state;
		if (!reading->GetMouseState(&state))
			continue;

		if (mouse.valid)
		{
			m_deltaX += state.positionX - mouse.positionX;
			m_deltaY += state.positionY - mouse.positionY;
			m_deltaWheel += state.wheelY - mouse.wheelY;
		}

		mouse.positionX = state.positionX;
		mouse.positionY = state.positionY;
		mouse.wheelY = state.wheelY;
		mouse.valid = true;

		if (reading->GetTimestamp() >= m_connectedTimestamp)
			m_buttons |= state.buttons;
	}

	GetCursorPos(&m_position);
	ScreenToClient(m_hWnd, &m_position);
	m_position.x = clamp< LONG >(m_position.x, 0, std::max< LONG >(m_rect.right - 1, 0));
	m_position.y = clamp< LONG >(m_position.y, 0, std::max< LONG >(m_rect.bottom - 1, 0));
}

bool MouseDeviceGameInput::supportRumble() const
{
	return false;
}

void MouseDeviceGameInput::setRumble(const InputRumble& rumble)
{
}

void MouseDeviceGameInput::setExclusive(bool exclusive)
{
	m_exclusive = exclusive;
}

void MouseDeviceGameInput::addDevice(IGameInputDevice* device)
{
	for (const auto& mouse : m_mice)
		if (mouse.device == device)
			return;
	m_mice.push_back().device = device;
}

}
