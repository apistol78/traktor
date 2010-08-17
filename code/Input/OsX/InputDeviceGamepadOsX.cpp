#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Input/OsX/InputDeviceGamepadOsX.h"

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

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.InputDeviceGamepadOsX", InputDeviceGamepadOsX, IInputDevice)

InputDeviceGamepadOsX::InputDeviceGamepadOsX(IOHIDDeviceRef deviceRef)
:	m_deviceRef(deviceRef)
{
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

int InputDeviceGamepadOsX::getControlCount()
{
	return 0;
}

std::wstring InputDeviceGamepadOsX::getControlName(int control)
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
		return L"Left trigger";
	case -6:
		return L"Right trigger";
	case -7:
		return L"Left trigger";
	case -8:
		return L"Right trigger";
	}
	return L"Button " + toString(control);
}

bool InputDeviceGamepadOsX::isControlAnalogue(int control) const
{
	return control < 0;
}

int32_t InputDeviceGamepadOsX::getActiveControlCount() const
{
	return 0;
}

float InputDeviceGamepadOsX::getControlValue(int control)
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
		return m_axis[2][0];
	else if (control == -6)
		return m_axis[2][1];
	else if (control == -7)
		return m_axis[2][0] > 0.5f ? 1.0f : 0.0f;
	else if (control == -8)
		return m_axis[2][1] > 0.5f ? 1.0f : 0.0f;
	else if (control >= 0 && control < sizeof_array(m_button))
		return m_button[control] ? 1.0f : 0.0f;
	else
		return 0.0f;
}

bool InputDeviceGamepadOsX::getDefaultControl(InputDefaultControlType controlType, bool analogue, int& control) const
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
		
		case DtTriggerLeft:
			control = -5;
			break;
		
		case DtTriggerRight:
			control = -6;
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
		
		case DtThumbLeftPush:
			control = 6;
			break;
		
		case DtThumbRightPush:
			control = 7;
			break;
		
		case DtSelect:
			control = 8;
			break;
		
		case DtCancel:
			control = 9;
			break;
		
		case DtUp:
			control = 10;
			break;
		
		case DtDown:
			control = 11;
			break;
		
		case DtLeft:
			control = 12;
			break;
		
		case DtRight:
			control = 13;
			break;
		
		case DtTriggerLeft:
			control = -7;
			break;
		
		case DtTriggerRight:
			control = -8;
			break;
		
		default:
			return false;
		}
	}

	return true;
}

void InputDeviceGamepadOsX::resetState()
{
	std::memset(m_button, 0, sizeof(m_button));
	std::memset(m_axis, 0, sizeof(m_axis));
}

void InputDeviceGamepadOsX::readState()
{
	if (!m_deviceRef)
		return;

	CFArrayRef elements = IOHIDDeviceCopyMatchingElements(m_deviceRef, NULL, kIOHIDOptionsTypeNone);
	for (CFIndex i = 0; i < CFArrayGetCount(elements); ++i)
	{
		IOHIDElementRef e = (IOHIDElementRef)CFArrayGetValueAtIndex(elements, i);
		if (!e)
			continue;

		uint32_t usage = (uint32_t)IOHIDElementGetUsage(e);
		if (usage == ~0UL)
			continue;
		
		IOHIDValueRef valueRef = 0;
		IOHIDDeviceGetValue(m_deviceRef, e, &valueRef);
		if (!valueRef)
			continue;

		int32_t v = (int32_t)IOHIDValueGetIntegerValue(valueRef);
		
		if (usage == 1)
			m_button[0] = bool(v != 0);		// A
		else if (usage == 2)
			m_button[1] = bool(v != 0);		// B
		else if (usage == 3)
			m_button[2] = bool(v != 0);		// X
		else if (usage == 4)
			m_button[3] = bool(v != 0);		// Y
		else if (usage == 5)
			m_button[4] = bool(v != 0);		// Left shoulder
		else if (usage == 6)
			m_button[5] = bool(v != 0);		// Right shoulder
		else if (usage == 7)
			m_button[6] = bool(v != 0);		// Left thumb down
		else if (usage == 8)
			m_button[7] = bool(v != 0);		// Right thumb down
		else if (usage == 9)
			m_button[8] = bool(v != 0);		// Start
		else if (usage == 10)
			m_button[9] = bool(v != 0);		// Back
		else if (usage == 12)
			m_button[10] = bool(v != 0);	// Dpad up
		else if (usage == 13)
			m_button[11] = bool(v != 0);	// Dpad down
		else if (usage == 14)
			m_button[12] = bool(v != 0);	// Dpad left
		else if (usage == 15)
			m_button[13] = bool(v != 0);	// Dpad right
		else if (usage == 48)
			m_axis[0][0] = adjustDeadZone(v / 32767.0f);	// Left Thumb X
		else if (usage == 49)
			m_axis[0][1] = adjustDeadZone(-v / 32767.0f);	// Left Thumb Y
		else if (usage == 51)
			m_axis[1][0] = adjustDeadZone(v / 32767.0f);	// Right Thumb X
		else if (usage == 52)
			m_axis[1][1] = adjustDeadZone(-v / 32767.0f);	// Right Thumb Y
		else if (usage == 50)				// Left trigger
			m_axis[2][0] = v / 255.0f;
		else if (usage == 53)				// Right trigger
			m_axis[2][1] = v / 255.0f;
	}
}

bool InputDeviceGamepadOsX::supportRumble() const
{
	return false;
}

void InputDeviceGamepadOsX::setRumble(const InputRumble& rumble)
{
}

	}
}
