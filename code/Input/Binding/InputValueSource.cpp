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
,	m_control(0)
{
}

void InputValueSource::update(InputSystem* inputSystem, InputValueSet& outValueSet)
{
	if (!m_device)
	{
		InputCategory category = m_data->getCategory();
		InputDefaultControlType controlType = m_data->getControlType();
		int32_t index = m_data->getIndex();
		
		if (index < 0)
		{
			int32_t deviceCount = inputSystem->getDeviceCount(category);
			for (int32_t i = 0; i < deviceCount; ++i)
			{
				m_device = inputSystem->getDevice(category, i, false);
				if (m_device)
					break;
			}
		}
		else
			m_device = inputSystem->getDevice(category, index, false);

		if (!m_device)
			return;
			
		m_device->getDefaultControl(controlType, m_control);
	}
	
	if (!m_device->isConnected())
		return;
	
	float value = m_device->getControlValue(m_control);
	outValueSet.set(m_data->getValueId(), InputValue(value));
}

	}
}
