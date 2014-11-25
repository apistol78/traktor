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
	if (value >= -0.3f && value <= 0.3f)
		value = 0.0f;
	return value;
}

const float c_highFrequencyThreshold = 30.0f;
const float c_lowFrequencyOffset = 80.0f;

static CellPadInfo2 s_info;

struct ControlInfo
{
	const wchar_t* name;
	InputDefaultControlType type;
	bool analogue;
	int32_t control;
	float rangeMin;
	float rangeMax;
}
c_controlInfo[] =
{
	{ L"INPUT_GAMEPAD_UP", DtUp, false, T_CONTROL_ID(CELL_PAD_BTN_OFFSET_DIGITAL1, CELL_PAD_CTRL_UP), 0.0f, 1.0f },
	{ L"INPUT_GAMEPAD_DOWN", DtDown, false, T_CONTROL_ID(CELL_PAD_BTN_OFFSET_DIGITAL1, CELL_PAD_CTRL_DOWN), 0.0f, 1.0f },
	{ L"INPUT_GAMEPAD_LEFT", DtLeft, false, T_CONTROL_ID(CELL_PAD_BTN_OFFSET_DIGITAL1, CELL_PAD_CTRL_LEFT), 0.0f, 1.0f },
	{ L"INPUT_GAMEPAD_RIGHT", DtRight, false, T_CONTROL_ID(CELL_PAD_BTN_OFFSET_DIGITAL1, CELL_PAD_CTRL_RIGHT), 0.0f, 1.0f },
	{ L"INPUT_GAMEPAD_SELECT", DtSelect, false, T_CONTROL_ID(CELL_PAD_BTN_OFFSET_DIGITAL1, CELL_PAD_CTRL_SELECT), 0.0f, 1.0f },
	{ L"INPUT_GAMEPAD_CANCEL", DtCancel, false, T_CONTROL_ID(CELL_PAD_BTN_OFFSET_DIGITAL1, CELL_PAD_CTRL_START), 0.0f, 1.0f },
	{ L"INPUT_GAMEPAD_BUTTON_1", DtButton1, false, T_CONTROL_ID(CELL_PAD_BTN_OFFSET_DIGITAL2, CELL_PAD_CTRL_CROSS), 0.0f, 1.0f },
	{ L"INPUT_GAMEPAD_BUTTON_2", DtButton2, false, T_CONTROL_ID(CELL_PAD_BTN_OFFSET_DIGITAL2, CELL_PAD_CTRL_CIRCLE), 0.0f, 1.0f },
	{ L"INPUT_GAMEPAD_BUTTON_3", DtButton3, false, T_CONTROL_ID(CELL_PAD_BTN_OFFSET_DIGITAL2, CELL_PAD_CTRL_TRIANGLE), 0.0f, 1.0f },
	{ L"INPUT_GAMEPAD_BUTTON_4", DtButton4, false, T_CONTROL_ID(CELL_PAD_BTN_OFFSET_DIGITAL2, CELL_PAD_CTRL_SQUARE), 0.0f, 1.0f },
	{ L"INPUT_GAMEPAD_TRIGGER_LEFT", DtTriggerLeft, false, T_CONTROL_ID(CELL_PAD_BTN_OFFSET_DIGITAL2, CELL_PAD_CTRL_L2), 0.0f, 1.0f },
	{ L"INPUT_GAMEPAD_TRIGGER_RIGHT", DtTriggerRight, false, T_CONTROL_ID(CELL_PAD_BTN_OFFSET_DIGITAL2, CELL_PAD_CTRL_R2), 0.0f, 1.0f },
	{ L"INPUT_GAMEPAD_SHOULDER_LEFT", DtShoulderLeft, false, T_CONTROL_ID(CELL_PAD_BTN_OFFSET_DIGITAL2, CELL_PAD_CTRL_L1), 0.0f, 1.0f },
	{ L"INPUT_GAMEPAD_SHOULDER_RIGHT", DtShoulderRight, false, T_CONTROL_ID(CELL_PAD_BTN_OFFSET_DIGITAL2, CELL_PAD_CTRL_R1), 0.0f, 1.0f },
	{ L"INPUT_GAMEPAD_THUMB_LEFT_X", DtThumbLeftX, true, 0, -1.0f, 1.0f },
	{ L"INPUT_GAMEPAD_THUMB_LEFT_Y", DtThumbLeftY, true, 1, -1.0f, 1.0f },
	{ L"INPUT_GAMEPAD_THUMB_RIGHT_X", DtThumbRightX, true, 2, -1.0f, 1.0f },
	{ L"INPUT_GAMEPAD_THUMB_RIGHT_Y", DtThumbRightY, true, 3, -1.0f, 1.0f }
};

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
	return L"INPUT_GAMEPAD_" + toString(m_padIndex);
}

