#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Input/OsX/InputDeviceGamepadOsX.h"

namespace traktor
{
	namespace input
	{
		namespace
		{
		
struct GamepadControlMap
{
	InputDefaultControlType control;
	int32_t index;
	bool analogue;
	const wchar_t* name;
}
c_gamepadControlMap[] =
{
	{ DtThumbLeftX, -1, true, L"Left thumb X" },
	{ DtThumbLeftY, -2, true, L"Left thumb Y" },
	{ DtThumbRightX, -3, true, L"Right thumb X" },
	{ DtThumbRightY, -4, true, L"Right thumb Y" },
	{ DtTriggerLeft, -5, true, L"Left trigger" },
	{ DtTriggerRight, -6, true, L"Right trigger" },
	{ DtShoulderLeft, -7, true, L"Left shoulder" },
	{ DtShoulderRight, -8, true, L"Right shoulder" },
	{ DtButton1, 0, false, L"Button 1" },
	{ DtButton2, 1, false, L"Button 2" },
	{ DtButton3, 2, false, L"Button 3" },
	{ DtButton4, 3, false, L"Button 4" },
	{ DtShoulderLeft, 4, false, L"Left shoulder" },
	{ DtShoulderRight, 5, false, L"Right shoulder" },
	{ DtThumbLeftPush, 6, false, L"Left thumb push" },
	{ DtThumbRightPush, 7, false, L"Right thumb push" },
	{ DtSelect, 8, false, L"Select" },
	{ DtCancel, 9, false, L"Cancel" },
	{ DtUp, 10, false, L"Up" },
	{ DtDown, 11, false, L"Down" },
	{ DtLeft, 12, false, L"Left" },
	{ DtRight, 13, false, L"Right" },
	{ DtTriggerLeft, -5, false, L"Left trigger" },
	{ DtTriggerRight, -6, false, L"Right trigger" }
};
		
float adjustDeadZone(float value)
{
	if (value >= -0.2f && value <= 0.2f)
		value = 0.0f;
	return value;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.InputDeviceGamepadOsX", InputDeviceGamepadOsX, IInputDevice)

InputDeviceGamepadOsX::InputDeviceGamepadOsX(IOHIDDeviceRef deviceRef)
:	m_deviceRef(deviceRef)
{
	if (m_deviceRef)
	{
		IOHIDDeviceRegisterRemovalCallback(m_deviceRef, &callbackRemoval, this);
		IOHIDDeviceRegisterInputValueCallback(m_deviceRef, &callbackValue, this);
	}

	resetState();
}

std::wstring InputDeviceGamepadOsX::getName() const
{
	return L"HID Gamepad";
}

InputCategory InputDeviceGamepadOsX::getCategory() const
{
	return CtJoystick;
}

bool InputDeviceGamepadOsX::isConnected() const
{
	return m_deviceRef != 0;
}

int32_t InputDeviceGamepadOsX::getControlCount()
{
	return sizeof_array(c_gamepadControlMap);
}

std::wstring InputDeviceGamepadOsX::getControlName(int32_t control)
{
	return c_gamepadControlMap[control].name;
}

bool InputDeviceGamepadOsX::isControlAnalogue(int32_t control) const
{
	return c_gamepadControlMap[control].analogue;
}

bool InputDeviceGamepadOsX::isControlStable(int32_t control) const
{
	return true;
}

float InputDeviceGamepadOsX::getControlValue(int32_t control)
{
	int32_t index = c_gamepadControlMap[control].index;
	bool analogue = c_gamepadControlMap[control].analogue;
	
	if (analogue)
	{
		if (index == -1)
			return m_axis[0][0];
		else if (index == -2)
			return m_axis[0][1];
		else if (index == -3)
			return m_axis[1][0];
		else if (index == -4)
			return m_axis[1][1];
		else if (index == -5)
			return m_axis[2][0];
		else if (index == -6)
			return m_axis[2][1];
		else if (index == -7)
			return m_button[4] ? 1.0f : 0.0f;
		else if (index == -8)
			return m_button[5] ? 1.0f : 0.0f;
		else
			return 0.0f;
	}
	else
	{
		float v = 0.0f;
		
		if (index == -5)
			v = m_axis[2][0];
		else if (index == -6)
			v = m_axis[2][1];
		else if (index >= 0 && index < sizeof_array(m_button))
			v = m_button[index] ? 1.0f : 0.0f;
	
		return v > 0.5f ? 1.0f : 0.0f;
	}
}

bool InputDeviceGamepadOsX::getControlRange(int32_t control, float& outMin, float& outMax) const
{
	return false;
}

bool InputDeviceGamepadOsX::getDefaultControl(InputDefaultControlType controlType, bool analogue, int32_t& control) const
{
	for (int32_t i = 0; i < sizeof_array(c_gamepadControlMap); ++i)
	{
		const GamepadControlMap& gc = c_gamepadControlMap[i];
		if (gc.control == controlType && gc.analogue == analogue)
		{
			control = i;
			return true;
		}
	}
	return false;
}

bool InputDeviceGamepadOsX::getKeyEvent(KeyEvent& outEvent)
{
	return false;
}

void InputDeviceGamepadOsX::resetState()
{
	std::memset(m_button, 0, sizeof(m_button));
	std::memset(m_axis, 0, sizeof(m_axis));
}

void InputDeviceGamepadOsX::readState()
{
}

bool InputDeviceGamepadOsX::supportRumble() const
{
	return false;
}

void InputDeviceGamepadOsX::setRumble(const InputRumble& rumble)
{
}

void InputDeviceGamepadOsX::setExclusive(bool exclusive)
{
}

void InputDeviceGamepadOsX::callbackRemoval(void* context, IOReturn result, void* sender)
{
	InputDeviceGamepadOsX* this_ = static_cast< InputDeviceGamepadOsX* >(context);
	this_->m_deviceRef = 0;
}

void InputDeviceGamepadOsX::callbackValue(void* context, IOReturn result, void* sender, IOHIDValueRef value)
{
	InputDeviceGamepadOsX* this_ = static_cast< InputDeviceGamepadOsX* >(context);
	
	IOHIDElementRef element = IOHIDValueGetElement(value);
	if (!element)
		return;
		
	uint32_t usage = (uint32_t)IOHIDElementGetUsage(element);
	if (usage == uint32_t(~0UL))
		return;
		
	int32_t v = (int32_t)IOHIDValueGetIntegerValue(value);
		
	if (usage == 1)
		this_->m_button[0] = bool(v != 0);		// A
	else if (usage == 2)
		this_->m_button[1] = bool(v != 0);		// B
	else if (usage == 3)
		this_->m_button[2] = bool(v != 0);		// X
	else if (usage == 4)
		this_->m_button[3] = bool(v != 0);		// Y
	else if (usage == 5)
		this_->m_button[4] = bool(v != 0);		// Left shoulder
	else if (usage == 6)
		this_->m_button[5] = bool(v != 0);		// Right shoulder
	else if (usage == 7)
		this_->m_button[6] = bool(v != 0);		// Left thumb down
	else if (usage == 8)
		this_->m_button[7] = bool(v != 0);		// Right thumb down
	else if (usage == 9)
		this_->m_button[8] = bool(v != 0);		// Start
	else if (usage == 10)
		this_->m_button[9] = bool(v != 0);		// Back
	else if (usage == 12)
		this_->m_button[10] = bool(v != 0);	// Dpad up
	else if (usage == 13)
		this_->m_button[11] = bool(v != 0);	// Dpad down
	else if (usage == 14)
		this_->m_button[12] = bool(v != 0);	// Dpad left
	else if (usage == 15)
		this_->m_button[13] = bool(v != 0);	// Dpad right
	else if (usage >= 48 && usage <= 53)
	{
		int32_t min = IOHIDElementGetLogicalMin(element);
		int32_t max = IOHIDElementGetLogicalMax(element);
		
		if (max > min)
		{
			if (usage == 50 || usage == 53)
			{
				float fv = float(v - min) / (max - min);
				if (usage == 50)				// Left trigger
					this_->m_axis[2][0] = fv;
				else if (usage == 53)			// Right trigger
					this_->m_axis[2][1] = fv;
			}
			else
			{
				float fv = 2.0f * float(v - min) / (max - min) - 1.0f;
				if (usage == 48)
					this_->m_axis[0][0] = adjustDeadZone(fv);	// Left Thumb X
				else if (usage == 49)
					this_->m_axis[0][1] = adjustDeadZone(-fv);	// Left Thumb Y
				else if (usage == 51)
					this_->m_axis[1][0] = adjustDeadZone(fv);	// Right Thumb X
				else if (usage == 52)
					this_->m_axis[1][1] = adjustDeadZone(-fv);	// Right Thumb Y
			}
		}
	}
}

	}
}
