/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Input/GameInput/GamepadDeviceGameInput.h"

#include "Core/Io/Utf8Encoding.h"
#include "Core/Math/MathUtils.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"

namespace traktor::input
{
namespace
{

const struct ControlConfig
{
	const wchar_t* name;
	DefaultControl controlType;
	bool analogue;
	int32_t index;
} c_controlConfig[] = {
	{ L"Up", DefaultControl::Up, false, GameInputGamepadDPadUp },
	{ L"Down", DefaultControl::Down, false, GameInputGamepadDPadDown },
	{ L"Left", DefaultControl::Left, false, GameInputGamepadDPadLeft },
	{ L"Right", DefaultControl::Right, false, GameInputGamepadDPadRight },
	{ L"Menu", DefaultControl::Select, false, GameInputGamepadMenu },
	{ L"View", DefaultControl::Cancel, false, GameInputGamepadView },
	{ L"Left Thumb Left/Right", DefaultControl::ThumbLeftX, true, -1 },
	{ L"Left Thumb Up/Down", DefaultControl::ThumbLeftY, true, -2 },
	{ L"Left Thumb Push", DefaultControl::ThumbLeftPush, false, GameInputGamepadLeftThumbstick },
	{ L"Right Thumb Left/Right", DefaultControl::ThumbRightX, true, -3 },
	{ L"Right Thumb Up/Down", DefaultControl::ThumbRightY, true, -4 },
	{ L"Right Thumb Push", DefaultControl::ThumbRightPush, false, GameInputGamepadRightThumbstick },
	{ L"Left Trigger", DefaultControl::TriggerLeft, true, -5 },
	{ L"Right Trigger", DefaultControl::TriggerRight, true, -6 },
	{ L"Left Trigger", DefaultControl::TriggerLeft, false, -7 },
	{ L"Right Trigger", DefaultControl::TriggerRight, false, -8 },
	{ L"Left Shoulder", DefaultControl::ShoulderLeft, false, GameInputGamepadLeftShoulder },
	{ L"Right Shoulder", DefaultControl::ShoulderRight, false, GameInputGamepadRightShoulder },
	{ L"Button A", DefaultControl::Button1, false, GameInputGamepadA },
	{ L"Button B", DefaultControl::Button2, false, GameInputGamepadB },
	{ L"Button X", DefaultControl::Button3, false, GameInputGamepadX },
	{ L"Button Y", DefaultControl::Button4, false, GameInputGamepadY }
};

float adjustDeadZone(float value)
{
	return (value >= -0.2f && value <= 0.2f) ? 0.0f : value;
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.GamepadDeviceGameInput", GamepadDeviceGameInput, IInputDevice)

GamepadDeviceGameInput::GamepadDeviceGameInput(IGameInput* gameInput, IGameInputDevice* device, int32_t index)
	: m_gameInput(gameInput)
	, m_device(device)
{
	const GameInputString* displayName = m_device->GetDeviceInfo()->displayName;
	if (displayName != nullptr && displayName->data != nullptr)
		m_name = mbstows(Utf8Encoding(), displayName->data);
	else
		m_name = L"Gamepad " + toString(index);

	resetState();
}

void GamepadDeviceGameInput::destroy()
{
	m_device->SetRumbleState(nullptr);
	m_connected = false;
}

std::wstring GamepadDeviceGameInput::getName() const
{
	return m_name;
}

InputCategory GamepadDeviceGameInput::getCategory() const
{
	return InputCategory::Joystick;
}

bool GamepadDeviceGameInput::isConnected() const
{
	return m_connected;
}

int32_t GamepadDeviceGameInput::getControlCount()
{
	return sizeof_array(c_controlConfig);
}

std::wstring GamepadDeviceGameInput::getControlName(int32_t control)
{
	return c_controlConfig[control].name;
}

bool GamepadDeviceGameInput::isControlAnalogue(int32_t control) const
{
	return c_controlConfig[control].analogue;
}

bool GamepadDeviceGameInput::isControlStable(int32_t control) const
{
	return true;
}

float GamepadDeviceGameInput::getControlValue(int32_t control)
{
	const ControlConfig& config = c_controlConfig[control];
	if (config.index >= 0)
		return (m_state.buttons & config.index) != 0 ? 1.0f : 0.0f;

	switch (config.index)
	{
	case -1:
		return adjustDeadZone(m_state.leftThumbstickX);
	case -2:
		return adjustDeadZone(m_state.leftThumbstickY);
	case -3:
		return adjustDeadZone(m_state.rightThumbstickX);
	case -4:
		return adjustDeadZone(m_state.rightThumbstickY);
	case -5:
		return m_state.leftTrigger;
	case -6:
		return m_state.rightTrigger;
	case -7:
		return m_state.leftTrigger > 0.5f ? 1.0f : 0.0f;
	case -8:
		return m_state.rightTrigger > 0.5f ? 1.0f : 0.0f;
	default:
		return 0.0f;
	}
}

bool GamepadDeviceGameInput::getControlRange(int32_t control, float& outMin, float& outMax) const
{
	return false;
}

bool GamepadDeviceGameInput::getDefaultControl(DefaultControl controlType, bool analogue, int32_t& control) const
{
	for (int32_t i = 0; i < (int32_t)sizeof_array(c_controlConfig); ++i)
	{
		if (c_controlConfig[i].controlType == controlType && c_controlConfig[i].analogue == analogue)
		{
			control = i;
			return true;
		}
	}
	return false;
}

bool GamepadDeviceGameInput::getKeyEvent(KeyEvent& outEvent)
{
	return false;
}

void GamepadDeviceGameInput::resetState()
{
	std::memset(&m_state, 0, sizeof(m_state));
}

void GamepadDeviceGameInput::readState()
{
	resetState();

	m_connected = (m_device->GetDeviceStatus() & GameInputDeviceConnected) != 0;
	if (!m_connected)
		return;

	ComRef< IGameInputReading > reading;
	if (SUCCEEDED(m_gameInput->GetCurrentReading(GameInputKindGamepad, m_device, &reading.getAssign())))
		reading->GetGamepadState(&m_state);
}

bool GamepadDeviceGameInput::supportRumble() const
{
	return (m_device->GetDeviceInfo()->supportedRumbleMotors & (GameInputRumbleLowFrequency | GameInputRumbleHighFrequency)) != 0;
}

void GamepadDeviceGameInput::setRumble(const InputRumble& rumble)
{
	const GameInputRumbleParams params = {
		.lowFrequency = clamp(rumble.lowFrequencyRumble, 0.0f, 1.0f),
		.highFrequency = clamp(rumble.highFrequencyRumble, 0.0f, 1.0f)
	};
	m_device->SetRumbleState(&params);
}

void GamepadDeviceGameInput::setExclusive(bool exclusive)
{
}

}
