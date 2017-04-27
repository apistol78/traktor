/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cmath>
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Input/Binding/DeviceControlManager.h"
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
:	m_idleTimer(0.0f)
,	m_T(0.0f)
{
}

bool InputMapping::create(
	InputSystem* inputSystem,
	const InputMappingSourceData* sourceData,
	const InputMappingStateData* stateData
)
{
	m_deviceControlManager = new DeviceControlManager(inputSystem);

	m_sources.clear();
	m_states.clear();
	
	const std::map< std::wstring, Ref< IInputSourceData > >& sourceDataMap = sourceData->getSourceData();
	for (std::map< std::wstring, Ref< IInputSourceData > >::const_iterator i = sourceDataMap.begin(); i != sourceDataMap.end(); ++i)
	{
		if (!i->second)
			continue;
	
		Ref< IInputSource > source = i->second->createInstance(m_deviceControlManager);
		if (!source)
		{
			log::error << L"Unable to create source instance \"" << i->first << L"\"" << Endl;
			return false;
		}
		
		m_sources[getParameterHandle(i->first)] = source;
	}

	const std::map< std::wstring, Ref< InputStateData > >& stateDataMap = stateData->getStateData();
	for (std::map< std::wstring, Ref< InputStateData > >::const_iterator i = stateDataMap.begin(); i != stateDataMap.end(); ++i)
	{
		if (!i->second)
			continue;
	
		Ref< InputState > state = new InputState();
		if (!state->create(i->second))
		{
			log::error << L"Unable to create state \"" << i->first << L"\"" << Endl;
			return false;
		}
			
		m_states[getParameterHandle(i->first)] = state;
	}

	return true;
}

void InputMapping::update(float dT, bool inputEnable)
{
	m_idleTimer += dT;

	if (inputEnable)
	{
		// Update device control manager.
		m_deviceControlManager->update();

		// Update value set with state's current values.
		for (SmallMap< handle_t, Ref< InputState > >::iterator i = m_states.begin(); i != m_states.end(); ++i)
		{
			float value = i->second->getValue();
			m_valueSet.set(i->first, value);
		}
	
		// Prepare all sources for a new state.
		for (SmallMap< handle_t, Ref< IInputSource > >::iterator i = m_sources.begin(); i != m_sources.end(); ++i)
			i->second->prepare(m_T, dT);
	
		// Input value set by updating all sources.
		for (SmallMap< handle_t, Ref< IInputSource > >::iterator i = m_sources.begin(); i != m_sources.end(); ++i)
		{
			float value = i->second->read(m_T, dT);

			// Reset idle timer when value change.
			float currentValue = m_valueSet.get(i->first);
			if (std::abs(currentValue - value) > FUZZY_EPSILON)
				m_idleTimer = 0.0f;

			m_valueSet.set(i->first, value);
		}
		
		// Update states.
		for (SmallMap< handle_t, Ref< InputState > >::iterator i = m_states.begin(); i != m_states.end(); ++i)
			i->second->update(m_valueSet, m_T, dT);
		
		m_T += dT;
	}
}

void InputMapping::reset()
{
	for (SmallMap< handle_t, Ref< InputState > >::iterator i = m_states.begin(); i != m_states.end(); ++i)
		i->second->reset();
}

void InputMapping::reset(handle_t id)
{
	InputState* state = getState(id);
	if (state)
		state->reset();
}

void InputMapping::setValue(handle_t id, float value)
{
	m_valueSet.set(id, value);
}

float InputMapping::getValue(handle_t id) const
{
	return m_valueSet.get(id);
}

IInputSource* InputMapping::getSource(handle_t id) const
{
	SmallMap< handle_t, Ref< IInputSource > >::const_iterator i = m_sources.find(id);
	return i != m_sources.end() ? i->second : 0;
}

const SmallMap< handle_t, Ref< IInputSource > >& InputMapping::getSources() const
{
	return m_sources;
}

InputState* InputMapping::getState(handle_t id) const
{
	SmallMap< handle_t, Ref< InputState > >::const_iterator i = m_states.find(id);
	return i != m_states.end() ? i->second : 0;
}

const SmallMap< handle_t, Ref< InputState > >& InputMapping::getStates() const
{
	return m_states;
}

float InputMapping::getStateValue(handle_t id) const
{
	InputState* state = getState(id);
	return state ? state->getValue() : 0.0f;
}

float InputMapping::getStatePreviousValue(handle_t id) const
{
	InputState* state = getState(id);
	return state ? state->getPreviousValue() : 0.0f;
}

float InputMapping::getStateDeltaValue(handle_t id) const
{
	InputState* state = getState(id);
	return state ? state->getValue() - state->getPreviousValue() : 0.0f;
}

bool InputMapping::isStateDown(handle_t id) const
{
	InputState* state = getState(id);
	return state ? state->isDown() : false;
}

bool InputMapping::isStateUp(handle_t id) const
{
	InputState* state = getState(id);
	return state ? state->isUp() : false;
}

bool InputMapping::isStatePressed(handle_t id) const
{
	InputState* state = getState(id);
	return state ? state->isPressed() : false;
}

bool InputMapping::isStateReleased(handle_t id) const
{
	InputState* state = getState(id);
	return state ? state->isReleased() : false;
}

bool InputMapping::hasStateChanged(handle_t id) const
{
	InputState* state = getState(id);
	return state ? state->hasChanged() : false;
}

float InputMapping::getIdleDuration() const
{
	return m_idleTimer;
}

	}
}
