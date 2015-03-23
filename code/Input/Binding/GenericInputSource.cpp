#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "Input/IInputDevice.h"
#include "Input/InputSystem.h"
#include "Input/Binding/DeviceControl.h"
#include "Input/Binding/DeviceControlManager.h"
#include "Input/Binding/GenericInputSource.h"
#include "Input/Binding/GenericInputSourceData.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.GenericInputSource", GenericInputSource, Object)

GenericInputSource::GenericInputSource(const GenericInputSourceData* data, DeviceControlManager* deviceControlManager)
:	m_data(data)
,	m_deviceControlManager(deviceControlManager)
,	m_matchingDeviceCount(0)
,	m_lastValue(0.0f)
{
}

std::wstring GenericInputSource::getDescription() const
{
	// Use name of control as description; use first valid name as it should be the same on all our devices.
	for (RefArray< DeviceControl >::const_iterator i = m_deviceControls.begin(); i != m_deviceControls.end(); ++i)
	{
		std::wstring controlName = i->getControlName();
		if (!controlName.empty())
			return controlName;
	}

	// No device connected; return empty string.
	return L"";
}

void GenericInputSource::prepare(float T, float dT)
{
}

float GenericInputSource::read(float T, float dT)
{
	InputCategory category = m_data->getCategory();
	InputDefaultControlType controlType = m_data->getControlType();
	bool analogue = m_data->isAnalogue();
	int32_t index = m_data->getIndex();
	
	// Abort early as no device should have this control.
	if (controlType == DtInvalid)
		return 0.0f;

	int32_t deviceCount = m_deviceControlManager->getDeviceControlCount(category);

	// Find all matching devices.
	if (deviceCount != m_matchingDeviceCount)
	{
		m_deviceControls.clear();
		if (index < 0)
		{
			for (int32_t i = 0; i < deviceCount; ++i)
				m_deviceControls.push_back(m_deviceControlManager->getDeviceControl(
					category,
					controlType,
					analogue,
					i
				));
		}
		else
		{
			m_deviceControls.push_back(m_deviceControlManager->getDeviceControl(
				category,
				controlType,
				analogue,
				index
			));
		}
		m_matchingDeviceCount = deviceCount;
	}

	if (!m_deviceControls.empty())
	{
		// Return first found modified value.
		for (RefArray< DeviceControl >::const_iterator i = m_deviceControls.begin(); i != m_deviceControls.end(); ++i)
		{
			float previousValue = (*i)->getPreviousValue();
			float currentValue = (*i)->getCurrentValue();

			if (abs< float >(currentValue - previousValue) > FUZZY_EPSILON)
			{
				// Normalize input value if desired.
				if (m_data->normalize())
				{
					float rangeMin = (*i)->getRangeMin();
					float rangeMax = (*i)->getRangeMax();
					if (abs(rangeMax - rangeMin) > FUZZY_EPSILON)
						currentValue = (currentValue - rangeMin) / (rangeMax - rangeMin);
				}
				m_lastValue = currentValue;
				break;
			}
		}
	}
	else
	{
		// No control available; ensure we reset our state as control might have been disconnected.
		m_lastValue = 0.0f;
	}

	return m_lastValue;
}

	}
}
