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
	const wchar_t* name;
}
c_keyControlMap[] =
{
	{ DtKeyUp, kHIDUsage_KeyboardUpArrow, L"Up" },
	{ DtKeyDown, kHIDUsage_KeyboardDownArrow, L"Down" },
	{ DtKeyLeft, kHIDUsage_KeyboardLeftArrow, L"Left" },
	{ DtKeyRight, kHIDUsage_KeyboardRightArrow, L"Right" },
	{ DtKeyA, kHIDUsage_KeyboardA, L"A" },
	{ DtKeyB, kHIDUsage_KeyboardB, L"B" },
	{ DtKeyC, kHIDUsage_KeyboardC, L"C" },
	{ DtKeyD, kHIDUsage_KeyboardD, L"D" },
	{ DtKeyE, kHIDUsage_KeyboardE, L"E" },
	{ DtKeyF, kHIDUsage_KeyboardF, L"F" },
	{ DtKeyG, kHIDUsage_KeyboardG, L"G" },
	{ DtKeyH, kHIDUsage_KeyboardH, L"H" },
	{ DtKeyI, kHIDUsage_KeyboardI, L"I" },
	{ DtKeyJ, kHIDUsage_KeyboardJ, L"J" },
	{ DtKeyK, kHIDUsage_KeyboardK, L"K" },
	{ DtKeyL, kHIDUsage_KeyboardL, L"L" },
	{ DtKeyM, kHIDUsage_KeyboardM, L"M" },
	{ DtKeyN, kHIDUsage_KeyboardN, L"N" },
	{ DtKeyO, kHIDUsage_KeyboardO, L"O" },
	{ DtKeyP, kHIDUsage_KeyboardP, L"P" },
	{ DtKeyQ, kHIDUsage_KeyboardQ, L"Q" },
	{ DtKeyR, kHIDUsage_KeyboardR, L"R" },
	{ DtKeyS, kHIDUsage_KeyboardS, L"S" },
	{ DtKeyT, kHIDUsage_KeyboardT, L"T" },
	{ DtKeyU, kHIDUsage_KeyboardU, L"U" },
	{ DtKeyV, kHIDUsage_KeyboardV, L"V" },
	{ DtKeyW, kHIDUsage_KeyboardW, L"W" },
	{ DtKeyX, kHIDUsage_KeyboardX, L"X" },
	{ DtKeyY, kHIDUsage_KeyboardY, L"Y" },
	{ DtKeyZ, kHIDUsage_KeyboardZ, L"Z" },
	{ DtKey0, kHIDUsage_Keyboard0, L"0" },
	{ DtKey1, kHIDUsage_Keyboard1, L"1" },
	{ DtKey2, kHIDUsage_Keyboard2, L"2" },
	{ DtKey3, kHIDUsage_Keyboard3, L"3" },
	{ DtKey4, kHIDUsage_Keyboard4, L"4" },
	{ DtKey5, kHIDUsage_Keyboard5, L"5" },
	{ DtKey6, kHIDUsage_Keyboard6, L"6" },
	{ DtKey7, kHIDUsage_Keyboard7, L"7" },
	{ DtKey8, kHIDUsage_Keyboard8, L"8" },
	{ DtKey9, kHIDUsage_Keyboard9, L"9" },
	{ DtKeySpace, kHIDUsage_KeyboardSpacebar, L"Space" },
	{ DtKeyEscape, kHIDUsage_KeyboardEscape, L"Escape" },
	{ DtKeyLeftShift, kHIDUsage_KeyboardLeftShift, L"Left shift" },
	{ DtKeyLeftControl, kHIDUsage_KeyboardLeftControl, L"Left ctrl" },
	{ DtKeyLeftMenu, kHIDUsage_KeyboardLeftGUI, L"Left cmd" },
	{ DtKeyLeftWin, kHIDUsage_KeyboardLeftAlt, L"Left alt" },
	{ DtKeyRightShift, kHIDUsage_KeyboardRightShift, L"Right shift" },
	{ DtKeyRightMenu, kHIDUsage_KeyboardRightGUI, L"Right cmd" },
	{ DtKeyRightWin, kHIDUsage_KeyboardRightAlt, L"Right alt" },
	{ DtKeyReturn, kHIDUsage_KeyboardReturnOrEnter, L"Enter" },
	{ DtKeyBack, kHIDUsage_KeyboardDeleteOrBackspace, L"Delete" }
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.InputDeviceKeyboardOsX", InputDeviceKeyboardOsX, IInputDevice)

InputDeviceKeyboardOsX::InputDeviceKeyboardOsX(IOHIDDeviceRef deviceRef)
:	m_deviceRef(deviceRef)
,	m_data(new uint8_t [sizeof_array(c_keyControlMap)])
{
	if (m_deviceRef)
		IOHIDDeviceRegisterRemovalCallback(m_deviceRef, &callbackRemoval, this);

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

int32_t InputDeviceKeyboardOsX::getControlCount()
{
	return sizeof_array(c_keyControlMap);
}

std::wstring InputDeviceKeyboardOsX::getControlName(int32_t control)
{
	const KeyControlMap& controlMap = c_keyControlMap[control];
	return controlMap.name;
}

bool InputDeviceKeyboardOsX::isControlAnalogue(int32_t control) const
{
	return false;
}

float InputDeviceKeyboardOsX::getControlValue(int32_t control)
{
	if (m_data[control])
		return 1.0f;
	else
		return 0.0f;
}

bool InputDeviceKeyboardOsX::getDefaultControl(InputDefaultControlType controlType, bool analogue, int32_t& control) const
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
	const uint32_t dataSize = sizeof_array(c_keyControlMap) * sizeof(uint8_t);
	std::memset(m_data.ptr(), 0, dataSize);
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

void InputDeviceKeyboardOsX::callbackRemoval(void* context, IOReturn result, void* sender)
{
	InputDeviceKeyboardOsX* this_ = static_cast< InputDeviceKeyboardOsX* >(context);
	this_->m_deviceRef = 0;
}

	}
}
