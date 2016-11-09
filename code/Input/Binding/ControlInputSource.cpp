#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "Input/IInputDevice.h"
#include "Input/InputSystem.h"
#include "Input/Binding/DeviceControl.h"
#include "Input/Binding/DeviceControlManager.h"
#include "Input/Binding/ControlInputSource.h"
#include "Input/Binding/ControlInputSourceData.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.ControlInputSource", ControlInputSource, IInputSource)

ControlInputSource::ControlInputSource(const ControlInputSourceData* data, DeviceControlManager* deviceControlManager)
:	m_data(data)
,	m_deviceControlManager(deviceControlManager)
,	m_matchingDeviceCount(0)
{
}

std::wstring ControlInputSource::getDescription() const
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

void ControlInputSource::prepare(float T, float dT)
{
}

float ControlInputSource::read(float T, float dT)
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

	if (m_data->getControlQuery() == ControlInputSourceData::CqConnectedDevice)
	{
		// Check if any matching control is connected.
		for (RefArray< DeviceControl >::const_iterator i = m_deviceControls.begin(); i != m_deviceControls.end(); ++i)
		{
			if ((*i)->getDevice()->isConnected())
				return 1.0f;
		}
	}
	else if (m_data->getControlQuery() == ControlInputSourceData::CqMatchingDevice)
	{
		// As long as we have found any device then this satisfy the query.
		if (!m_deviceControls.empty())
			return 1.0f;
	}

	// No connected matching controls.
	return 0.0f;
}

	}
}
