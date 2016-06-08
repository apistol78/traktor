#include <Carbon/Carbon.h>

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

std::wstring fromNSString(const NSString* str)
{
	char buffer[4096];
	[str getCString: buffer maxLength: sizeof_array(buffer) encoding: NSUTF8StringEncoding];
	return mbstows(buffer);
}

struct KeyControlMap
{
	InputDefaultControlType control;
	uint16_t keyCode;
	const wchar_t* name;
}
c_keyControlMap[] =
{
	{ DtKeyUp, kVK_UpArrow, L"Up" },
	{ DtKeyDown, kVK_DownArrow, L"Down" },
	{ DtKeyLeft, kVK_LeftArrow, L"Left" },
	{ DtKeyRight, kVK_RightArrow, L"Right" },
	{ DtKeyA, kVK_ANSI_A, L"A" },
	{ DtKeyB, kVK_ANSI_B, L"B" },
	{ DtKeyC, kVK_ANSI_C, L"C" },
	{ DtKeyD, kVK_ANSI_D, L"D" },
	{ DtKeyE, kVK_ANSI_E, L"E" },
	{ DtKeyF, kVK_ANSI_F, L"F" },
	{ DtKeyG, kVK_ANSI_G, L"G" },
	{ DtKeyH, kVK_ANSI_H, L"H" },
	{ DtKeyI, kVK_ANSI_I, L"I" },
	{ DtKeyJ, kVK_ANSI_J, L"J" },
	{ DtKeyK, kVK_ANSI_K, L"K" },
	{ DtKeyL, kVK_ANSI_L, L"L" },
	{ DtKeyM, kVK_ANSI_M, L"M" },
	{ DtKeyN, kVK_ANSI_N, L"N" },
	{ DtKeyO, kVK_ANSI_O, L"O" },
	{ DtKeyP, kVK_ANSI_P, L"P" },
	{ DtKeyQ, kVK_ANSI_Q, L"Q" },
	{ DtKeyR, kVK_ANSI_R, L"R" },
	{ DtKeyS, kVK_ANSI_S, L"S" },
	{ DtKeyT, kVK_ANSI_T, L"T" },
	{ DtKeyU, kVK_ANSI_U, L"U" },
	{ DtKeyV, kVK_ANSI_V, L"V" },
	{ DtKeyW, kVK_ANSI_W, L"W" },
	{ DtKeyX, kVK_ANSI_X, L"X" },
	{ DtKeyY, kVK_ANSI_Y, L"Y" },
	{ DtKeyZ, kVK_ANSI_Z, L"Z" },
	{ DtKey0, kVK_ANSI_0, L"0" },
	{ DtKey1, kVK_ANSI_1, L"1" },
	{ DtKey2, kVK_ANSI_2, L"2" },
	{ DtKey3, kVK_ANSI_3, L"3" },
	{ DtKey4, kVK_ANSI_4, L"4" },
	{ DtKey5, kVK_ANSI_5, L"5" },
	{ DtKey6, kVK_ANSI_6, L"6" },
	{ DtKey7, kVK_ANSI_7, L"7" },
	{ DtKey8, kVK_ANSI_8, L"8" },
	{ DtKey9, kVK_ANSI_9, L"9" },
	{ DtKeySpace, kVK_Space, L"Space" },
	{ DtKeyEscape, kVK_Escape, L"Escape" },
	{ DtKeyLeftShift, kVK_Shift, L"Left shift" },
	{ DtKeyLeftControl, kVK_Control, L"Left ctrl" },
	{ DtKeyLeftMenu, kVK_Command, L"Left cmd" },
	{ DtKeyLeftWin, kVK_Option, L"Left alt" },
	{ DtKeyRightShift, kVK_RightShift, L"Right shift" },
	{ DtKeyRightControl, kVK_RightControl, L"Right ctrl" },
	{ DtKeyRightWin, kVK_RightOption, L"Right alt" },
	{ DtKeyReturn, kVK_Return, L"Return" },
	{ DtKeyBack, kVK_Delete, L"Delete" },
	{ DtKeyTab, kVK_Tab, L"Tab" },
	{ DtKeyEquals, kVK_ANSI_KeypadEquals, L"=" },
	{ DtKeySemicolon, kVK_ANSI_Semicolon, L";" },
	{ DtKeyComma, kVK_ANSI_Comma, L"," },
	{ DtKeyPeriod, kVK_ANSI_Period, L"." },
	{ DtKeySlash, kVK_ANSI_Slash, L"/" },
	{ DtKeyF1, kVK_F1, L"F1" },
	{ DtKeyF2, kVK_F2, L"F2" },
	{ DtKeyF3, kVK_F3, L"F3" },
	{ DtKeyF4, kVK_F4, L"F4" },
	{ DtKeyF5, kVK_F5, L"F5" },
	{ DtKeyF6, kVK_F6, L"F6" },
	{ DtKeyF7, kVK_F7, L"F7" },
	{ DtKeyF8, kVK_F8, L"F8" },
	{ DtKeyF9, kVK_F9, L"F9" },
	{ DtKeyF10, kVK_F10, L"F10" },
	{ DtKeyF11, kVK_F11, L"F11" },
	{ DtKeyF12, kVK_F12, L"F12" },
	{ DtKeyNumPadEnter, kVK_ANSI_KeypadEnter, L"Num Enter" },
	{ DtKeyNumPad0, kVK_ANSI_Keypad0, L"Num 0" },
	{ DtKeyNumPad1, kVK_ANSI_Keypad1, L"Num 1" },
	{ DtKeyNumPad2, kVK_ANSI_Keypad2, L"Num 2" },
	{ DtKeyNumPad3, kVK_ANSI_Keypad3, L"Num 3" },
	{ DtKeyNumPad4, kVK_ANSI_Keypad4, L"Num 4" },
	{ DtKeyNumPad5, kVK_ANSI_Keypad5, L"Num 5" },
	{ DtKeyNumPad6, kVK_ANSI_Keypad6, L"Num 6" },
	{ DtKeyNumPad7, kVK_ANSI_Keypad7, L"Num 7" },
	{ DtKeyNumPad8, kVK_ANSI_Keypad8, L"Num 8" },
	{ DtKeyNumPad9, kVK_ANSI_Keypad9, L"Num 9" }
};

