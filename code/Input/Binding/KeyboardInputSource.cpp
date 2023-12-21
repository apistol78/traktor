/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include <list>
#include "Core/Io/StringOutputStream.h"
#include "Input/IInputDevice.h"
#include "Input/InputSystem.h"
#include "Input/Binding/DeviceControl.h"
#include "Input/Binding/DeviceControlManager.h"
#include "Input/Binding/KeyboardInputSource.h"
#include "Input/Binding/ValueDigital.h"

namespace traktor::input
{
	namespace
	{

class KeyboardRegistry
{
public:
	static KeyboardRegistry& getInstance()
	{
		static KeyboardRegistry s_instance;
		return s_instance;
	}

	void add(KeyboardInputSource* source)
	{
		m_sources.push_back(source);
	}

	void remove(KeyboardInputSource* source)
	{
		auto it = std::find(m_sources.begin(), m_sources.end(), source);
		m_sources.erase(it);
	}

	const std::list< KeyboardInputSource* >& get() const
	{
		return m_sources;
	}

private:
	std::list< KeyboardInputSource* > m_sources;
};

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.KeyboardInputSource", KeyboardInputSource, IInputSource)

KeyboardInputSource::KeyboardInputSource(
	const AlignedVector< DefaultControl >& controlTypes,
	DeviceControlManager* deviceControlManager
)
:	m_deviceControlManager(deviceControlManager)
,	m_deviceCount(0)
,	m_state(false)
{
	for (auto controlType : controlTypes)
	{
		Key key;
		key.controlType = controlType;
		key.state = false;
		m_keys.push_back(key);
	}
	KeyboardRegistry::getInstance().add(this);
}

KeyboardInputSource::~KeyboardInputSource()
{
	KeyboardRegistry::getInstance().remove(this);
}

std::wstring KeyboardInputSource::getDescription() const
{
	StringOutputStream ss;
	for (auto i = m_keys.begin(); i != m_keys.end(); ++i)
	{
		if (i != m_keys.begin())
			ss << L" + ";

		for (auto j = i->deviceControls.begin(); j != i->deviceControls.end(); ++j)
		{
			std::wstring controlName = (*j)->getControlName();
			if (!controlName.empty())
			{
				ss << controlName;
				break;
			}
		}
	}
	return ss.str();
}

void KeyboardInputSource::prepare(float T, float dT)
{
	// Check if number of connected keyboard devices has changed since last preparation.
	const int32_t deviceCount = m_deviceControlManager->getDeviceControlCount(InputCategory::Keyboard);
	if (deviceCount != m_deviceCount)
	{
		for (auto i = m_keys.begin(); i != m_keys.end(); ++i)
		{
			i->deviceControls.resize(deviceCount);
			for (int32_t j = 0; j < deviceCount; ++j)
			{
				i->deviceControls[j] = m_deviceControlManager->getDeviceControl(
					InputCategory::Keyboard,
					i->controlType,
					false,
					j
				);
			}
		}
		m_deviceCount = deviceCount;
	}

	// Update our local state; ie state without looking at other keyboard sources.
	m_state = true;
	for (auto i = m_keys.begin(); i != m_keys.end(); ++i)
	{
		i->state = false;
		for (auto j = i->deviceControls.begin(); j != i->deviceControls.end(); ++j)
			i->state |= asBoolean(j->getCurrentValue());
		m_state &= i->state;
	}
}

float KeyboardInputSource::read(float T, float dT)
{
	if (!m_state)
		return 0.0f;

	// Our chain of controls are all active, ie all pressed.

	// If our controls are part of any other keyboard input source's chain
	// then we need to ensure our chain of controls are longest.

	const std::list< KeyboardInputSource* >& otherSources = KeyboardRegistry::getInstance().get();
	for (std::list< KeyboardInputSource* >::const_iterator i = otherSources.begin(); i != otherSources.end(); ++i)
	{
		if ((*i)->m_keys.size() <= m_keys.size())
			continue;

		const bool otherActive = asBoolean((*i)->read(T, dT));
		if (!otherActive)
			continue;

		bool match = true;
		for (auto j = m_keys.begin(); j != m_keys.end(); ++j)
		{
			if (std::find((*i)->m_keys.begin(), (*i)->m_keys.end(), *j) == (*i)->m_keys.end())
			{
				match = false;
				break;
			}
		}

		// Found other source which are active with all our our keys mapped; we
		// cannot be as it contain more keys thus have higher priority.
		if (match)
		{
			m_state = false;
			break;
		}
	}

	return asFloat(m_state);
}

bool KeyboardInputSource::Key::operator == (const Key& rh) const
{
	return controlType == rh.controlType;
}

}
