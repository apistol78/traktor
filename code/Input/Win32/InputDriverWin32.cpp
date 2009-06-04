#include "Input/Win32/InputDriverWin32.h"
#include "Input/Win32/KeyboardDeviceWin32.h"
#include "Input/Win32/MouseDeviceWin32.h"
#include "Core/Heap/New.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.InputDriverWin32", InputDriverWin32, InputDriver)

InputDriverWin32::InputDriverWin32()
:	m_keyboardDevice(gc_new< KeyboardDeviceWin32 >())
,	m_mouseDevice(gc_new< MouseDeviceWin32 >())
{
}

int InputDriverWin32::getDeviceCount()
{
	return 2;
}

InputDevice* InputDriverWin32::getDevice(int index)
{
	if (index == 0)
		return m_keyboardDevice;
	if (index == 1)
		return m_mouseDevice;
	return 0;
}

	}
}
