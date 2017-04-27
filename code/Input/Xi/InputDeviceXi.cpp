/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Input/Xi/InputDeviceXi.h"
#include "Core/Misc/String.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace input
	{
		namespace
		{

const struct ControlConfig
{
	const wchar_t* name;
	InputDefaultControlType controlType;
	bool analogue;
	int32_t index;
}
c_controlConfig[] =
{
	{ L"Up", DtUp, false, XINPUT_GAMEPAD_DPAD_UP },
	{ L"Down", DtDown, false, XINPUT_GAMEPAD_DPAD_DOWN },
	{ L"Left", DtLeft, false, XINPUT_GAMEPAD_DPAD_LEFT },
	{ L"Right", DtRight, false, XINPUT_GAMEPAD_DPAD_RIGHT },
	{ L"Start", DtSelect, false, XINPUT_GAMEPAD_START },
	{ L"Back", DtCancel, false, XINPUT_GAMEPAD_BACK },
	{ L"Left Thumb Left/Right", DtThumbLeftX, true, -1 },
	{ L"Left Thumb Up/Down", DtThumbLeftY, true, -2 },
	{ L"Left Thumb Push", DtThumbLeftPush, false, XINPUT_GAMEPAD_LEFT_THUMB },
	{ L"Right Thumb Left/Right", DtThumbRightX, true, -3 },
	{ L"Right Thumb Up/Down", DtThumbRightY, true, -4 },
	{ L"Right Thumb Push", DtThumbRightPush, false, XINPUT_GAMEPAD_RIGHT_THUMB },
	{ L"Left Trigger", DtTriggerLeft, true, -5 },
	{ L"Right Trigger", DtTriggerRight, true, -6 },
	{ L"Left Trigger", DtTriggerLeft, false, -7 },
	{ L"Right Trigger", DtTriggerRight, false, -8 },
	{ L"Left Shoulder", DtShoulderLeft, false, XINPUT_GAMEPAD_LEFT_SHOULDER },
	{ L"Right Shoulder", DtShoulderRight, false, XINPUT_GAMEPAD_RIGHT_SHOULDER },
	{ L"Button A", DtButton1, false, XINPUT_GAMEPAD_A },
	{ L"Button B", DtButton2, false, XINPUT_GAMEPAD_B },
	{ L"Button X", DtButton3, false, XINPUT_GAMEPAD_X },
	{ L"Button Y", DtButton4, false, XINPUT_GAMEPAD_Y }
};

const uint32_t c_skipReadStateNoConnect = 15;

float adjustDeadZone(float value)
{
	if (value >= -0.2f && value <= 0.2f)
		value = 0.0f;
	return value;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.InputDeviceXi", InputDeviceXi, IInputDevice)

InputDeviceXi::InputDeviceXi(DWORD controller)
:	m_controller(controller)
,	m_connected(false)
,	m_skip(0)
{
	std::memset(&m_state, 0, sizeof(m_state));
	readState();
}

std::wstring InputDeviceXi::getName() const
{
	return L"Xbox360 gamepad " + toString< uint32_t >(m_controller);
}

InputCategory InputDeviceXi::getCategory() const
{
	return CtJoystick;
}

bool InputDeviceXi::isConnected() const
{
	return m_connected;
}

int32_t InputDeviceXi::getControlCount()
{
	return sizeof_array(c_controlConfig);
}

std::wstring InputDeviceXi::getControlName(int32_t control)
{
	return c_controlConfig[control].name;
}

bool InputDeviceXi::isControlAnalogue(int32_t control) const
{
	return c_controlConfig[control].analogue;
}

bool InputDeviceXi::isControlStable(int32_t control) const
{
	return true;
}

float InputDeviceXi::getControlValue(int32_t control)
{
	const ControlConfig& config = c_controlConfig[control];

	if (config.index >= 0)
		return ((m_state.Gamepad.wButtons & config.index) == config.index) ? 1.0f : 0.0f;
	else
	{
		switch (config.index)
		{
		case -1:
			return adjustDeadZone(m_state.Gamepad.sThumbLX / 32767.0f);
			
		case -2:
			return adjustDeadZone(m_state.Gamepad.sThumbLY / 32767.0f);

		case -3:
			return adjustDeadZone(m_state.Gamepad.sThumbRX / 32767.0f);

		case -4:
			return adjustDeadZone(m_state.Gamepad.sThumbRY / 32767.0f);

		case -5:
			return m_state.Gamepad.bLeftTrigger / 255.0f;

		case -6:
			return m_state.Gamepad.bRightTrigger / 255.0f;

		case -7:
			return (m_state.Gamepad.bLeftTrigger > 128.0f) ? 1.0f : 0.0f;

		case -8:
			return (m_state.Gamepad.bRightTrigger > 128.0f) ? 1.0f : 0.0f;
		}
	}

	return 0.0f;
}

bool InputDeviceXi::getControlRange(int32_t control, float& outMin, float& outMax) const
{
	return false;
}

bool InputDeviceXi::getDefaultControl(InputDefaultControlType controlType, bool analogue, int32_t& control) const
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

bool InputDeviceXi::getKeyEvent(KeyEvent& outEvent)
{
	return false;
}

void InputDeviceXi::resetState()
{
}

void InputDeviceXi::readState()
{
	if (!m_skip)
	{
		bool connected = m_connected;

#if !defined(_XBOX)
		HWND hWndActive = GetActiveWindow();
		DWORD dwPID = 0; GetWindowThreadProcessId(hWndActive, &dwPID);
		if (GetCurrentProcessId() != dwPID)
		{
			m_connected = false;
			m_skip = c_skipReadStateNoConnect;
		}
		else
#endif
			m_connected = bool(XInputGetState(m_controller, &m_state) == ERROR_SUCCESS);

#if defined(_DEBUG)
		if (connected && !m_connected)
			T_DEBUG(L"Input device " << int32_t(m_controller) << L" disconnected");
		else if (!connected && m_connected)
			T_DEBUG(L"Input device " << int32_t(m_controller) << L" connected");
#endif

		if (!m_connected)
			m_skip = c_skipReadStateNoConnect;
	}
	else
		m_skip--;
}

bool InputDeviceXi::supportRumble() const
{
	return true;
}

void InputDeviceXi::setRumble(const InputRumble& rumble)
{
	int32_t lf = int32_t(rumble.lowFrequencyRumble * 65535.0f);
	int32_t hf = int32_t(rumble.highFrequencyRumble * 65535.0f);

	lf = std::max< int32_t >(lf, 0);
	lf = std::min< int32_t >(lf, 65535);
	hf = std::max< int32_t >(hf, 0);
	hf = std::min< int32_t >(hf, 65535);

	XINPUT_VIBRATION xiv = { WORD(lf), WORD(hf) };
	XInputSetState(m_controller, &xiv);
}

void InputDeviceXi::setExclusive(bool exclusive)
{
}

	}
}
