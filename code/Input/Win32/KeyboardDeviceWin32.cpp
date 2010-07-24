#include <windows.h>
#include "Core/Misc/TString.h"
#include "Input/Win32/TypesWin32.h"
#include "Input/Win32/KeyboardDeviceWin32.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.KeyboardDeviceWin32", KeyboardDeviceWin32, IInputDevice)

KeyboardDeviceWin32::KeyboardDeviceWin32()
:	m_connected(false)
{
}

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
	return m_connected;
}

int KeyboardDeviceWin32::getControlCount()
{
	return sizeof_array(c_vkControlKeys);
}

std::wstring KeyboardDeviceWin32::getControlName(int control)
{
	UINT scanCode = MapVirtualKey(control, MAPVK_VK_TO_VSC);

	switch (control)
	{
	case VK_LEFT: case VK_UP: case VK_RIGHT: case VK_DOWN:
	case VK_PRIOR: case VK_NEXT: case VK_END: case VK_HOME:
	case VK_INSERT: case VK_DELETE: case VK_DIVIDE: case VK_NUMLOCK:
		scanCode |= 0x100;
		break;
	}

	TCHAR keyName[50];
	if (GetKeyNameText(scanCode << 16, keyName, sizeof(keyName)) != 0)
		return tstows(keyName);
	else
		return L"";
}

bool KeyboardDeviceWin32::isControlAnalogue(int /*control*/) const
{
	return false;
}

float KeyboardDeviceWin32::getControlValue(int control)
{
	if (m_connected)
	{
		SHORT state = GetAsyncKeyState(control);
		return (state & 0x8000) ? 1.0f : 0.0f;
	}
	else
		return 0.0f;
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
