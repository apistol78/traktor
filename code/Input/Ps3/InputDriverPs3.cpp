#include <cell/sysmodule.h>
#include "Input/Ps3/InputDriverPs3.h"
#include "Input/Ps3/InputDevicePs3.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.InputDriverPs3", 0, InputDriverPs3, IInputDriver)

InputDriverPs3::InputDriverPs3(int padCount)
{
	int32_t ret;

	cellSysmoduleLoadModule(CELL_SYSMODULE_IO);

	ret = cellPadInit(padCount);
	T_ASSERT (ret == CELL_OK);

	m_devices.resize(padCount);
	for (int i = 0; i < padCount; ++i)
		m_devices[i] = new InputDevicePs3(i);
}

bool InputDriverPs3::create(const SystemWindow& systemWindow, uint32_t inputCategories)
{
	return true;
}

int InputDriverPs3::getDeviceCount()
{
	return int(m_devices.size());
}

Ref< IInputDevice > InputDriverPs3::getDevice(int index)
{
	return m_devices[index];
}

IInputDriver::UpdateResult InputDriverPs3::update()
{
	return UrOk;
}

	}
}
