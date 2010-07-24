#include "Input/Binding/IInputSource.h"
#include "Input/Binding/IInputSourceData.h"
#include "Input/Binding/InputMapping.h"
#include "Input/Binding/InputMappingData.h"
#include "Input/Binding/InputState.h"
#include "Input/Binding/InputStateData.h"

namespace traktor
{
	namespace input
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.input.InputMapping", InputMapping, Object)

InputMapping::InputMapping()
:	m_T(0.0f)
{
}

bool InputMapping::create(const InputMappingData* data)
{
	const std::map< std::wstring, Ref< IInputSourceData > >& sourceData = data->getSourceData();
	for (std::map< std::wstring, Ref< IInputSourceData > >::const_iterator i = sourceData.begin(); i != sourceData.end(); ++i)
	{
		Ref< IInputSource > source = i->second->createInstance();
		if (!source)
			return false;

		m_sources.insert(std::make_pair(i->first, source));
	}

	const std::map< std::wstring, Ref< InputStateData > >& stateData = data->getStateData();
	for (std::map< std::wstring, Ref< InputStateData > >::const_iterator i = stateData.begin(); i != stateData.end(); ++i)
	{
		Ref< InputState > state = new InputState();

		if (!state->create(i->second))
			return false;
			
		m_states.insert(std::make_pair(i->first, state));
	}
	
	return true;
}

void InputMapping::update(InputSystem* InputSystem, float dT)
{
	// Update value set with state's current values.
	for (std::map< std::wstring, Ref< InputState > >::iterator i = m_states.begin(); i != m_states.end(); ++i)
	{
		float value = i->second->getValue();
		m_valueSet.set(i->first, value);
	}
	
	// Input value set by updating all sources.
	for (std::map< std::wstring, Ref< IInputSource > >::iterator i = m_sources.begin(); i != m_sources.end(); ++i)
	{
		float value = i->second->read(InputSystem, m_T, dT);
		m_valueSet.set(i->first, value);
	}
		
	// Update states.
	for (std::map< std::wstring, Ref< InputState > >::iterator i = m_states.begin(); i != m_states.end(); ++i)
		i->second->update(m_valueSet, m_T, dT);
		
	m_T += dT;
}

void InputMapping::reset()
{
	for (std::map< std::wstring, Ref< InputState > >::iterator i = m_states.begin(); i != m_states.end(); ++i)
		i->second->reset();
}

void InputMapping::setValue(const std::wstring& id, float value)
{
	m_valueSet.set(id, value);
}

float InputMapping::getValue(const std::wstring& id) const
{
	return m_valueSet.get(id);
}

IInputSource* InputMapping::getSource(const std::wstring& id) const
{
	std::map< std::wstring, Ref< IInputSource > >::const_iterator i = m_sources.find(id);
	return i != m_sources.end() ? i->second : 0;
}

const std::map< std::wstring, Ref< IInputSource > >& InputMapping::getSources() const
{
	return m_sources;
}

InputState* InputMapping::getState(const std::wstring& id) const
{
	std::map< std::wstring, Ref< InputState > >::const_iterator i = m_states.find(id);
	return i != m_states.end() ? i->second : 0;
}

const std::map< std::wstring, Ref< InputState > >& InputMapping::getStates() const
{
	return m_states;
}

	}
}
