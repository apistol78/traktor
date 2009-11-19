#include "Input/iPhone/InputDriverIPhone.h"
#include "Input/iPhone/InputDeviceTouch.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.InputDriverIPhone", InputDriverIPhone, IInputDriver)

InputDriverIPhone::InputDriverIPhone()
{
}

bool InputDriverIPhone::create(void* nativeWindowHandle)
{
	m_inputDeviceTouch = new InputDeviceTouch();
	if (m_inputDeviceTouch->create(nativeWindowHandle))
		return m_inputDeviceTouch;
	else
		return 0;
}

int InputDriverIPhone::getDeviceCount()
{
	return 1;
}

Ref< IInputDevice > InputDriverIPhone::getDevice(int index)
{
	if (index == 0)
		return m_inputDeviceTouch;
	else
		return 0;
}
	
	}
}
