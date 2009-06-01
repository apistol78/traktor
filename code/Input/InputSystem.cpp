#include <algorithm>
#include "Input/InputSystem.h"
#include "Input/InputDriver.h"
#include "Input/InputDevice.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.InputSystem", InputSystem, Object)

void InputSystem::addDriver(InputDriver* inputDriver)
{
	m_drivers.push_back(inputDriver);
	updateDevices();
}

void InputSystem::removeDriver(InputDriver* inputDriver)
{
	RefArray< InputDriver >::iterator i = std::find(m_drivers.begin(), m_drivers.end(), inputDriver);
	if (i != m_drivers.end())
	{
		m_drivers.erase(i);
		updateDevices();
	}
}

void InputSystem::addDevice(InputDevice* inputDevice)
{
	m_devices.push_back(inputDevice);
}

void InputSystem::removeDevice(InputDevice* inputDevice)
{
	RefArray< InputDevice >::iterator i = std::find(m_devices.begin(), m_devices.end(), inputDevice);
	if (i != m_devices.end())
		m_devices.erase(i);
}

int InputSystem::getDeviceCount() const
{
	return int(m_devices.size());
}

InputDevice* InputSystem::getDevice(int index)
{
	T_ASSERT (index >= 0 && index < getDeviceCount());
	return m_devices[index];
}

int InputSystem::getDeviceCount(InputCategory category) const
{
	int deviceCount = 0;
	for (RefArray< InputDevice >::const_iterator i = m_devices.begin(); i != m_devices.end(); ++i)
	{
		if ((*i)->getCategory() == category)
			++deviceCount;
	}
	return deviceCount;
}

InputDevice* InputSystem::getDevice(InputCategory category, int index, bool connected)
{
	for (RefArray< InputDevice >::iterator i = m_devices.begin(); i != m_devices.end(); ++i)
	{
		if ((*i)->getCategory() == category && (!connected || (*i)->isConnected()))
		{
			if (index-- <= 0)
				return *i;
		}
	}
	return 0;
}

bool InputSystem::update(float deltaTime)
{
	for (RefArray< InputDevice >::iterator i = m_devices.begin(); i != m_devices.end(); ++i)
	{
		InputDevice* inputDevice = *i;
		T_ASSERT (inputDevice);

		inputDevice->readState();
	}
	return true;
}

void InputSystem::updateDevices()
{
	m_devices.resize(0);
	for (RefArray< InputDriver >::iterator i = m_drivers.begin(); i != m_drivers.end(); ++i)
	{
		InputDriver* inputDriver = *i;
		T_ASSERT (inputDriver);

		for (int j = 0; j < inputDriver->getDeviceCount(); ++j)
		{
			Ref< InputDevice > inputDevice = inputDriver->getDevice(j);
			T_ASSERT (inputDevice);

			m_devices.push_back(inputDevice);
		}
	}
}

	}
}
