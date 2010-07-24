#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "Input/IInputDevice.h"
#include "Input/InputSystem.h"
#include "Input/Binding/GenericInputSource.h"
#include "Input/Binding/GenericInputSourceData.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.GenericInputSource", GenericInputSource, Object)

GenericInputSource::GenericInputSource(const GenericInputSourceData* data)
:	m_data(data)
,	m_lastValue(0.0f)
{
}

std::wstring GenericInputSource::getDescription() const
{
	// Use name of control as description; use from first connected device as it should be the same name on all.
	for (std::list< DeviceControl >::const_iterator i = m_deviceControls.begin(); i != m_deviceControls.end(); ++i)
	{
		if (i->device->isConnected())
			return i->device->getControlName(i->control);
	}

	// No device connected; return empty string.
	return L"";
}

float GenericInputSource::read(InputSystem* inputSystem, float T, float dT)
{
	InputCategory category = m_data->getCategory();
	InputDefaultControlType controlType = m_data->getControlType();
	int32_t index = m_data->getIndex();
	
	// Abort early as no device should have this control.
	if (controlType == DtInvalid)
		return 0.0f;

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
						dc.previousValue = 0.0f;
						dc.currentValue = 0.0f;
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
					dc.previousValue = 0.0f;
					dc.currentValue = 0.0f;
					m_deviceControls.push_back(dc);
				}
			}
		}
	}
	
	// Query all matching devices.
	for (std::list< DeviceControl >::iterator i = m_deviceControls.begin(); i != m_deviceControls.end(); ++i)
	{
		i->previousValue = i->currentValue;
		if (i->device->isConnected())
			i->currentValue = i->device->getControlValue(i->control);
	}

	// Return first found modified value.
	for (std::list< DeviceControl >::const_iterator i = m_deviceControls.begin(); i != m_deviceControls.end(); ++i)
	{
		if (abs< float >(i->currentValue - i->previousValue) > FUZZY_EPSILON)
		{
			m_lastValue = i->currentValue;
			return i->currentValue;
		}
	}

	// No device modified; return same value as last time.
	return m_lastValue;
}

	}
}
