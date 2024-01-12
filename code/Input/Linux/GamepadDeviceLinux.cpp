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
#include <linux/joystick.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include "Core/Log/Log.h"
#include "Input/Linux/GamepadDeviceLinux.h"

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
}
c_controlConfig[] =
{
	{ L"Up", DefaultControl::Up, false, -7 },
	{ L"Down", DefaultControl::Down, false, -8 },
	{ L"Left", DefaultControl::Left, false, -9 },
	{ L"Right", DefaultControl::Right, false, -10 },
	// { L"Start", DefaultControl::Select, false, XINPUT_GAMEPAD_START },
	// { L"Back", DefaultControl::Cancel, false, XINPUT_GAMEPAD_BACK },
	{ L"Left Thumb Left/Right", DefaultControl::ThumbLeftX, true, -1 },
	{ L"Left Thumb Up/Down", DefaultControl::ThumbLeftY, true, -2 },
	// { L"Left Thumb Push", DefaultControl::ThumbLeftPush, false, XINPUT_GAMEPAD_LEFT_THUMB },
	{ L"Right Thumb Left/Right", DefaultControl::ThumbRightX, true, -3 },
	{ L"Right Thumb Up/Down", DefaultControl::ThumbRightY, true, -4 },
	// { L"Right Thumb Push", DefaultControl::ThumbRightPush, false, XINPUT_GAMEPAD_RIGHT_THUMB },
	{ L"Left Trigger", DefaultControl::TriggerLeft, true, -5 },
	{ L"Right Trigger", DefaultControl::TriggerRight, true, -6 },
	// { L"Left Trigger", DefaultControl::TriggerLeft, false, -7 },
	// { L"Right Trigger", DefaultControl::TriggerRight, false, -8 },
	// { L"Left Shoulder", DefaultControl::ShoulderLeft, false, XINPUT_GAMEPAD_LEFT_SHOULDER },
	// { L"Right Shoulder", DefaultControl::ShoulderRight, false, XINPUT_GAMEPAD_RIGHT_SHOULDER },
	{ L"Button A", DefaultControl::Button1, false, 0 },
	{ L"Button B", DefaultControl::Button2, false, 1 },
	{ L"Button X", DefaultControl::Button3, false, 2 },
	{ L"Button Y", DefaultControl::Button4, false, 3 }
};

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.GamepadDeviceLinux", GamepadDeviceLinux, IInputDevice)

GamepadDeviceLinux::GamepadDeviceLinux(int fd)
:   m_fd(fd)
{
	resetState();
}

GamepadDeviceLinux::~GamepadDeviceLinux()
{
	close(m_fd);
}

std::wstring GamepadDeviceLinux::getName() const
{
	return L"Gamepad";
}

InputCategory GamepadDeviceLinux::getCategory() const
{
	return InputCategory::Joystick;
}

bool GamepadDeviceLinux::isConnected() const
{
	return true;
}

int32_t GamepadDeviceLinux::getControlCount()
{
	return sizeof_array(c_controlConfig);
}

std::wstring GamepadDeviceLinux::getControlName(int32_t control)
{
	return c_controlConfig[control].name;
}

bool GamepadDeviceLinux::isControlAnalogue(int32_t control) const
{
	return c_controlConfig[control].analogue;
}

bool GamepadDeviceLinux::isControlStable(int32_t control) const
{
	return true;
}

float GamepadDeviceLinux::getControlValue(int32_t control)
{
	const ControlConfig& config = c_controlConfig[control];
	if (config.index >= 0)
		return m_buttons[config.index] != 0 ? 1.0f : 0.0f;
	else
	{
		switch (config.index)
		{
		case -1:
			return m_leftThumbX / 32767.0f;
		case -2:
			return m_leftThumbY / 32767.0f;
		case -3:
			return m_rightThumbX / 32767.0f;
		case -4:
			return m_rightThumbY / 32767.0f;
		case -5:
			return m_leftTrigger / 65535.0f;
		case -6:
			return m_rightTrigger / 65535.0f;
		case -7:
			return m_dpadY < 0 ? 1.0f : 0.0f;
		case -8:
			return m_dpadY > 0 ? 1.0f : 0.0f;
		case -9:
			return m_dpadX < 0 ? 1.0f : 0.0f;
		case -10:
			return m_dpadX > 0 ? 1.0f : 0.0f;
		default:
			break;
		}
	}
	return 0.0f;
}

bool GamepadDeviceLinux::getControlRange(int32_t control, float& outMin, float& outMax) const
{
	return false;
}

bool GamepadDeviceLinux::getDefaultControl(DefaultControl controlType, bool analogue, int32_t& control) const
{
	for (uint32_t i = 0; i < sizeof_array(c_controlConfig); ++i)
	{
		if (c_controlConfig[i].controlType == controlType && c_controlConfig[i].analogue == analogue)
		{
			control = i;
			return true;
		}
	}
	return false;
}

bool GamepadDeviceLinux::getKeyEvent(KeyEvent& outEvent)
{
	return false;
}

void GamepadDeviceLinux::resetState()
{
	m_buttons[0] =
	m_buttons[1] =
	m_buttons[2] =
	m_buttons[3] = 0;
	m_leftThumbX = 0;
	m_leftThumbY = 0;
	m_rightThumbX = 0;
	m_rightThumbY = 0;
	m_leftTrigger = 0;
	m_rightTrigger = 0;
	m_dpadX = 0;
	m_dpadY = 0;
}

void GamepadDeviceLinux::readState()
{
	js_event events[8];
	
	const ssize_t nr = read(m_fd, events, sizeof(events));
	if (nr <= 0)
		return;

	const ssize_t count = (size_t)(nr / sizeof(js_event));
	for (ssize_t i = 0; i < count; ++i)
	{
		const auto& e = events[i];
		switch (e.type)
		{
		case JS_EVENT_BUTTON:
			{
				//log::info << (int)e.number << L" == " << e.value << Endl;
				switch (e.number)
				{
				case 0:
					m_buttons[0] = e.value;	// A
					break;
				case 1:
					m_buttons[1] = e.value;	// B
					break;
				case 3:
					m_buttons[2] = e.value;	// X
					break;
				case 4:
					m_buttons[3] = e.value;	// Y
					break;
				default:
					/*
					11 = Burger
					7 = Right sh
					6 = Left sh
					13 = Left stick p
					14 = Right stick p
					*/
					break;
				}
			}
			break;

		case JS_EVENT_AXIS:
			{
				// log::info << (int)e.number << L" == " << e.value << Endl;
				switch (e.number)
				{
				case 0:
					m_leftThumbX = e.value;
					break;
				case 1:
					m_leftThumbY = e.value;
					break;
				case 2:
					m_rightThumbX = e.value;
					break;
				case 3:
					m_rightThumbY = e.value;
					break;
				case 4:
					m_rightTrigger = (int)e.value + 32767;
					break;
				case 5:
					m_leftTrigger = (int)e.value + 32767;
					break;
				case 6:
					m_dpadX = e.value;
					break;
				case 7:
					m_dpadY = e.value;
					break;
				}
			}

		default:
			//log::info << L"unknown type " << e.type << Endl;
			break;
		}
	}
}

bool GamepadDeviceLinux::supportRumble() const
{
	return false;
}

void GamepadDeviceLinux::setRumble(const InputRumble& /*rumble*/)
{
}

void GamepadDeviceLinux::setExclusive(bool exclusive)
{
}

}