int32_t controlFromKeyCode(uint16_t keyCode)
{
	for (int32_t i = 0; i < sizeof_array(c_keyControlMap); ++i)
	{
		if (c_keyControlMap[i].keyCode == keyCode)
			return i;
	}
	return -1;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.InputDeviceKeyboardOsX", InputDeviceKeyboardOsX, IInputDevice)

InputDeviceKeyboardOsX::InputDeviceKeyboardOsX()
{
	resetState();
}

std::wstring InputDeviceKeyboardOsX::getName() const
{
	return L"Keyboard";
}

InputCategory InputDeviceKeyboardOsX::getCategory() const
{
	return CtKeyboard;
}

bool InputDeviceKeyboardOsX::isConnected() const
{
	return true;
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

bool InputDeviceKeyboardOsX::isControlStable(int32_t control) const
{
	return true;
}

float InputDeviceKeyboardOsX::getControlValue(int32_t control)
{
	return m_data[control] ? 1.0f : 0.0f;
}

bool InputDeviceKeyboardOsX::getControlRange(int32_t control, float& outMin, float& outMax) const
{
	return false;
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

bool InputDeviceKeyboardOsX::getKeyEvent(KeyEvent& outEvent)
{
	if (m_keyEvents.empty())
		return false;

	outEvent = m_keyEvents.front();
	m_keyEvents.pop_front();

	return true;
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

void InputDeviceKeyboardOsX::setExclusive(bool exclusive)
{
}

void InputDeviceKeyboardOsX::consumeEvent(NSEvent* event)
{
	switch ([event type])
	{
	case NSKeyDown:
		{
			if (![event isARepeat])
			{
				int32_t control = controlFromKeyCode([event keyCode]);
				if (control >= 0)
				{
					KeyEvent ke;
					ke.type = KtDown;
					ke.character = c_keyControlMap[control].control;
					m_keyEvents.push_back(ke);
					m_data[control] = 255;
				}
			}

			if ([event keyCode] == kVK_Delete)
			{
				KeyEvent ke;
				ke.type = KtCharacter;
				ke.character = L'\b';
				m_keyEvents.push_back(ke);
			}
			else
			{
				NSString* cs = [event characters];
				if (cs)
				{
					std::wstring chrs = fromNSString(cs);
					for (size_t i = 0; i < chrs.size(); ++i)
					{
						KeyEvent ke;
						ke.type = KtCharacter;
						ke.character = wchar_t(chrs[i]);
						m_keyEvents.push_back(ke);
					}
				}
			}
		}
		break;

	case NSKeyUp:
		{
			int32_t control = controlFromKeyCode([event keyCode]);
			if (control >= 0)
			{
				KeyEvent ke;
				ke.type = KtUp;
				ke.character = c_keyControlMap[control].control;
				m_keyEvents.push_back(ke);
				m_data[control] = 0;
			}
		}
		break;
	}
}

	}
}
