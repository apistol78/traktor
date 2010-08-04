#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
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
	{ DtKeyUp, kHIDUsage_KeyboardUpArrow },
	{ DtKeyDown, kHIDUsage_KeyboardDownArrow },
	{ DtKeyLeft, kHIDUsage_KeyboardLeftArrow },
	{ DtKeyRight, kHIDUsage_KeyboardRightArrow },
	{ DtKeyA, kHIDUsage_KeyboardA },
	{ DtKeyB, kHIDUsage_KeyboardB },
	{ DtKeyC, kHIDUsage_KeyboardC },
	{ DtKeyD, kHIDUsage_KeyboardD },
	{ DtKeyE, kHIDUsage_KeyboardE },
	{ DtKeyF, kHIDUsage_KeyboardF },
	{ DtKeyG, kHIDUsage_KeyboardG },
	{ DtKeyH, kHIDUsage_KeyboardH },
	{ DtKeyI, kHIDUsage_KeyboardI },
	{ DtKeyJ, kHIDUsage_KeyboardJ },
	{ DtKeyK, kHIDUsage_KeyboardK },
	{ DtKeyL, kHIDUsage_KeyboardL },
	{ DtKeyM, kHIDUsage_KeyboardM },
	{ DtKeyN, kHIDUsage_KeyboardN },
	{ DtKeyO, kHIDUsage_KeyboardO },
	{ DtKeyP, kHIDUsage_KeyboardP },
	{ DtKeyQ, kHIDUsage_KeyboardQ },
	{ DtKeyR, kHIDUsage_KeyboardR },
	{ DtKeyS, kHIDUsage_KeyboardS },
	{ DtKeyT, kHIDUsage_KeyboardT },
	{ DtKeyU, kHIDUsage_KeyboardU },
	{ DtKeyV, kHIDUsage_KeyboardV },
	{ DtKeyW, kHIDUsage_KeyboardW },
	{ DtKeyX, kHIDUsage_KeyboardX },
	{ DtKeyY, kHIDUsage_KeyboardY },
	{ DtKeyZ, kHIDUsage_KeyboardZ },
	{ DtKeySpace, kHIDUsage_KeyboardSpacebar },
	{ DtKeyEscape, kHIDUsage_KeyboardEscape },
	{ DtKeyLeftShift, kHIDUsage_KeyboardLeftShift },
	{ DtKeyLeftControl, kHIDUsage_KeyboardLeftControl },
	{ DtKeyLeftMenu, kHIDUsage_KeyboardMenu },
	{ DtKeyLeftWin, kHIDUsage_KeyboardLeftAlt },
	{ DtKeyRightShift, kHIDUsage_KeyboardRightShift },
	{ DtKeyRightMenu, kHIDUsage_KeyboardMenu },
	{ DtKeyRightWin, kHIDUsage_KeyboardRightAlt },
	{ DtKeyReturn, kHIDUsage_KeyboardReturnOrEnter }
};

const KeyControlMap* findControlMapFromDefault(InputDefaultControlType control)
{
	for (int i = 0; i < sizeof_array(c_keyControlMap); ++i)
	{
		if (c_keyControlMap[i].control == control)
			return &c_keyControlMap[i];
	}
	return 0;
}

const KeyControlMap* findControlMapFromIndex(int32_t index)
{
	for (int i = 0; i < sizeof_array(c_keyControlMap); ++i)
	{
		if (c_keyControlMap[i].index == index)
			return &c_keyControlMap[i];
	}
	return 0;
}

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
	CFArrayRef elements = IOHIDDeviceCopyMatchingElements(m_deviceRef, NULL, kIOHIDOptionsTypeNone);
	for (CFIndex i = 0; i < CFArrayGetCount(elements); ++i)
	{
		IOHIDElementRef e = (IOHIDElementRef)CFArrayGetValueAtIndex(elements, i);
		if (!e)
			continue;
			
		int usage = (int)IOHIDElementGetUsage(e);
		if (usage == control)
		{
			CFStringRef name = IOHIDElementGetName(e);
			
			// Get c-style string.
			const char* cname = CFStringGetCStringPtr(name, kCFStringEncodingUTF8);
			if (cname)
				return mbstows(cname);
		}
	}

	return toString(control);
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
	const KeyControlMap* controlMap = findControlMapFromDefault(controlType);
	if (!controlMap)
		return false;
		
	control = controlMap->index;
	return true;
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

		IOHIDValueRef valueRef = 0;
		IOHIDDeviceGetValue(m_deviceRef, e, &valueRef);
		if (!valueRef)
			continue;
			
		int value = (int)IOHIDValueGetIntegerValue(valueRef);
		int keycode = (int)IOHIDElementGetUsage(e);

		if (keycode >= 0 && keycode < sizeof_array(m_data))
			m_data[keycode] = value ? 255 : 0;
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
