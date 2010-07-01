#include "Core/Log/Log.h"
#include "Input/OsX/InputDeviceKeyboardOsX.h"

namespace traktor
{
	namespace input
	{
		namespace
		{

struct KeyControlMap
{
	InputDefaultControlType control;
	int32_t index;
}
c_keyControlMap[] =
{
	{ DtKeyUp, 98 },
	{ DtKeyDown, 97 },
	{ DtKeyLeft, 96 },
	{ DtKeyRight, 95 },
	{ DtKeyA, 20 },
	{ DtKeyS, 38 },
	{ DtKeyW, 42 },
	{ DtKeyD, 23 },
	{ DtKeySpace, 60 },
	{ DtKeyEscape, 57 },
	{ DtKeyLeftShift, 3 },
	{ DtKeyLeftControl, 2 },
	{ DtKeyLeftMenu, 4 },
	{ DtKeyLeftWin, 5 },
	{ DtKeyRightShift, 7 },
	{ DtKeyRightMenu, 8 },
	{ DtKeyRightWin, 9 },
	{ DtKeyReturn, 56 }
};
		
		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.InputDeviceKeyboardOsX", InputDeviceKeyboardOsX, IInputDevice)

InputDeviceKeyboardOsX::InputDeviceKeyboardOsX(IOHIDDeviceRef deviceRef)
:	m_deviceRef(deviceRef)
{
	resetState();
}

std::wstring InputDeviceKeyboardOsX::getName() const
{
	return L"HID Keyboard";
}

InputCategory InputDeviceKeyboardOsX::getCategory() const
{
	return CtKeyboard;
}

bool InputDeviceKeyboardOsX::isConnected() const
{
	return m_deviceRef != 0;
}

int InputDeviceKeyboardOsX::getControlCount()
{
	return sizeof_array(c_keyControlMap);
}

std::wstring InputDeviceKeyboardOsX::getControlName(int control)
{
	return L"";
}

bool InputDeviceKeyboardOsX::isControlAnalogue(int control) const
{
	return false;
}

float InputDeviceKeyboardOsX::getControlValue(int control)
{
	if (m_data[control])
		return 1.0f;
	else
		return 0.0f;
}

bool InputDeviceKeyboardOsX::getDefaultControl(InputDefaultControlType controlType, int& control) const
{
	for (int i = 0; i < sizeof_array(c_keyControlMap); ++i)
	{
		if (c_keyControlMap[i].control == controlType)
		{
			control = c_keyControlMap[i].index;
			return true;
		}
	}
	return false;
}

void InputDeviceKeyboardOsX::resetState()
{
	std::memset(m_data, 0, sizeof(m_data));
}

void InputDeviceKeyboardOsX::readState()
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

		if (cookie < sizeof_array(m_data))
			m_data[cookie] = v ? 255 : 0;
			
//		if (m_data[cookie])
//			log::info << L"DOWN " << cookie << Endl;
	}
}

bool InputDeviceKeyboardOsX::supportRumble() const
{
	return false;
}

void InputDeviceKeyboardOsX::setRumble(const InputRumble& rumble)
{
}

	}
}
