/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Input/IInputDevice.h"
#include "Input/InputSystem.h"
#include "Input/Binding/CombinedInputSourceData.h"
#include "Input/Binding/GenericInputSourceData.h"
#include "Input/Binding/InputSourceFabricator.h"
#include "Input/Binding/ValueDigital.h"

namespace traktor::input
{
	namespace
	{

const float c_valueThreshold = 0.5f;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.InputSourceFabricator", InputSourceFabricator, Object)

InputSourceFabricator::InputSourceFabricator(InputSystem* inputSystem, InputCategory category, bool analogue)
:	m_analogue(analogue)
{
	const int32_t deviceCount = inputSystem->getDeviceCount();
	for (int32_t i = 0; i < deviceCount; ++i)
	{
		Ref< IInputDevice > device = inputSystem->getDevice(i);
		if ((device->getCategory() & category) == InputCategory::Invalid || !device->isConnected())
			continue;

		DeviceState ds;
		ds.device = device;
		ds.index = -1;

		// Name the device only when there is more than one of its category to choose from.
		if (inputSystem->getDeviceCount(device->getCategory(), true) > 1)
		{
			ds.index = 0;
			for (const auto& other : m_deviceStates)
			{
				if (other.device->getCategory() == device->getCategory())
					++ds.index;
			}
		}

		m_deviceStates.push_back(ds);
	}
}

Ref< IInputSourceData > InputSourceFabricator::update()
{
	float min, max;

	// Have we already finished fabricating?
	if (m_outputData)
		return m_outputData;

	// Enumerate all devices and controls to see if anything has changed.
	for (std::list< DeviceState >::iterator i = m_deviceStates.begin(); i != m_deviceStates.end(); ++i)
	{
		for (int32_t j = (int32_t)DefaultControl::Up; j <= (int32_t)DefaultControl::KeyLastIndex; ++j)
		{
			DefaultControl controlType = (DefaultControl)j;

			int32_t control;
			if (!i->device->getDefaultControl(controlType, m_analogue, control))
				continue;

			float value = i->device->getControlValue(control);
			bool first = i->values.find(controlType) == i->values.end();

			// Wait until digital control has been released before we
			// start tracking it.
			if (
				!m_analogue &&
				asBoolean(value) &&
				first
			)
				continue;

			// Determine motion threshold from control range.
			float threshold = c_valueThreshold;
			if (i->device->getControlRange(control, min, max))
				threshold = (max - min) * 0.1f;

			const float dV = value - i->values[controlType];
			if (std::abs(dV) > threshold)
			{
				if (!first)
				{
					// Control has been modified; record it's use.
					if (m_analogue)
					{
						// Analogue control cannot be combined, thus create generic
						// source and finish.
						m_outputData = new GenericInputSourceData(
							i->device->getCategory(),
							i->index,
							controlType,
							m_analogue,
							false		//<! \fixme
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
								m_combinedData = new CombinedInputSourceData(CombinedInputSource::CmAll);

							m_combinedData->addSource(new GenericInputSourceData(
								i->device->getCategory(),
								i->index,
								controlType,
								m_analogue,
								false		//<! \fixme
							));
						}
						else
						{
							// Remove combined wrapper if it only contain one source.
							const RefArray< IInputSourceData >& sources = m_combinedData->getSources();
							if (sources.size() > 1)
								m_outputData = m_combinedData;
							else if (sources.size() == 1)
								m_outputData = sources[0];

							if (m_outputData)
								break;
						}
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
