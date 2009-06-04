#include "Input/Xi/InputDeviceXi.h"
#include "Core/Misc/String.h"

namespace traktor
{
	namespace input
	{
		namespace
		{

float adjustDeadZone(float value)
{
	if (value >= -0.2f && value <= 0.2f)
		value = 0.0f;
	return value;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.InputDeviceXi", InputDeviceXi, InputDevice)

InputDeviceXi::InputDeviceXi(DWORD controller)
:	m_controller(controller)
,	m_connected(false)
{
	std::memset(&m_state, 0, sizeof(m_state));
	readState();
}

std::wstring InputDeviceXi::getName() const
{
	return L"Joystick " + toString(m_controller);
}

InputCategory InputDeviceXi::getCategory() const
{
	return CtJoystick;
}

bool InputDeviceXi::isConnected() const
{
	return m_connected;
}

int InputDeviceXi::getControlCount()
{
	return 0;
}

std::wstring InputDeviceXi::getControlName(int control)
{
	return L"";
}

bool InputDeviceXi::isControlAnalogue(int control) const
{
	return control < 0;
}

float InputDeviceXi::getControlValue(int control)
{
	if (control > 0)
		return ((m_state.Gamepad.wButtons & control) == control) ? 1.0f : 0.0f;
	else if (control < 0)
	{
		switch (control)
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
		}
	}
	return 0.0f;
}

bool InputDeviceXi::getDefaultControl(InputDefaultControlType controlType, int& control) const
{
	switch (controlType)
	{
	case DtUp:
		control = XINPUT_GAMEPAD_DPAD_UP;
		break;

	case DtDown:
		control = XINPUT_GAMEPAD_DPAD_DOWN;
		break;

	case DtLeft:
		control = XINPUT_GAMEPAD_DPAD_LEFT;
		break;

	case DtRight:
		control = XINPUT_GAMEPAD_DPAD_RIGHT;
		break;

	case DtSelect:
		control = XINPUT_GAMEPAD_START;
		break;

	case DtCancel:
		control = XINPUT_GAMEPAD_BACK;
		break;

	case DtThumbLeftX:
		control = -1;
		break;

	case DtThumbLeftY:
		control = -2;
		break;

	case DtThumbLeftPush:
		control = XINPUT_GAMEPAD_LEFT_THUMB;
		break;

	case DtThumbRightX:
		control = -3;
		break;

	case DtThumbRightY:
		control = -4;
		break;

	case DtThumbRightPush:
		control = XINPUT_GAMEPAD_RIGHT_THUMB;
		break;

	case DtTriggerLeft:
		control = -5;
		break;

	case DtTriggerRight:
		control = -6;
		break;

	case DtShoulderLeft:
		control = XINPUT_GAMEPAD_LEFT_SHOULDER;
		break;

	case DtShoulderRight:
		control = XINPUT_GAMEPAD_RIGHT_SHOULDER;
		break;

	case DtButton1:
		control = XINPUT_GAMEPAD_A;
		break;

	case DtButton2:
		control = XINPUT_GAMEPAD_B;
		break;

	case DtButton3:
		control = XINPUT_GAMEPAD_X;
		break;

	case DtButton4:
		control = XINPUT_GAMEPAD_Y;
		break;

	default:
		return false;
	}
	return true;
}

void InputDeviceXi::resetState()
{
}

void InputDeviceXi::readState()
{
	m_connected = bool(XInputGetState(m_controller, &m_state) == ERROR_SUCCESS);
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

	}
}
