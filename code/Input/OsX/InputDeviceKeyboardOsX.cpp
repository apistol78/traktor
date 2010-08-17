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
	uint32_t usage;
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
	{ DtKey0, kHIDUsage_Keyboard0 },
	{ DtKey1, kHIDUsage_Keyboard1 },
	{ DtKey2, kHIDUsage_Keyboard2 },
	{ DtKey3, kHIDUsage_Keyboard3 },
	{ DtKey4, kHIDUsage_Keyboard4 },
	{ DtKey5, kHIDUsage_Keyboard5 },
	{ DtKey6, kHIDUsage_Keyboard6 },
	{ DtKey7, kHIDUsage_Keyboard7 },
	{ DtKey8, kHIDUsage_Keyboard8 },
	{ DtKey9, kHIDUsage_Keyboard9 },
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

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.InputDeviceKeyboardOsX", InputDeviceKeyboardOsX, IInputDevice)

InputDeviceKeyboardOsX::InputDeviceKeyboardOsX(IOHIDDeviceRef deviceRef)
:	m_deviceRef(deviceRef)
,	m_data(new uint8_t [sizeof_array(c_keyControlMap)])
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
	const KeyControlMap& controlMap = c_keyControlMap[control];
	
	CFArrayRef elements = IOHIDDeviceCopyMatchingElements(m_deviceRef, NULL, kIOHIDOptionsTypeNone);
	for (CFIndex i = 0; i < CFArrayGetCount(elements); ++i)
	{
		IOHIDElementRef e = (IOHIDElementRef)CFArrayGetValueAtIndex(elements, i);
		if (!e)
			continue;
			
		if (IOHIDElementGetUsage(e) == controlMap.usage)
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

bool InputDeviceKeyboardOsX::getDefaultControl(InputDefaultControlType controlType, bool analogue, int& control) const
{
	if (analogue)
		return false;

	for (int32_t i = 0; i < sizeof_array(c_keyControlMap); ++i)
	{
		const KeyControlMap& controlMap = c_keyControlMap[i];
		if (controlMap.control == controlType)
		{
			control = i;
			return true;
		}
	}

	return false;
}

void InputDeviceKeyboardOsX::resetState()
{
}

void InputDeviceKeyboardOsX::readState()
{
	if (!m_deviceRef)
		return;
		
	const uint32_t dataSize = sizeof_array(c_keyControlMap) * sizeof(uint8_t);
	std::memset(m_data.ptr(), 0, dataSize);

	CFArrayRef elements = IOHIDDeviceCopyMatchingElements(m_deviceRef, NULL, kIOHIDOptionsTypeNone);
	for (CFIndex i = 0; i < CFArrayGetCount(elements); ++i)
	{
		IOHIDElementRef e = (IOHIDElementRef)CFArrayGetValueAtIndex(elements, i);
		if (!e)
			continue;

		uint32_t usage = (uint32_t)IOHIDElementGetUsage(e);
		for (uint32_t j = 0; j < sizeof_array(c_keyControlMap); ++j)
		{
			const KeyControlMap& controlMap = c_keyControlMap[j];
			if (controlMap.usage == usage)
			{
				IOHIDValueRef valueRef = 0;
				IOHIDDeviceGetValue(m_deviceRef, e, &valueRef);
				if (valueRef)
				{
					int32_t value = (int32_t)IOHIDValueGetIntegerValue(valueRef);
					if (value != 0)
						m_data[j] |= 255;
				}
				break;
			}
		}
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
