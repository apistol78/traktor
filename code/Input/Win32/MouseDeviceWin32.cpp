#include <windows.h>
#include "Input/Win32/MouseDeviceWin32.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.MouseDeviceWin32", MouseDeviceWin32, IInputDevice)

MouseDeviceWin32::MouseDeviceWin32()
:	m_connected(false)
,	m_hWndActive(0)
,	m_haveCursorPosition(false)
,	m_axisX(0.0f)
,	m_axisY(0.0f)
,	m_button1(0.0f)
,	m_button2(0.0f)
,	m_button3(0.0f)
{
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

int MouseDeviceWin32::getControlCount()
{
	return 5;
}

std::wstring MouseDeviceWin32::getControlName(int control)
{
	InputDefaultControlType controlType = InputDefaultControlType(control);
	switch (controlType)
	{
	case DtButton1:
		return L"Left mouse button";
	case DtButton2:
		return L"Right mouse button";
	case DtButton3:
		return L"Middle mouse button";
	case DtAxisX:
		return L"Mouse X axis";
	case DtAxisY:
		return L"Mouse Y axis";
	}
	return L"";
}

bool MouseDeviceWin32::isControlAnalogue(int control) const
{
	InputDefaultControlType controlType = InputDefaultControlType(control);
	return bool(controlType >= DtButton1 && controlType <= DtButton3);
}

float MouseDeviceWin32::getControlValue(int control)
{
	if (!m_connected)
		return 0.0f;

	InputDefaultControlType controlType = InputDefaultControlType(control);
	if (controlType == DtAxisX)
		return m_axisX;
	else if(controlType == DtAxisY)
		return m_axisY;
	else if (controlType == DtButton1)
		return m_button1;
	else if (controlType == DtButton2)
		return m_button2;
	else if (controlType == DtButton3)
		return m_button3;
	else
		return 0.0f;
}

bool MouseDeviceWin32::getDefaultControl(InputDefaultControlType controlType, int& control) const
{
	control = 0;

	switch (controlType)
	{
	case DtButton1:
	case DtButton2:
	case DtButton3:
	case DtAxisX:
	case DtAxisY:
		control = int(controlType);
		break;
	}

	return bool(control != 0);
}

void MouseDeviceWin32::resetState()
{
	m_axisX = 0.0f;
	m_axisY = 0.0f;
	m_button1 = 0.0f;
	m_button2 = 0.0f;
	m_button3 = 0.0f;
	m_haveCursorPosition = false;
}

void MouseDeviceWin32::readState()
{
	if (m_connected && m_hWndActive)
	{
		bool exclusive = ((GetWindowLong(m_hWndActive, GWL_EXSTYLE) & WS_EX_TOPMOST) == WS_EX_TOPMOST);

		if (exclusive)
		{
			GetCursorPos(&m_cursorPosition);
			ScreenToClient(m_hWndActive, &m_cursorPosition);

			RECT rc;
			GetClientRect(m_hWndActive, &rc);

			POINT cursorCenter;
			cursorCenter.x = (rc.right - rc.left) / 2;
			cursorCenter.y = (rc.bottom - rc.top) / 2;

			m_axisX += float(m_cursorPosition.x - cursorCenter.x);
			m_axisY += float(m_cursorPosition.y - cursorCenter.y);

			ClientToScreen(m_hWndActive, &cursorCenter);
			SetCursorPos(cursorCenter.x, cursorCenter.y);
		}
		else
		{
			POINT cursorPosition;
			GetCursorPos(&cursorPosition);

			if (!m_haveCursorPosition)
				m_cursorPosition = cursorPosition;

			m_axisX += float(cursorPosition.x - m_cursorPosition.x);
			m_axisY += float(cursorPosition.y - m_cursorPosition.y);

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

	}
}
