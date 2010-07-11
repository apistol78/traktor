#include "Core/Log/Log.h"
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
	return L"";
}

bool InputDeviceGamepadOsX::isControlAnalogue(int control) const
{
	return false;
}

float InputDeviceGamepadOsX::getControlValue(int control)
{
	if (control >= 0 && control <= 7)
		return m_button[control] ? 1.0f : 0.0f;
		
	if (control == -1)
		return m_axis[0][0];
	if (control == -2)
		return m_axis[0][1];
	if (control == -3)
		return m_axis[1][0];
	if (control == -4)
		return m_axis[1][1];

	return 0.0f;
}

bool InputDeviceGamepadOsX::getDefaultControl(InputDefaultControlType controlType, int& control) const
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
		
	case DtUp:
		control = 4;
		break;
		
	case DtDown:
		control = 5;
		break;
		
	case DtLeft:
		control = 6;
		break;
		
	case DtRight:
		control = 7;
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

void InputDeviceGamepadOsX::resetState()
{
	std::memset(m_button, 0, sizeof(m_button));
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

		int cookie = (int)IOHIDElementGetCookie(e);
		
		IOHIDValueRef valueRef = 0;
		IOHIDDeviceGetValue(m_deviceRef, e, &valueRef);
		if (!valueRef)
			continue;
			
		int v = (int)IOHIDValueGetIntegerValue(valueRef);
		
		if (cookie == 21)
			m_button[0] = bool(v != 0);		// A
		if (cookie == 22)
			m_button[1] = bool(v != 0);		// B
		if (cookie == 23)
			m_button[2] = bool(v != 0);		// X
		if (cookie == 24)
			m_button[3] = bool(v != 0);		// Y

		if (cookie == 10)
			m_button[4] = bool(v != 0);		// Dpad up
		if (cookie == 11)
			m_button[5] = bool(v != 0);		// Dpad down
		if (cookie == 12)
			m_button[6] = bool(v != 0);		// Dpad left
		if (cookie == 13)
			m_button[7] = bool(v != 0);		// Dpad right
			
		if (cookie == 27)
			m_axis[0][0] = adjustDeadZone(v / 32767.0f);	// Left Thumb X
		if (cookie == 28)
			m_axis[0][1] = adjustDeadZone(-v / 32767.0f);	// Left Thumb Y
		if (cookie == 29)
			m_axis[1][0] = adjustDeadZone(v / 32767.0f);	// Right Thumb X
		if (cookie == 30)
			m_axis[1][1] = adjustDeadZone(-v / 32767.0f);	// Right Thumb Y
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
