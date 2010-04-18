#include <algorithm>
#include "Input/InputSystem.h"
#include "Input/IInputDriver.h"
#include "Input/IInputDevice.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.InputSystem", InputSystem, Object)

void InputSystem::addDriver(IInputDriver* inputDriver)
{
	m_drivers.push_back(inputDriver);
	updateDevices();
}

void InputSystem::removeDriver(IInputDriver* inputDriver)
{
	RefArray< IInputDriver >::iterator i = std::find(m_drivers.begin(), m_drivers.end(), inputDriver);
	if (i != m_drivers.end())
	{
		m_drivers.erase(i);
		updateDevices();
	}
}

void InputSystem::addDevice(IInputDevice* inputDevice)
{
	m_devices.push_back(inputDevice);
}

void InputSystem::removeDevice(IInputDevice* inputDevice)
{
	RefArray< IInputDevice >::iterator i = std::find(m_devices.begin(), m_devices.end(), inputDevice);
	if (i != m_devices.end())
		m_devices.erase(i);
}

int InputSystem::getDeviceCount() const
{
	return int(m_devices.size());
}

Ref< IInputDevice > InputSystem::getDevice(int index)
{
	T_ASSERT (index >= 0 && index < getDeviceCount());
	return m_devices[index];
}

int InputSystem::getDeviceCount(InputCategory category) const
{
	int deviceCount = 0;
	for (RefArray< IInputDevice >::const_iterator i = m_devices.begin(); i != m_devices.end(); ++i)
	{
		if ((*i)->getCategory() == category)
			++deviceCount;
	}
	return deviceCount;
}

Ref< IInputDevice > InputSystem::getDevice(InputCategory category, int index, bool connected)
{
	for (RefArray< IInputDevice >::iterator i = m_devices.begin(); i != m_devices.end(); ++i)
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
	bool shouldUpdateDevices = false;
	
	for (RefArray< IInputDriver >::iterator i = m_drivers.begin(); i != m_drivers.end(); ++i)
	{
		IInputDriver::UpdateResult result = (*i)->update();
		if (result == IInputDriver::UrDevicesChanged)
			shouldUpdateDevices |= true;
	}
	
	if (shouldUpdateDevices)
		updateDevices();

	for (RefArray< IInputDevice >::iterator i = m_devices.begin(); i != m_devices.end(); ++i)
		(*i)->readState();

	return true;
}

void InputSystem::updateDevices()
{
	m_devices.resize(0);
	for (RefArray< IInputDriver >::iterator i = m_drivers.begin(); i != m_drivers.end(); ++i)
	{
		IInputDriver* inputDriver = *i;
		T_ASSERT (inputDriver);

		for (int j = 0; j < inputDriver->getDeviceCount(); ++j)
		{
			Ref< IInputDevice > inputDevice = inputDriver->getDevice(j);
			T_ASSERT (inputDevice);

			m_devices.push_back(inputDevice);
		}
	}
}

	}
}
