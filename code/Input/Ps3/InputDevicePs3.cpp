#include "Input/Ps3/InputDevicePs3.h"
#include "Core/Misc/String.h"

namespace traktor
{
	namespace input
	{
		namespace
		{

#define T_CONTROL_ID(offset, bit) \
	((offset) << 8) | (bit)

#define T_CONTROL_OFFSET(id) \
	((id) >> 8)

#define T_CONTROL_BIT(id) \
	((id) & 255)

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
	{
		int offset = T_CONTROL_OFFSET(control);
		int bit = T_CONTROL_BIT(control);
		return (m_padData.button[offset] & bit) ? 1.0f : 0.0f;
	}
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
	case DtUp:
		control = T_CONTROL_ID(CELL_PAD_BTN_OFFSET_DIGITAL1, CELL_PAD_CTRL_UP);
		break;

	case DtDown:
		control = T_CONTROL_ID(CELL_PAD_BTN_OFFSET_DIGITAL1, CELL_PAD_CTRL_DOWN);
		break;

	case DtLeft:
		control = T_CONTROL_ID(CELL_PAD_BTN_OFFSET_DIGITAL1, CELL_PAD_CTRL_LEFT);
		break;

	case DtRight:
		control = T_CONTROL_ID(CELL_PAD_BTN_OFFSET_DIGITAL1, CELL_PAD_CTRL_RIGHT);
		break;

	case DtSelect:
		control = T_CONTROL_ID(CELL_PAD_BTN_OFFSET_DIGITAL1, CELL_PAD_CTRL_SELECT);
		break;

	case DtCancel:
		control = T_CONTROL_ID(CELL_PAD_BTN_OFFSET_DIGITAL1, CELL_PAD_CTRL_START);
		break;

	case DtButton1:
		control = T_CONTROL_ID(CELL_PAD_BTN_OFFSET_DIGITAL2, CELL_PAD_CTRL_CROSS);
		break;

	case DtButton2:
		control = T_CONTROL_ID(CELL_PAD_BTN_OFFSET_DIGITAL2, CELL_PAD_CTRL_CIRCLE);
		break;

	case DtButton3:
		control = T_CONTROL_ID(CELL_PAD_BTN_OFFSET_DIGITAL2, CELL_PAD_CTRL_TRIANGLE);
		break;

	case DtButton4:
		control = T_CONTROL_ID(CELL_PAD_BTN_OFFSET_DIGITAL2, CELL_PAD_CTRL_SQUARE);
		break;

	case DtTriggerLeft:
		control = T_CONTROL_ID(CELL_PAD_BTN_OFFSET_DIGITAL2, CELL_PAD_CTRL_L2);
		break;

	case DtTriggerRight:
		control = T_CONTROL_ID(CELL_PAD_BTN_OFFSET_DIGITAL2, CELL_PAD_CTRL_R2);
		break;

	case DtShoulderLeft:
		control = T_CONTROL_ID(CELL_PAD_BTN_OFFSET_DIGITAL2, CELL_PAD_CTRL_L1);
		break;

	case DtShoulderRight:
		control = T_CONTROL_ID(CELL_PAD_BTN_OFFSET_DIGITAL2, CELL_PAD_CTRL_R1);
		break;

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
