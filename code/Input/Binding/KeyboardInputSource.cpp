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

class DeviceControlPred
{
public:
	DeviceControlPred(const DeviceControl* dc)
	:	m_dc(dc)
	{
	}
	
	bool operator () (const DeviceControl* dc) const
	{
		if (dc->getDevice() != m_dc->getDevice())
			return false;
		else if (dc->getControlType() != m_dc->getControlType())
			return false;
		else
			return true;
	}
	
private:
	const DeviceControl* m_dc;
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.KeyboardInputSource", KeyboardInputSource, Object)

KeyboardInputSource::KeyboardInputSource(
	const std::vector< InputDefaultControlType >& controlTypes,
	DeviceControlManager* deviceControlManager
)
{
	for (std::vector< InputDefaultControlType >::const_iterator i = controlTypes.begin(); i != controlTypes.end(); ++i)
	{
		m_deviceControls.push_back(deviceControlManager->getDeviceControl(
			CtKeyboard,
			*i,
			false,
			0
		));
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
	for (RefArray< DeviceControl >::const_iterator i = m_deviceControls.begin(); i != m_deviceControls.end(); ++i)
	{
		if (i != m_deviceControls.begin())
			ss << L" + ";

		std::wstring controlName = i->getControlName();
		ss << controlName;
	}
	return ss.str();
}

float KeyboardInputSource::read(float T, float dT)
{
	if (m_deviceControls.empty())
		return 0.0f;
		
	for (RefArray< DeviceControl >::const_iterator i = m_deviceControls.begin(); i != m_deviceControls.end(); ++i)
	{
		if (!asBoolean((*i)->getCurrentValue()))
			return 0.0f;
	}
	
	// Our chain of controls are all active, ie all pressed.

	// If our controls are part of any other keyboard input source's chain
	// then we need to ensure our chain of controls are longest.
	
	const std::list< KeyboardInputSource* >& otherSources = KeyboardRegistry::getInstance().get();
	for (std::list< KeyboardInputSource* >::const_iterator i = otherSources.begin(); i != otherSources.end(); ++i)
	{
		if ((*i)->m_deviceControls.size() <= m_deviceControls.size())
			continue;
		
		bool match = true;
	
		for (RefArray< DeviceControl >::const_iterator j = m_deviceControls.begin(); j != m_deviceControls.end(); ++j)
		{
			if (std::find_if((*i)->m_deviceControls.begin(), (*i)->m_deviceControls.end(), DeviceControlPred(*j)) == (*i)->m_deviceControls.end())
			{
				match = false;
				break;
			}
		}
		
		if (match)
		{
			// Found other source with all our our keys mapped; if other source also
			// active then we cannot be.
			
			for (RefArray< DeviceControl >::const_iterator j = (*i)->m_deviceControls.begin(); j != (*i)->m_deviceControls.end(); ++j)
			{
				if (!asBoolean((*j)->getCurrentValue()))
				{
					match = false;
					break;
				}
			}
			
			if (match)
				return 0.0f;
		}
	}
	

	return 1.0f;
}

	}
}
