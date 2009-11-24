#include "Input/Ps3/InputDevicePs3.h"
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

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.InputDevicePs3", InputDevicePs3, IInputDevice)

InputDevicePs3::InputDevicePs3(int padIndex)
:	m_enabled(false)
,	m_padIndex(padIndex)
{
	std::memset(&m_padData, 0, sizeof(m_padData));
}

std::wstring InputDevicePs3::getName() const
{
	return L"Pad " + toString(m_padIndex);
}

InputCategory InputDevicePs3::getCategory() const
{
	return CtJoystick;
}

bool InputDevicePs3::isConnected() const
{
	return true;
}

int InputDevicePs3::getControlCount()
{
	return 0;
}

std::wstring InputDevicePs3::getControlName(int control)
{
	return L"";
}

bool InputDevicePs3::isControlAnalogue(int control) const
{
	return false;
}

float InputDevicePs3::getControlValue(int control)
{
	if (!m_enabled)
		return 0.0f;

	if (control > 0)
		return m_padData.button[control >> 4] & (1 << (control & 15)) ? 1.0f : 0.0f;
	else if (control < 0)
	{
		float thumb = 0.0f;
		switch (control)
		{
		case -1:
			thumb = m_padData.button[6] / 128.0f - 1.0f;
			break;
			
		case -2:
			thumb = -(m_padData.button[7] / 128.0f - 1.0f);
			break;

		case -3:
			thumb = m_padData.button[4] / 128.0f - 1.0f;
			break;

		case -4:
			thumb = -(m_padData.button[5] / 128.0f - 1.0f);
			break;
		}
		return adjustDeadZone(thumb);
	}

	return 0.0f;
}

bool InputDevicePs3::getDefaultControl(InputDefaultControlType controlType, int& control) const
{
	switch (controlType)
	{
	//case DtUp:
	//	control = XINPUT_GAMEPAD_DPAD_UP;
	//	break;

	//case DtDown:
	//	control = XINPUT_GAMEPAD_DPAD_DOWN;
	//	break;

	//case DtLeft:
	//	control = XINPUT_GAMEPAD_DPAD_LEFT;
	//	break;

	//case DtRight:
	//	control = XINPUT_GAMEPAD_DPAD_RIGHT;
	//	break;

	case DtSelect:
		control = 4 * 16 + 0;
		break;

	//case DtCancel:
	//	control = XINPUT_GAMEPAD_BACK;
	//	break;

	case DtThumbLeftX:
		control = -1;
		break;

	case DtThumbLeftY:
		control = -2;
		break;

	case DtThumbRightX:
		control = -3;
		break;

	case DtThumbRightY:
		control = -4;
		break;

	default:
		return false;
	}
	return true;
}

void InputDevicePs3::resetState()
{
}

void InputDevicePs3::readState()
{
	m_enabled = bool(cellPadGetData(m_padIndex, &m_padData) == CELL_PAD_OK);
}

bool InputDevicePs3::supportRumble() const
{
	return false;
}

void InputDevicePs3::setRumble(const InputRumble& rumble)
{
}

	}
}
