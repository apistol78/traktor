#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Input/OsX/InputDeviceJoystickOsX.h"

namespace traktor
{
	namespace input
	{
		namespace
		{

struct JoystickControlMap
{
	InputDefaultControlType control;
	int32_t index;
	bool analogue;
	const wchar_t* name;
}
c_joystickControlMap[] =
{
	{ DtThumbLeftX, -1, true, L"Left thumb X" },
	{ DtThumbLeftY, -2, true, L"Left thumb Y" },
	{ DtThumbRightX, -3, true, L"Right thumb X" },
	{ DtThumbRightY, -4, true, L"Right thumb Y" },
	{ DtButton1, 0, false, L"Button 1" },
	{ DtButton2, 1, false, L"Button 2" },
	{ DtButton3, 2, false, L"Button 3" },
	{ DtButton4, 3, false, L"Button 4" },
	{ DtShoulderLeft, 4, false, L"Left shoulder" },
	{ DtShoulderRight, 5, false, L"Right shoulder" },
	{ DtTriggerLeft, 6, false, L"Left trigger" },
	{ DtTriggerRight, 7, false, L"Right trigger" },
	{ DtCancel, 8, false, L"Cancel" },
	{ DtSelect, 9, false, L"Select" },
	{ DtUp, -5, false, L"Up" },
	{ DtDown, -7, false, L"Down" },
	{ DtLeft, -8, false, L"Left" },
	{ DtRight, -6, false, L"Right" }
};
		
float adjustDeadZone(float value)
{
	if (value >= -0.2f && value <= 0.2f)
		value = 0.0f;
	return value;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.InputDeviceJoystickOsX", InputDeviceJoystickOsX, IInputDevice)

InputDeviceJoystickOsX::InputDeviceJoystickOsX(IOHIDDeviceRef deviceRef)
:	m_deviceRef(deviceRef)
{
	if (m_deviceRef)
	{
		IOHIDDeviceRegisterRemovalCallback(m_deviceRef, &callbackRemoval, this);
		IOHIDDeviceRegisterInputValueCallback(m_deviceRef, &callbackValue, this);
	}

	resetState();
}

std::wstring InputDeviceJoystickOsX::getName() const
{
	return L"HID Joystick";
}

InputCategory InputDeviceJoystickOsX::getCategory() const
{
	return CtJoystick;
}

bool InputDeviceJoystickOsX::isConnected() const
{
	return m_deviceRef != 0;
}

int32_t InputDeviceJoystickOsX::getControlCount()
{
	return sizeof_array(c_joystickControlMap);
}

std::wstring InputDeviceJoystickOsX::getControlName(int32_t control)
{
	return c_joystickControlMap[control].name;
}

bool InputDeviceJoystickOsX::isControlAnalogue(int32_t control) const
{
	return c_joystickControlMap[control].analogue;
}

bool InputDeviceJoystickOsX::isControlStable(int32_t control) const
{
	return true;
}

float InputDeviceJoystickOsX::getControlValue(int32_t control)
{
	const JoystickControlMap& jm = c_joystickControlMap[control];
	int32_t index = jm.index;

	if (index == -1)
		return m_axis[0][0];
	else if (index == -2)
		return m_axis[0][1];
	else if (index == -3)
		return m_axis[1][0];
	else if (index == -4)
		return m_axis[1][1];
	else if (index == -5)
		return (m_dpad == 7 || m_dpad == 0 || m_dpad == 1) ? 1.0f : 0.0f;
	else if (index == -6)
		return (m_dpad == 1 || m_dpad == 2 || m_dpad == 3) ? 1.0f : 0.0f;
	else if (index == -7)
		return (m_dpad == 3 || m_dpad == 4 || m_dpad == 5) ? 1.0f : 0.0f;
	else if (index == -8)
		return (m_dpad == 5 || m_dpad == 6 || m_dpad == 7) ? 1.0f : 0.0f;
	else if (index >= 0 && index < sizeof_array(m_button))
		return m_button[index] ? 1.0f : 0.0f;
	else
		return 0.0f;
}

bool InputDeviceJoystickOsX::getControlRange(int32_t control, float& outMin, float& outMax) const
{
	return false;
}

bool InputDeviceJoystickOsX::getDefaultControl(InputDefaultControlType controlType, bool analogue, int32_t& control) const
{
	for (int32_t i = 0; i < sizeof_array(c_joystickControlMap); ++i)
	{
		const JoystickControlMap& jc = c_joystickControlMap[i];
		if (jc.control == controlType && jc.analogue == analogue)
		{
			control = i;
			return true;
		}
	}
	return false;
}

bool InputDeviceJoystickOsX::getKeyEvent(KeyEvent& outEvent)
{
	return false;
}

void InputDeviceJoystickOsX::resetState()
{
	std::memset(m_button, 0, sizeof(m_button));
	std::memset(m_axis, 0, sizeof(m_axis));
	m_dpad = 8;
}

void InputDeviceJoystickOsX::readState()
{
}

bool InputDeviceJoystickOsX::supportRumble() const
{
	return false;
}

void InputDeviceJoystickOsX::setRumble(const InputRumble& rumble)
{
}

void InputDeviceJoystickOsX::setExclusive(bool exclusive)
{
}

void InputDeviceJoystickOsX::callbackRemoval(void* context, IOReturn result, void* sender)
{
	InputDeviceJoystickOsX* this_ = static_cast< InputDeviceJoystickOsX* >(context);
	this_->m_deviceRef = 0;
}

void InputDeviceJoystickOsX::callbackValue(void* context, IOReturn result, void* sender, IOHIDValueRef value)
{
	InputDeviceJoystickOsX* this_ = static_cast< InputDeviceJoystickOsX* >(context);
	
	IOHIDElementRef element = IOHIDValueGetElement(value);
	if (!element)
		return;
		
	uint32_t usage = (uint32_t)IOHIDElementGetUsage(element);
	if (usage == uint32_t(~0UL))
		return;
		
	int32_t v = (int32_t)IOHIDValueGetIntegerValue(value);
		
	if (usage == 1)
		this_->m_button[0] = bool(v != 0);
	else if (usage == 2)
		this_->m_button[1] = bool(v != 0);
	else if (usage == 3)
		this_->m_button[2] = bool(v != 0);
	else if (usage == 4)
		this_->m_button[3] = bool(v != 0);
	else if (usage == 5)
		this_->m_button[4] = bool(v != 0);
	else if (usage == 6)
		this_->m_button[5] = bool(v != 0);
	else if (usage == 7)
		this_->m_button[6] = bool(v != 0);
	else if (usage == 8)
		this_->m_button[7] = bool(v != 0);
	else if (usage == 9)
		this_->m_button[8] = bool(v != 0);
	else if (usage == 10)
		this_->m_button[9] = bool(v != 0);
	else if (usage == 12)
		this_->m_button[10] = bool(v != 0);
	else if (usage == 13)
		this_->m_button[11] = bool(v != 0);
	else if (usage == 14)
		this_->m_button[12] = bool(v != 0);
	else if (usage == 15)
		this_->m_button[13] = bool(v != 0);
	else if (usage >= 48 && usage <= 53)
	{
		int32_t min = IOHIDElementGetLogicalMin(element);
		int32_t max = IOHIDElementGetLogicalMax(element);
		
		if (max > min)
		{
			float fv = 2.0f * float(v - min) / (max - min) - 1.0f;
			if (usage == 48)
				this_->m_axis[0][0] = adjustDeadZone(fv);	// Left Thumb X
			else if (usage == 49)
				this_->m_axis[0][1] = adjustDeadZone(-fv);	// Left Thumb Y
			else if (usage == 50)
				this_->m_axis[1][0] = adjustDeadZone(fv);	// Right Thumb X
			else if (usage == 53)
				this_->m_axis[1][1] = adjustDeadZone(-fv);	// Right Thumb Y
		}
	}
	else if (usage == 57)
		this_->m_dpad = v;
}

	}
}
