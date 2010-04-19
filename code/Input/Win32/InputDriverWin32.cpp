#include "Input/Win32/InputDriverWin32.h"
#include "Input/Win32/KeyboardDeviceWin32.h"
#include "Input/Win32/MouseDeviceWin32.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.InputDriverWin32", InputDriverWin32, IInputDriver)

InputDriverWin32::InputDriverWin32()
:	m_keyboardDevice(new KeyboardDeviceWin32())
,	m_mouseDevice(new MouseDeviceWin32())
{
}

int InputDriverWin32::getDeviceCount()
{
	return 2;
}

Ref< IInputDevice > InputDriverWin32::getDevice(int index)
{
	if (index == 0)
		return m_keyboardDevice;
	if (index == 1)
		return m_mouseDevice;
	return 0;
}

IInputDriver::UpdateResult InputDriverWin32::update()
{
	return UrOk;
}

	}
}
