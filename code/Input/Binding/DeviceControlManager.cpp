#include "Input/IInputDevice.h"
#include "Input/InputSystem.h"
#include "Input/Binding/DeviceControl.h"
#include "Input/Binding/DeviceControlManager.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.DeviceControlManager", DeviceControlManager, Object)

DeviceControlManager::DeviceControlManager(InputSystem* inputSystem)
:	m_inputSystem(inputSystem)
{
}

int32_t DeviceControlManager::getDeviceControlCount(InputCategory category)
{
	return m_inputSystem->getDeviceCount(category);
}

Ref< DeviceControl > DeviceControlManager::getDeviceControl(InputCategory category, InputDefaultControlType controlType, int32_t index)
{
	for (RefArray< DeviceControl >::iterator i = m_deviceControls.begin(); i != m_deviceControls.end(); ++i)
	{
		DeviceControl* deviceControl = *i;
		if (
			deviceControl->m_category == category &&
			deviceControl->m_controlType == controlType &&
			deviceControl->m_index == index
		)
			return deviceControl;
	}

	Ref< DeviceControl > deviceControl = new DeviceControl();
	deviceControl->m_category = category;
	deviceControl->m_controlType = controlType;
	deviceControl->m_index = index;

	m_deviceControls.push_back(deviceControl);
	return deviceControl;
}

void DeviceControlManager::update()
{
	for (RefArray< DeviceControl >::iterator i = m_deviceControls.begin(); i != m_deviceControls.end(); ++i)
	{
		DeviceControl* deviceControl = *i;

		// Resolve device if unbound.
		if (!deviceControl->m_device)
		{
			Ref< IInputDevice > device = m_inputSystem->getDevice(deviceControl->m_category, deviceControl->m_index, false);
			if (device)
			{
				int32_t control;
				if (device->getDefaultControl(deviceControl->m_controlType, control))
				{
					deviceControl->m_device = device;
					deviceControl->m_control = control;
					deviceControl->m_previousValue = 0.0f;
					deviceControl->m_currentValue = 0.0f;
				}
			}
		}

		// Read control value.
		deviceControl->m_previousValue = deviceControl->m_currentValue;
		if (deviceControl->m_device)
		{
			if (deviceControl->m_device->isConnected())
				deviceControl->m_currentValue = deviceControl->m_device->getControlValue(deviceControl->m_control);
			else
				deviceControl->m_currentValue = 0.0f;
		}
		else
			deviceControl->m_currentValue = 0.0f;
	}
}

	}
}