InputCategory InputDevicePs3::getCategory() const
{
	return CtJoystick;
}

bool InputDevicePs3::isConnected() const
{
	return m_connected;
}

int32_t InputDevicePs3::getControlCount()
{
	return sizeof_array(c_controlInfo);
}

std::wstring InputDevicePs3::getControlName(int32_t control)
{
	const ControlInfo& controlInfo = c_controlInfo[control];
	return controlInfo.name;
}

bool InputDevicePs3::isControlAnalogue(int32_t control) const
{
	const ControlInfo& controlInfo = c_controlInfo[control];
	return controlInfo.analogue;
}

bool InputDevicePs3::isControlStable(int32_t control) const
{
	return true;
}

float InputDevicePs3::getControlValue(int32_t control)
{
	if (!m_connected)
		return 0.0f;

	const ControlInfo& controlInfo = c_controlInfo[control];
	if (!controlInfo.analogue)
	{
		int offset = T_CONTROL_OFFSET(controlInfo.control);
		int bit = T_CONTROL_BIT(controlInfo.control);
		return (m_padData.button[offset] & bit) ? 1.0f : 0.0f;
	}
	else
	{
		float thumb = 0.0f;
		switch (controlInfo.control)
		{
		case 0:
			thumb = m_padData.button[6] / 128.0f - 1.0f;
			break;
			
		case 1:
			thumb = -(m_padData.button[7] / 128.0f - 1.0f);
			break;

		case 2:
			thumb = m_padData.button[4] / 128.0f - 1.0f;
			break;

		case 3:
			thumb = -(m_padData.button[5] / 128.0f - 1.0f);
			break;
		}
		return adjustDeadZone(thumb);
	}

	return 0.0f;
}

bool InputDevicePs3::getControlRange(int32_t control, float& outMin, float& outMax) const
{
	const ControlInfo& controlInfo = c_controlInfo[control];
	outMin = controlInfo.rangeMin;
	outMax = controlInfo.rangeMax;
	return true;
}

bool InputDevicePs3::getDefaultControl(InputDefaultControlType controlType, bool analogue, int& control) const
{
	for (int i = 0; i < sizeof_array(c_controlInfo); ++i)
	{
		if (c_controlInfo[i].type == controlType && c_controlInfo[i].analogue == analogue)
		{
			control = i;
			return true;
		}
	}
	return false;
}

bool InputDevicePs3::getKeyEvent(KeyEvent& outEvent)
{
	return false;
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
			if (s_info.device_capability[m_padIndex] & CELL_PAD_CAPABILITY_PS3_CONFORMITY)
				log::debug << L"   + PS3_CONFORMITY" << Endl;
			if (s_info.device_capability[m_padIndex] & CELL_PAD_CAPABILITY_PRESS_MODE)
				log::debug << L"   + PRESS_MODE" << Endl;
			if (s_info.device_capability[m_padIndex] & CELL_PAD_CAPABILITY_SENSOR_MODE)
				log::debug << L"   + SENSOR_MODE" << Endl;
			if (s_info.device_capability[m_padIndex] & CELL_PAD_CAPABILITY_HP_ANALOG_STICK)
				log::debug << L"   + HP_ANALOG_STICK" << Endl;
			if (s_info.device_capability[m_padIndex] & CELL_PAD_CAPABILITY_ACTUATOR)
				log::debug << L"   + ACTUATOR" << Endl;

			log::debug << L"   Port Setting: " << s_info.port_setting[m_padIndex] << Endl;
			if (s_info.port_setting[m_padIndex] & CELL_PAD_SETTING_PRESS_ON)
				log::debug << L"   + PRESS_MODE ON" << Endl;
			if (s_info.port_setting[m_padIndex] & CELL_PAD_SETTING_SENSOR_ON)
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
	if (s_info.device_capability[m_padIndex] & CELL_PAD_CAPABILITY_ACTUATOR)
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
	param.motor[0] = (rumble.highFrequencyRumble >= c_highFrequencyThreshold) ? 1 : 0;
	if (rumble.lowFrequencyRumble > FUZZY_EPSILON)
		param.motor[1] = (uint8_t)clamp((rumble.lowFrequencyRumble * (255 - c_lowFrequencyOffset) + c_lowFrequencyOffset) / 100.0f, c_lowFrequencyOffset, 255.0f);
	else
		param.motor[1] = 0;

	if (m_padActuator.motor[0] != param.motor[0] || m_padActuator.motor[1] != param.motor[1])
	{
		cellPadSetActDirect(m_padIndex, &param);
		std::memcpy(&m_padActuator, &param, sizeof(CellPadActParam));
	}
}

void InputDevicePs3::setExclusive(bool exclusive)
{
}

	}
}
