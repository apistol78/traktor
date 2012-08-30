#include <windows.h>
#include "Core/Misc/TString.h"
#include "Input/Win32/KeyboardDeviceWin32.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.KeyboardDeviceWin32", KeyboardDeviceWin32, IInputDevice)

KeyboardDeviceWin32::KeyboardDeviceWin32()
:	m_connected(false)
{
	resetState();
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

int32_t KeyboardDeviceWin32::getControlCount()
{
	return sizeof_array(c_vkControlKeys);
}

std::wstring KeyboardDeviceWin32::getControlName(int32_t control)
{
#if !defined(WINCE)
	UINT scanCode = MapVirtualKey(c_vkControlKeys[control], MAPVK_VK_TO_VSC);

	switch (c_vkControlKeys[control])
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
#else
	return L"";
#endif
}

bool KeyboardDeviceWin32::isControlAnalogue(int32_t control) const
{
	return false;
}

bool KeyboardDeviceWin32::isControlStable(int32_t control) const
{
	return false;
}

float KeyboardDeviceWin32::getControlValue(int32_t control)
{
	if (m_connected)
		return m_keyStates[control] ? 1.0f : 0.0f;
	else
		return 0.0f;
}

bool KeyboardDeviceWin32::getControlRange(int32_t control, float& outMin, float& outMax) const
{
	outMin = 0.0f;
	outMax = 1.0f;
	return true;
}

bool KeyboardDeviceWin32::getDefaultControl(InputDefaultControlType controlType, bool analogue, int32_t& control) const
{
	if (analogue || !c_vkControlKeys[int32_t(controlType)])
		return false;

	control = int32_t(controlType);
	return true;
}

void KeyboardDeviceWin32::resetState()
{
	std::memset(m_keyStates, 0, sizeof(m_keyStates));
}

void KeyboardDeviceWin32::readState()
{
	if (m_connected)
	{
		for (int32_t i = 0; i < sizeof_array(c_vkControlKeys); ++i)
		{
			if (c_vkControlKeys[i] == 0)
				continue;

			SHORT keyState = GetAsyncKeyState(c_vkControlKeys[i]);
			if (keyState & 0x8000)
				m_keyStates[i] = 0xff;
			else
				m_keyStates[i] = 0x00;
		}
	}
	else
		resetState();
}

bool KeyboardDeviceWin32::supportRumble() const
{
	return false;
}

void KeyboardDeviceWin32::setRumble(const InputRumble& /*rumble*/)
{
}

void KeyboardDeviceWin32::setExclusive(bool exclusive)
{
}

	}
}
