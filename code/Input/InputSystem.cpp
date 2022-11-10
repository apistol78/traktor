/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Timer/Profiler.h"
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
	auto it = std::find(m_drivers.begin(), m_drivers.end(), inputDriver);
	if (it != m_drivers.end())
	{
		m_drivers.erase(it);
		updateDevices();
	}
}

void InputSystem::addDevice(IInputDevice* inputDevice)
{
	m_devices.push_back(inputDevice);
}

void InputSystem::removeDevice(IInputDevice* inputDevice)
{
	auto it = std::find(m_devices.begin(), m_devices.end(), inputDevice);
	if (it != m_devices.end())
		m_devices.erase(it);
}

int32_t InputSystem::getDeviceCount() const
{
	return int32_t(m_devices.size());
}

IInputDevice* InputSystem::getDevice(int32_t index)
{
	T_ASSERT(index >= 0 && index < getDeviceCount());
	return m_devices[index];
}

int32_t InputSystem::getDeviceCount(InputCategory category, bool connected) const
{
	int32_t deviceCount = 0;
	for (auto device : m_devices)
	{
		if (device->getCategory() == category && (!connected || device->isConnected()))
			++deviceCount;
	}
	return deviceCount;
}

IInputDevice* InputSystem::getDevice(InputCategory category, int32_t index, bool connected)
{
	for (auto device : m_devices)
	{
		if (device->getCategory() == category && (!connected || device->isConnected()))
		{
			if (index-- <= 0)
				return device;
		}
	}
	return nullptr;
}

void InputSystem::setExclusive(bool exclusive)
{
	for (auto device : m_devices)
		device->setExclusive(exclusive);
}

bool InputSystem::update()
{
	T_PROFILER_SCOPE(L"InputSystem update");

	// Update drivers.
	bool shouldUpdateDevices = false;
	for (auto driver : m_drivers)
	{
		IInputDriver::UpdateResult result = driver->update();
		if (result == IInputDriver::UrDevicesChanged)
			shouldUpdateDevices |= true;
	}
	if (shouldUpdateDevices)
		updateDevices();

	// Read state of all ready devices.
	for (auto device : m_devices)
		device->readState();

	return true;
}

void InputSystem::updateDevices()
{
	T_PROFILER_SCOPE(L"InputSystem updateDevices");

	m_devices.resize(0);
	for (auto driver : m_drivers)
	{
		for (int32_t i = 0; i < driver->getDeviceCount(); ++i)
		{
			Ref< IInputDevice > inputDevice = driver->getDevice(i);
			T_ASSERT(inputDevice);

			m_devices.push_back(inputDevice);
		}
	}
}

	}
}
