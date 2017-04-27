/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <windows.h>
#include "Core/Math/MathUtils.h"
#include "Input/Win32/MouseDeviceWin32.h"

namespace traktor
{
	namespace input
	{
		namespace
		{

const struct MouseControlMap
{
	const wchar_t* name;
	InputDefaultControlType controlType;
	bool analogue;
	bool stable;
}
c_mouseControlMap[] =
{
	{ L"Left mouse button", DtButton1, false, true },
	{ L"Right mouse button", DtButton2, false, true },
	{ L"Middle mouse button", DtButton3, false, true },
	{ L"Mouse X axis", DtAxisX, true, true },
	{ L"Mouse Y axis", DtAxisY, true, true },
	{ L"Mouse X axis", DtPositionX, true, false },
	{ L"Mouse Y axis", DtPositionY, true, false }
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.MouseDeviceWin32", MouseDeviceWin32, IInputDevice)

MouseDeviceWin32::MouseDeviceWin32(HWND hWnd)
:	m_connected(false)
,	m_hWnd(hWnd)
,	m_haveCursorPosition(false)
,	m_axisX(0.0f)
,	m_axisY(0.0f)
,	m_positionX(0.0f)
,	m_positionY(0.0f)
,	m_button1(0.0f)
,	m_button2(0.0f)
,	m_button3(0.0f)
{
	resetState();
}

std::wstring MouseDeviceWin32::getName() const
{
	return L"Standard Mouse";
}

InputCategory MouseDeviceWin32::getCategory() const
{
	return CtMouse;
}

bool MouseDeviceWin32::isConnected() const
{
	return m_connected;
}

int32_t MouseDeviceWin32::getControlCount()
{
	return sizeof_array(c_mouseControlMap);
}

std::wstring MouseDeviceWin32::getControlName(int32_t control)
{
	return c_mouseControlMap[control].name;
}

bool MouseDeviceWin32::isControlAnalogue(int32_t control) const
{
	return c_mouseControlMap[control].analogue;
}

bool MouseDeviceWin32::isControlStable(int32_t control) const
{
	return c_mouseControlMap[control].stable;
}

float MouseDeviceWin32::getControlValue(int32_t control)
{
	if (!m_connected)
		return 0.0f;

	const MouseControlMap& mc = c_mouseControlMap[control];
	if (mc.controlType == DtAxisX)
		return m_axisX;
	else if (mc.controlType == DtAxisY)
		return m_axisY;
	else if (mc.controlType == DtPositionX)
		return m_positionX;
	else if (mc.controlType == DtPositionY)
		return m_positionY;
	else if (mc.controlType == DtButton1)
		return m_button1;
	else if (mc.controlType == DtButton2)
		return m_button2;
	else if (mc.controlType == DtButton3)
		return m_button3;
	else
		return 0.0f;
}

bool MouseDeviceWin32::getControlRange(int32_t control, float& outMin, float& outMax) const
{
	return false;
}

bool MouseDeviceWin32::getDefaultControl(InputDefaultControlType controlType, bool analogue, int32_t& control) const
{
	for (uint32_t i = 0; i < sizeof_array(c_mouseControlMap); ++i)
	{
		const MouseControlMap& mc = c_mouseControlMap[i];
		if (mc.controlType == controlType && mc.analogue == analogue)
		{
			control = i;
			return true;
		}
	}
	return false;
}

bool MouseDeviceWin32::getKeyEvent(KeyEvent& outEvent)
{
	return false;
}

void MouseDeviceWin32::resetState()
{
	m_axisX = 0.0f;
	m_axisY = 0.0f;
	m_positionX = 0.0f;
	m_positionY = 0.0f;
	m_button1 = 0.0f;
	m_button2 = 0.0f;
	m_button3 = 0.0f;
	m_haveCursorPosition = false;
}

void MouseDeviceWin32::readState()
{
	if (m_connected && m_hWnd)
	{
		bool exclusive = ((GetWindowLong(m_hWnd, GWL_EXSTYLE) & WS_EX_TOPMOST) == WS_EX_TOPMOST);

		if (exclusive)
		{
			GetCursorPos(&m_cursorPosition);
			ScreenToClient(m_hWnd, &m_cursorPosition);

			RECT rc;
			GetClientRect(m_hWnd, &rc);

			POINT cursorCenter;
			cursorCenter.x = (rc.right - rc.left) / 2;
			cursorCenter.y = (rc.bottom - rc.top) / 2;

			m_axisX = float(m_cursorPosition.x - cursorCenter.x);
			m_axisY = float(m_cursorPosition.y - cursorCenter.y);

			m_positionX += m_axisX;
			m_positionY += m_axisY;

			m_positionX = clamp(m_positionX, 0.0f, float(rc.right - rc.left));
			m_positionY = clamp(m_positionY, 0.0f, float(rc.bottom - rc.top));

			ClientToScreen(m_hWnd, &cursorCenter);
			SetCursorPos(cursorCenter.x, cursorCenter.y);
		}
		else
		{
			POINT cursorPosition;
			GetCursorPos(&cursorPosition);

			if (!m_haveCursorPosition)
				m_cursorPosition = cursorPosition;

			m_axisX = float(cursorPosition.x - m_cursorPosition.x);
			m_axisY = float(cursorPosition.y - m_cursorPosition.y);

			m_positionX = float(cursorPosition.x);
			m_positionY = float(cursorPosition.y);

			m_cursorPosition = cursorPosition;
			m_haveCursorPosition = true;
		}

		{
			m_button1 = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) ? 1.0f : 0.0f;
			m_button2 = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) ? 1.0f : 0.0f;
			m_button3 = (GetAsyncKeyState(VK_MBUTTON) & 0x8000) ? 1.0f : 0.0f;
		}
	}
	else
		resetState();
}

bool MouseDeviceWin32::supportRumble() const
{
	return false;
}

void MouseDeviceWin32::setRumble(const InputRumble& /*rumble*/)
{
}

void MouseDeviceWin32::setExclusive(bool exclusive)
{
}

	}
}
