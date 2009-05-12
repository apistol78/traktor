#include <windows.h>
#include "Input/Win32/MouseDeviceWin32.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.MouseDeviceWin32", MouseDeviceWin32, InputDevice)

std::wstring MouseDeviceWin32::getName() const
{
	return L"Mouse";
}

InputCategory MouseDeviceWin32::getCategory() const
{
	return CtMouse;
}

int MouseDeviceWin32::getControlCount()
{
	return 0;
}

std::wstring MouseDeviceWin32::getControlName(int /*control*/)
{
	return L"";
}

bool MouseDeviceWin32::isControlAnalogue(int /*control*/) const
{
	return false;
}

float MouseDeviceWin32::getControlValue(int /*control*/)
{
	return 0;
}

bool MouseDeviceWin32::getDefaultControl(InputDefaultControlType /*controlType*/, int& /*control*/) const
{
	return false;
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
