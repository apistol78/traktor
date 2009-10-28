#include "Input/iPhone/InputDriverIPhone.h"
#include "Input/iPhone/InputDeviceTouch.h"
#include "Core/Heap/GcNew.h"

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
	m_inputDeviceTouch = gc_new< InputDeviceTouch >();
	if (m_inputDeviceTouch->create(nativeWindowHandle))
		return m_inputDeviceTouch;
	else
		return 0;
}

int InputDriverIPhone::getDeviceCount()
{
	return 1;
}

IInputDevice* InputDriverIPhone::getDevice(int index)
{
	if (index == 0)
		return m_inputDeviceTouch;
	else
		return 0;
}
	
	}
}
