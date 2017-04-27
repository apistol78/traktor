/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <windows.h>
#include "Core/Misc/TString.h"
#include "Input/Win32/KeyboardDeviceWin32.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.KeyboardDeviceWin32", KeyboardDeviceWin32, IInputDevice)

KeyboardDeviceWin32::KeyboardDeviceWin32(HWND hWnd)
:	m_connected(false)
,	m_hWnd(hWnd)
,	m_pWndProc(0)
{
	T_FATAL_ASSERT (GetWindowLongPtr(m_hWnd, GWLP_USERDATA) == 0);

	// Subclass window to get access to window events.
	m_pWndProc = (WNDPROC)GetWindowLongPtr(m_hWnd, GWLP_WNDPROC);
	SetWindowLongPtr(m_hWnd, GWLP_WNDPROC, (LONG_PTR)&KeyboardDeviceWin32::wndProc);
	SetWindowLongPtr(m_hWnd, GWLP_USERDATA, (LONG_PTR)this);

	// Set initally reset.
	resetState();
}

KeyboardDeviceWin32::~KeyboardDeviceWin32()
{
	// Restore original window proc.
	SetWindowLongPtr(m_hWnd, GWLP_WNDPROC, (LONG_PTR)m_pWndProc);
	SetWindowLongPtr(m_hWnd, GWLP_USERDATA, 0);
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
	if (GetKeyNameText(scanCode << 16, keyName, sizeof_array(keyName)) != 0)
		return tstows(keyName);
	else
		return L"";
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

bool KeyboardDeviceWin32::getKeyEvent(KeyEvent& outEvent)
{
	if (m_keyEvents.empty())
		return false;

	outEvent = m_keyEvents.front();
	m_keyEvents.pop_front();

	return true;
}

void KeyboardDeviceWin32::resetState()
{
	std::memset(m_keyStates, 0, sizeof(m_keyStates));
	m_keyEvents.clear();
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

LRESULT WINAPI KeyboardDeviceWin32::wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	KeyboardDeviceWin32* this_ = reinterpret_cast< KeyboardDeviceWin32* >(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	T_ASSERT (this_);

	if (uMsg == WM_CHAR)
	{
		if (wParam != '\r')
		{
			KeyEvent ke;
			ke.type = KtCharacter;
			ke.character = (wchar_t)wParam;
			this_->m_keyEvents.push_back(ke);
		}
	}
	else if (uMsg == WM_KEYDOWN)
	{
		uint32_t keyCode = translateFromVk(uint32_t(wParam));
		if (keyCode != 0)
		{
			KeyEvent ke;
			ke.type = KtDown;
			ke.keyCode = keyCode;
			this_->m_keyEvents.push_back(ke);
		}
	}
	else if (uMsg == WM_KEYUP)
	{
		uint32_t keyCode = translateFromVk(uint32_t(wParam));
		if (keyCode != 0)
		{
			KeyEvent ke;
			ke.type = KtUp;
			ke.keyCode = keyCode;
			this_->m_keyEvents.push_back(ke);
		}
	}

	return CallWindowProc(this_->m_pWndProc, hWnd, uMsg, wParam, lParam);
}

	}
}
