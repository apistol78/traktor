#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Input/OsX/InputDeviceJoystickOsX.h"

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
	return 0;
}

std::wstring InputDeviceJoystickOsX::getControlName(int32_t control)
{
	switch (control)
	{
	case -1:
		return L"Left thumb X";
	case -2:
		return L"Left thumb Y";
	case -3:
		return L"Right thumb X";
	case -4:
		return L"Right thumb Y";
	case -5:
		return L"Up";
	case -6:
		return L"Right";
	case -7:
		return L"Down";
	case -8:
		return L"Left";
	}
	return L"Button " + toString(control + 1);
}

bool InputDeviceJoystickOsX::isControlAnalogue(int32_t control) const
{
	return control < 0;
}

float InputDeviceJoystickOsX::getControlValue(int32_t control)
{
	if (control == -1)
		return m_axis[0][0];
	else if (control == -2)
		return m_axis[0][1];
	else if (control == -3)
		return m_axis[1][0];
	else if (control == -4)
		return m_axis[1][1];
	else if (control == -5)
		return (m_dpad == 7 || m_dpad == 0 || m_dpad == 1) ? 1.0f : 0.0f;
	else if (control == -6)
		return (m_dpad == 1 || m_dpad == 2 || m_dpad == 3) ? 1.0f : 0.0f;
	else if (control == -7)
		return (m_dpad == 3 || m_dpad == 4 || m_dpad == 5) ? 1.0f : 0.0f;
	else if (control == -8)
		return (m_dpad == 5 || m_dpad == 6 || m_dpad == 7) ? 1.0f : 0.0f;
	else if (control >= 0 && control < sizeof_array(m_button))
		return m_button[control] ? 1.0f : 0.0f;
	else
		return 0.0f;
}

bool InputDeviceJoystickOsX::getDefaultControl(InputDefaultControlType controlType, bool analogue, int32_t& control) const
{
	if (analogue)
	{
		switch (controlType)
		{
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
	}
	else
	{
		switch (controlType)
		{
		case DtButton1:
			control = 0;
			break;
		
		case DtButton2:
			control = 1;
			break;
		
		case DtButton3:
			control = 2;
			break;
		
		case DtButton4:
			control = 3;
			break;
		
		case DtShoulderLeft:
			control = 4;
			break;
		
		case DtShoulderRight:
			control = 5;
			break;
				
		case DtSelect:
			control = 9;
			break;
		
		case DtCancel:
			control = 8;
			break;
		
		case DtUp:
			control = -5;
			break;
		
		case DtDown:
			control = -7;
			break;
		
		case DtLeft:
			control = -8;
			break;
		
		case DtRight:
			control = -6;
			break;
		
		case DtTriggerLeft:
			control = 6;
			break;
		
		case DtTriggerRight:
			control = 7;
			break;
		
		default:
			return false;
		}
	}

	return true;
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
	if (usage == ~0UL)
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
		
	else if (usage == 48)
		this_->m_axis[0][0] = adjustDeadZone((v - 127.5f) / 127.5f);	// Left Thumb X
	else if (usage == 49)
		this_->m_axis[0][1] = adjustDeadZone(-(v - 127.5f) / 127.5f);	// Left Thumb Y
	else if (usage == 50)
		this_->m_axis[1][0] = adjustDeadZone((v - 127.5f) / 127.5f);	// Right Thumb X
	else if (usage == 53)
		this_->m_axis[1][1] = adjustDeadZone(-(v - 127.5f) / 127.5f);	// Right Thumb Y
		
	else if (usage == 57)
		this_->m_dpad = v;
}

	}
}
