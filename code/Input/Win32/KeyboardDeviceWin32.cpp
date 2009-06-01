#include <windows.h>
#include "Input/Win32/KeyboardDeviceWin32.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.KeyboardDeviceWin32", KeyboardDeviceWin32, InputDevice)

std::wstring KeyboardDeviceWin32::getName() const
{
	return L"Keyboard";
}

InputCategory KeyboardDeviceWin32::getCategory() const
{
	return CtKeyboard;
}

bool KeyboardDeviceWin32::isConnected() const
{
	return true;
}

int KeyboardDeviceWin32::getControlCount()
{
	return 256;
}

std::wstring KeyboardDeviceWin32::getControlName(int /*control*/)
{
	return L"";
}

bool KeyboardDeviceWin32::isControlAnalogue(int /*control*/) const
{
	return false;
}

float KeyboardDeviceWin32::getControlValue(int control)
{
	SHORT state = GetAsyncKeyState(control);
	return (state & 0x8000) ? 1.0f : 0.0f;
}

bool KeyboardDeviceWin32::getDefaultControl(InputDefaultControlType controlType, int& control) const
{
	control = 0;

	switch (controlType)
	{
	case DtUp:
		control = VK_UP;
		break;

	case DtDown:
		control = VK_DOWN;
		break;

	case DtLeft:
		control = VK_LEFT;
		break;

	case DtRight:
		control = VK_RIGHT;
		break;

	case DtSelect:
		control = VK_RETURN;
		break;

	case DtCancel:
		control = VK_ESCAPE;
		break;

	case DtButton1:
		control = '1';
		break;

	case DtButton2:
		control = '2';
		break;

	case DtButton3:
		control = '3';
		break;

	case DtButton4:
		control = '4';
		break;
	}

	return bool(control != 0);
}

void KeyboardDeviceWin32::resetState()
{
}

void KeyboardDeviceWin32::readState()
{
}

bool KeyboardDeviceWin32::supportRumble() const
{
	return false;
}

void KeyboardDeviceWin32::setRumble(const InputRumble& /*rumble*/)
{
}

	}
}
