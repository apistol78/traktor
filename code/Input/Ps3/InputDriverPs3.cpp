#include "Input/Ps3/InputDriverPs3.h"
#include "Input/Ps3/InputDevicePs3.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_CLASS("traktor.input.InputDriverPs3", InputDriverPs3, InputDriver)

InputDriverPs3::InputDriverPs3(int padCount)
{
	int32_t ret;

	ret = cellPadInit(padCount);
	assert (ret == CELL_OK);

	m_devices.resize(padCount);
	for (int i = 0; i < padCount; ++i)
		m_devices[i] = gc_new< InputDevicePs3 >(i);
}

int InputDriverPs3::getDeviceCount()
{
	return int(m_devices.size());
}

InputDevice* InputDriverPs3::getDevice(int index)
{
	return m_devices[index];
}

	}
}
