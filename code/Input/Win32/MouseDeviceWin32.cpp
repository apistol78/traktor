#include <windows.h>
#include "Input/Win32/MouseDeviceWin32.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.MouseDeviceWin32", MouseDeviceWin32, IInputDevice)

MouseDeviceWin32::MouseDeviceWin32()
:	m_connected(false)
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
	if (m_connected)
	{
		InputDefaultControlType controlType = InputDefaultControlType(control);
		if (controlType == DtAxisX || controlType == DtAxisY)
		{
			POINT cursorPosition;
			if (GetCursorPos(&cursorPosition))
				return (controlType == DtAxisX) ? float(cursorPosition.x) : float(cursorPosition.y);
		}
		else if (controlType == DtButton1)
			return (GetAsyncKeyState(VK_LBUTTON) & 0x8000) ? 1.0f : 0.0f;
		else if (controlType == DtButton2)
			return (GetAsyncKeyState(VK_RBUTTON) & 0x8000) ? 1.0f : 0.0f;
		else if (controlType == DtButton3)
			return (GetAsyncKeyState(VK_MBUTTON) & 0x8000) ? 1.0f : 0.0f;
	}
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
}

void MouseDeviceWin32::readState()
{
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
