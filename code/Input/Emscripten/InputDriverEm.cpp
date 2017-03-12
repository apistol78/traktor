#include "Core/Log/Log.h"
#include "Input/Emscripten/InputDriverEm.h"
#include "Input/Emscripten/MouseDeviceEm.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.InputDriverEm", 0, InputDriverEm, IInputDriver)

InputDriverEm::InputDriverEm()
{
}

InputDriverEm::~InputDriverEm()
{
}

bool InputDriverEm::create(const SystemApplication& sysapp, const SystemWindow& syswin, uint32_t inputCategories)
{
	if (inputCategories & CtMouse)
	{
		m_mouseDevice = new MouseDeviceEm();
		m_devices.push_back(m_mouseDevice);
	}

	log::info << L"Emscripten input driver created successfully." << Endl;
	return true;
}

int InputDriverEm::getDeviceCount()
{
	return int(m_devices.size());
}

Ref< IInputDevice > InputDriverEm::getDevice(int index)
{
	if (index < int(m_devices.size()))
		return m_devices[index];
	else
		return 0;
}

IInputDriver::UpdateResult InputDriverEm::update()
{
	return UrOk;
}

	}
}
