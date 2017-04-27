/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
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

namespace traktor
{
	namespace input
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
		std::list< KeyboardInputSource* >::iterator i = std::find(m_sources.begin(), m_sources.end(), source);
		m_sources.erase(i);
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
	const std::vector< InputDefaultControlType >& controlTypes,
	DeviceControlManager* deviceControlManager
)
:	m_deviceControlManager(deviceControlManager)
,	m_deviceCount(0)
,	m_state(false)
{
	for (std::vector< InputDefaultControlType >::const_iterator i = controlTypes.begin(); i != controlTypes.end(); ++i)
	{
		Key key;
		key.controlType = *i;
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
	for (std::vector< Key >::const_iterator i = m_keys.begin(); i != m_keys.end(); ++i)
	{
		if (i != m_keys.begin())
			ss << L" + ";
			
		for (RefArray< DeviceControl >::const_iterator j = i->deviceControls.begin(); j != i->deviceControls.end(); ++j)
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
	int32_t deviceCount = m_deviceControlManager->getDeviceControlCount(CtKeyboard);
	if (deviceCount != m_deviceCount)
	{
		for (std::vector< Key >::iterator i = m_keys.begin(); i != m_keys.end(); ++i)
		{
			i->deviceControls.resize(deviceCount);
			for (int32_t j = 0; j < deviceCount; ++j)
			{			
				i->deviceControls[j] = m_deviceControlManager->getDeviceControl(
					CtKeyboard,
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
	for (std::vector< Key >::iterator i = m_keys.begin(); i != m_keys.end(); ++i)
	{
		i->state = false;
		for (RefArray< DeviceControl >::const_iterator j = i->deviceControls.begin(); j != i->deviceControls.end(); ++j)
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
			
		bool otherActive = asBoolean((*i)->read(T, dT));
		if (!otherActive)
			continue;
		
		bool match = true;
		for (std::vector< Key >::iterator j = m_keys.begin(); j != m_keys.end(); ++j)
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
}
