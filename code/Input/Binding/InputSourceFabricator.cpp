#include "Input/IInputDevice.h"
#include "Input/InputSystem.h"
#include "Input/Binding/CombinedInputSourceData.h"
#include "Input/Binding/GenericInputSourceData.h"
#include "Input/Binding/InputSourceFabricator.h"
#include "Input/Binding/ValueDigital.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.InputSourceFabricator", InputSourceFabricator, Object)

InputSourceFabricator::InputSourceFabricator(InputSystem* inputSystem, InputCategory category, bool analogue)
:	m_category(category)
,	m_analogue(analogue)
{
	int32_t deviceCount = inputSystem->getDeviceCount(m_category);
	for (int32_t i = 0; i < deviceCount; ++i)
	{
		Ref< IInputDevice > device = inputSystem->getDevice(m_category, i, false);
		if (device)
		{
			DeviceState ds;
			ds.device = device;
			m_deviceStates.push_back(ds);
		}
	}
}

Ref< IInputSourceData > InputSourceFabricator::update()
{
	// Have we already finished fabricating?
	if (m_outputData)
		return m_outputData;
		
	// Enumerate all devices and controls to see if anything has changed.
	for (std::list< DeviceState >::iterator i = m_deviceStates.begin(); i != m_deviceStates.end(); ++i)
	{
		for (int32_t j = DtUp; j <= DtKeyLastIndex; ++j)
		{
			InputDefaultControlType controlType = (InputDefaultControlType)j;
		
			int32_t control;
			if (!i->device->getDefaultControl(controlType, control))
				continue;
			
			if (i->device->isControlAnalogue(control) != m_analogue)
				continue;
			
			float value = i->device->getControlValue(control);
			if (value != i->values[controlType])
			{
				// Control has been modified; record it's use.
				if (m_analogue)
				{
					// Analogue control cannot be combined, thus create generic
					// source and finish.
					m_outputData = new GenericInputSourceData(
						m_category,
						std::distance(m_deviceStates.begin(), i),
						controlType
					);
					break;
				}
				else
				{
					// Digital controls are chained until any control are
					// released and then we're finished.
					if (asBoolean(value))
					{
						if (!m_combinedData)
							m_combinedData = new CombinedInputSourceData();

						m_combinedData->addSource(new GenericInputSourceData(
							m_category,
							std::distance(m_deviceStates.begin(), i),
							controlType
						));
					}
					else
					{
						m_outputData = m_combinedData;
						break;
					}
				}
				i->values[controlType] = value;
			}
		}
		if (m_outputData)
			break;
	}
	
	return m_outputData;
}

	}
}
