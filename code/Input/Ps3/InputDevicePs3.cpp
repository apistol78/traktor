#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "Core/Misc/String.h"
#include "Input/Ps3/InputDevicePs3.h"

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

static CellPadInfo2 s_info;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.InputDevicePs3", InputDevicePs3, IInputDevice)

InputDevicePs3::InputDevicePs3(int padIndex)
:	m_padIndex(padIndex)
,	m_connected(false)
{
	std::memset(&m_padData, 0, sizeof(m_padData));
	std::memset(&m_padActuator, 0, sizeof(m_padActuator));
	readState();
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
	return m_connected;
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
	return control < 0;
}

float InputDevicePs3::getControlValue(int control)
{
	if (!m_connected)
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
	cellPadClearBuf(m_padIndex);
}

void InputDevicePs3::readState()
{
	m_connected = false;

	// Hack; only first pad read info; shared with all devices
	if (m_padIndex == 0)
	{
		if (cellPadGetInfo2(&s_info) != CELL_PAD_OK)
			return;
	}

	if ((s_info.port_status[m_padIndex] & CELL_PAD_STATUS_ASSIGN_CHANGES) != 0)
	{
		if ((s_info.port_status[m_padIndex] & CELL_PAD_STATUS_CONNECTED) != 0)
		{
			log::debug << L"Pad " << m_padIndex << L" connected" << Endl;

			log::debug << L"   Capability  :" << s_info.device_capability[m_padIndex] << Endl;
			if(s_info.device_capability[m_padIndex] & CELL_PAD_CAPABILITY_PS3_CONFORMITY)
				log::debug << L"   + PS3_CONFORMITY" << Endl;
			if(s_info.device_capability[m_padIndex] & CELL_PAD_CAPABILITY_PRESS_MODE)
				log::debug << L"   + PRESS_MODE" << Endl;
			if(s_info.device_capability[m_padIndex] & CELL_PAD_CAPABILITY_SENSOR_MODE)
				log::debug << L"   + SENSOR_MODE" << Endl;
			if(s_info.device_capability[m_padIndex] & CELL_PAD_CAPABILITY_HP_ANALOG_STICK)
				log::debug << L"   + HP_ANALOG_STICK" << Endl;
			if(s_info.device_capability[m_padIndex] & CELL_PAD_CAPABILITY_ACTUATOR)
				log::debug << L"   + ACTUATOR" << Endl;

			log::debug << L"   Port Setting: " << s_info.port_setting[m_padIndex] << Endl;
			if(s_info.port_setting[m_padIndex] & CELL_PAD_SETTING_PRESS_ON)
				log::debug << L"   + PRESS_MODE ON" << Endl;
			if(s_info.port_setting[m_padIndex] & CELL_PAD_SETTING_SENSOR_ON)
				log::debug << L"   + SENSOR_MODE ON" << Endl;

			log::debug << L"   Device Type : " << s_info.device_type[m_padIndex] << Endl;
		}
		else
			log::debug << L"Pad " << m_padIndex << L" disconnected" << Endl;
	}

	if ((s_info.port_status[m_padIndex] & CELL_PAD_STATUS_CONNECTED) != 0)
	{
		if (cellPadGetData(m_padIndex, &m_padData) == CELL_PAD_OK)
			m_connected = true;
	}
}

bool InputDevicePs3::supportRumble() const
{
	if(s_info.device_capability[m_padIndex] & CELL_PAD_CAPABILITY_ACTUATOR)
		return true;
	else
		return false;
}

void InputDevicePs3::setRumble(const InputRumble& rumble)
{
	if (!m_connected)
		return;

	CellPadActParam param;

	std::memset(&param, 0, sizeof(param));
	param.motor[0] = (rumble.highFrequencyRumble >= 50.0f) ? 1 : 0;
	if (rumble.lowFrequencyRumble > FUZZY_EPSILON)
		param.motor[1] = (uint8_t)clamp((rumble.lowFrequencyRumble * (255 - 64) + 64) / 100.0f, 64.0f, 255.0f);
	else
		param.motor[1] = 0;

	if (m_padActuator.motor[0] != param.motor[0] || m_padActuator.motor[1] != param.motor[1])
	{
		cellPadSetActDirect(m_padIndex, &param);
		std::memcpy(&m_padActuator, &param, sizeof(CellPadActParam));
	}
}

	}
}
