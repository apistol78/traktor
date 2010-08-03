#include "Input/Binding/IInputSource.h"
#include "Input/Binding/IInputSourceData.h"
#include "Input/Binding/InputMapping.h"
#include "Input/Binding/InputMappingSourceData.h"
#include "Input/Binding/InputMappingStateData.h"
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

bool InputMapping::create(
	const InputMappingSourceData* sourceData,
	const InputMappingStateData* stateData
)
{
	const std::map< std::wstring, Ref< IInputSourceData > >& sourceDataMap = sourceData->getSourceData();
	for (std::map< std::wstring, Ref< IInputSourceData > >::const_iterator i = sourceDataMap.begin(); i != sourceDataMap.end(); ++i)
	{
		Ref< IInputSource > source = i->second->createInstance();
		if (!source)
			return false;

		m_sources.insert(std::make_pair(i->first, source));
	}

	const std::map< std::wstring, Ref< InputStateData > >& stateDataMap = stateData->getStateData();
	for (std::map< std::wstring, Ref< InputStateData > >::const_iterator i = stateDataMap.begin(); i != stateDataMap.end(); ++i)
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

float InputMapping::getStateValue(const std::wstring& id) const
{
	InputState* state = getState(id);
	return state ? state->getValue() : 0.0f;
}

float InputMapping::getStatePreviousValue(const std::wstring& id) const
{
	InputState* state = getState(id);
	return state ? state->getPreviousValue() : 0.0f;
}

bool InputMapping::isStateDown(const std::wstring& id) const
{
	InputState* state = getState(id);
	return state ? state->isDown() : false;
}

bool InputMapping::isStateUp(const std::wstring& id) const
{
	InputState* state = getState(id);
	return state ? state->isUp() : false;
}

bool InputMapping::isStatePressed(const std::wstring& id) const
{
	InputState* state = getState(id);
	return state ? state->isPressed() : false;
}

bool InputMapping::isStateReleased(const std::wstring& id) const
{
	InputState* state = getState(id);
	return state ? state->isReleased() : false;
}

bool InputMapping::hasStateChanged(const std::wstring& id) const
{
	InputState* state = getState(id);
	return state ? state->hasChanged() : false;
}

	}
}
