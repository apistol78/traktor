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
	{ DtKeyBack, kHIDUsage_KeyboardDeleteOrBackspace, L"Delete" },
	{ DtKeyTab, kHIDUsage_KeyboardTab, L"Tab" },
	{ DtKeyEquals, kHIDUsage_KeyboardEqualSign, L"=" },
	{ DtKeySemicolon, kHIDUsage_KeyboardSemicolon, L";" },
	{ DtKeyComma, kHIDUsage_KeyboardComma, L"," },
	{ DtKeyPeriod, kHIDUsage_KeyboardPeriod, L"." },
	{ DtKeySlash, kHIDUsage_KeyboardSlash, L"/" },
	{ DtKeyF1, kHIDUsage_KeyboardF1, L"F1" },
	{ DtKeyF2, kHIDUsage_KeyboardF2, L"F2" },
	{ DtKeyF3, kHIDUsage_KeyboardF3, L"F3" },
	{ DtKeyF4, kHIDUsage_KeyboardF4, L"F4" },
	{ DtKeyF5, kHIDUsage_KeyboardF5, L"F5" },
	{ DtKeyF6, kHIDUsage_KeyboardF6, L"F6" },
	{ DtKeyF7, kHIDUsage_KeyboardF7, L"F7" },
	{ DtKeyF8, kHIDUsage_KeyboardF8, L"F8" },
	{ DtKeyF9, kHIDUsage_KeyboardF9, L"F9" },
	{ DtKeyF10, kHIDUsage_KeyboardF10, L"F10" },
	{ DtKeyF11, kHIDUsage_KeyboardF11, L"F11" },
	{ DtKeyF12, kHIDUsage_KeyboardF12, L"F12" },
	{ DtKeyNumPadEnter, kHIDUsage_KeypadEnter, L"Num Enter" },
	{ DtKeyNumPad0, kHIDUsage_Keypad0, L"Num 0" },
	{ DtKeyNumPad1, kHIDUsage_Keypad1, L"Num 1" },
	{ DtKeyNumPad2, kHIDUsage_Keypad2, L"Num 2" },
	{ DtKeyNumPad3, kHIDUsage_Keypad3, L"Num 3" },
	{ DtKeyNumPad4, kHIDUsage_Keypad4, L"Num 4" },
	{ DtKeyNumPad5, kHIDUsage_Keypad5, L"Num 5" },
	{ DtKeyNumPad6, kHIDUsage_Keypad6, L"Num 6" },
	{ DtKeyNumPad7, kHIDUsage_Keypad7, L"Num 7" },
	{ DtKeyNumPad8, kHIDUsage_Keypad8, L"Num 8" },
	{ DtKeyNumPad9, kHIDUsage_Keypad9, L"Num 9" }
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.InputDeviceKeyboardOsX", InputDeviceKeyboardOsX, IInputDevice)

InputDeviceKeyboardOsX::InputDeviceKeyboardOsX(IOHIDDeviceRef deviceRef)
:	m_deviceRef(deviceRef)
{
	if (m_deviceRef)
	{
		IOHIDDeviceRegisterRemovalCallback(m_deviceRef, &callbackRemoval, this);
		IOHIDDeviceRegisterInputValueCallback(m_deviceRef, &callbackValue, this);
	}
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

bool InputDeviceKeyboardOsX::isControlRelative(int32_t control) const
{
	return false;
}

float InputDeviceKeyboardOsX::getControlValue(int32_t control)
{
	const KeyControlMap& controlMap = c_keyControlMap[control];
	T_ASSERT (controlMap.usage < sizeof_array(m_data));
	return m_data[controlMap.usage] ? 1.0f : 0.0f;
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
	std::memset(m_data, 0, sizeof(m_data));
}

void InputDeviceKeyboardOsX::readState()
{
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

void InputDeviceKeyboardOsX::callbackValue(void* context, IOReturn result, void* sender, IOHIDValueRef value)
{
	InputDeviceKeyboardOsX* this_ = static_cast< InputDeviceKeyboardOsX* >(context);
	
	IOHIDElementRef element = IOHIDValueGetElement(value);
	if (!element)
		return;
		
	uint32_t page = (uint32_t)IOHIDElementGetUsagePage(element);
	if (page != kHIDPage_KeyboardOrKeypad)
		return;

	uint32_t usage = (uint32_t)IOHIDElementGetUsage(element);
	if (usage >= sizeof_array(this_->m_data))
		return;

	this_->m_data[usage] = (int32_t)IOHIDValueGetIntegerValue(value) ? 255 : 0;	
}

	}
}
