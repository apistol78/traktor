#include <windows.h>
#include "Input/Win32/TypesWin32.h"
#include "Input/Win32/KeyboardDeviceWin32.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.KeyboardDeviceWin32", KeyboardDeviceWin32, IInputDevice)

std::wstring KeyboardDeviceWin32::getName() const
{
	return L"Standard Keyboard";
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
	return sizeof_array(c_vkControlKeys);
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
	control = c_vkControlKeys[int(controlType)];
	return true;
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
