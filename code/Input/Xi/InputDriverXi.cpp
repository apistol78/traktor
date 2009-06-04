#include "Input/Xi/InputDriverXi.h"
#include "Input/Xi/InputDeviceXi.h"
#include "Core/Heap/New.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.InputDriverXi", InputDriverXi, InputDriver)

InputDriverXi::InputDriverXi(DWORD deviceCount)
{
	for (DWORD i = 0; i < deviceCount; ++i)
		m_devices.push_back(gc_new< InputDeviceXi >(i));
}

int InputDriverXi::getDeviceCount()
{
	return int(m_devices.size());
}

InputDevice* InputDriverXi::getDevice(int index)
{
	return m_devices[index];
}

	}
}