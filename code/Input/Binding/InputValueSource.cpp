#include "Input/IInputDevice.h"
#include "Input/InputSystem.h"
#include "Input/Binding/InputValueSet.h"
#include "Input/Binding/InputValueSource.h"
#include "Input/Binding/InputValueSourceData.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.InputValueSource", InputValueSource, Object)

InputValueSource::InputValueSource(const InputValueSourceData* data)
:	m_data(data)
{
}

void InputValueSource::update(InputSystem* inputSystem, InputValueSet& outValueSet)
{
	InputCategory category = m_data->getCategory();
	InputDefaultControlType controlType = m_data->getControlType();
	int32_t index = m_data->getIndex();

	int32_t deviceCount = inputSystem->getDeviceCount(category);

	// Find all matching devices.
	if (m_deviceControls.size() != deviceCount)
	{
		m_deviceControls.clear();
		if (index < 0)
		{
			for (int32_t i = 0; i < deviceCount; ++i)
			{
				Ref< IInputDevice > device = inputSystem->getDevice(category, i, false);
				if (device)
				{
					int32_t control;
					if (device->getDefaultControl(controlType, control))
					{
						DeviceControl dc;
						dc.device = device;
						dc.control = control;
						m_deviceControls.push_back(dc);
					}
				}
			}
		}
		else
		{
			Ref< IInputDevice > device = inputSystem->getDevice(category, index, false);
			if (device)
			{
				int32_t control;
				if (device->getDefaultControl(controlType, control))
				{
					DeviceControl dc;
					dc.device = device;
					dc.control = control;
					m_deviceControls.push_back(dc);
				}
			}
		}
	}
	
	// Query all matching devices.
	float value = 0.0f;
	for (std::list< DeviceControl >::const_iterator i = m_deviceControls.begin(); i != m_deviceControls.end(); ++i)
	{
		if (!i->device->isConnected())
			continue;

		value = std::max(value, i->device->getControlValue(i->control));
	}
	
	outValueSet.set(m_data->getValueId(), InputValue(value));
}

	}
}
