/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Input/IInputDevice.h"
#include "Input/InputSystem.h"
#include "Input/Binding/DeviceControl.h"
#include "Input/Binding/DeviceControlManager.h"

namespace traktor::input
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.DeviceControlManager", DeviceControlManager, Object)

DeviceControlManager::DeviceControlManager(InputSystem* inputSystem)
:	m_inputSystem(inputSystem)
{
}

int32_t DeviceControlManager::getDeviceControlCount(InputCategory category)
{
	return m_inputSystem->getDeviceCount(category, true);
}

Ref< DeviceControl > DeviceControlManager::getDeviceControl(InputCategory category, DefaultControl controlType, bool analogue, int32_t index)
{
	for (auto deviceControl : m_deviceControls)
	{
		if (
			deviceControl->m_category == category &&
			deviceControl->m_controlType == controlType &&
			deviceControl->m_analogue == analogue &&
			deviceControl->m_index == index
		)
			return deviceControl;
	}

	Ref< DeviceControl > deviceControl = new DeviceControl();
	deviceControl->m_category = category;
	deviceControl->m_controlType = controlType;
	deviceControl->m_analogue = analogue;
	deviceControl->m_index = index;

	Ref< IInputDevice > device = m_inputSystem->getDevice(deviceControl->m_category, deviceControl->m_index, true);
	if (device)
	{
		int32_t control;
		if (device->getDefaultControl(deviceControl->m_controlType, deviceControl->m_analogue, control))
		{
			deviceControl->m_device = device;
			deviceControl->m_control = control;
		}
	}

	m_deviceControls.push_back(deviceControl);
	return deviceControl;
}

void DeviceControlManager::update()
{
	for (auto deviceControl : m_deviceControls)
	{
		if (!deviceControl->m_device)
		{
			deviceControl->m_device = m_inputSystem->getDevice(deviceControl->m_category, deviceControl->m_index, true);
			if (deviceControl->m_device)
			{
				int32_t control;
				if (deviceControl->m_device->getDefaultControl(deviceControl->m_controlType, deviceControl->m_analogue, control))
					deviceControl->m_control = control;
				else
					deviceControl->m_device = nullptr;
			}
		}

		deviceControl->m_previousValue = deviceControl->m_currentValue;
		if (deviceControl->m_device)
		{
			if (deviceControl->m_device->isConnected())
			{
				// Keep updating range as it might change due to screen resize etc.
				float rangeMin, rangeMax;
				if (deviceControl->m_device->getControlRange(deviceControl->m_control, rangeMin, rangeMax))
				{
					deviceControl->m_rangeMin = rangeMin;
					deviceControl->m_rangeMax = rangeMax;
				}
				else
				{
					deviceControl->m_rangeMin = 0.0f;
					deviceControl->m_rangeMax = 0.0f;
				}

				// Read value of control from device.
				deviceControl->m_currentValue = deviceControl->m_device->getControlValue(deviceControl->m_control);
			}
			else
				deviceControl->m_currentValue = 0.0f;
		}
		else
			deviceControl->m_currentValue = 0.0f;
	}
}

}
