/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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

namespace traktor::input
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.InputMapping", InputMapping, Object)

bool InputMapping::create(
	InputSystem* inputSystem,
	const InputMappingSourceData* sourceData,
	const InputMappingStateData* stateData
)
{
	m_deviceControlManager = new DeviceControlManager(inputSystem);

	m_sources.clear();
	m_states.clear();

	for (const auto& it : sourceData->getSourceData())
	{
		if (!it.second)
			continue;

		Ref< IInputSource > source = it.second->createInstance(m_deviceControlManager);
		if (!source)
		{
			log::error << L"Unable to create source instance \"" << it.first << L"\"." << Endl;
			return false;
		}

		m_sources[getParameterHandle(it.first)] = source;
	}

	for (const auto& it : stateData->getStateData())
	{
		if (!it.second)
			continue;

		Ref< InputState > state = new InputState();
		if (!state->create(it.second))
		{
			log::error << L"Unable to create state \"" << it.first << L"\"." << Endl;
			return false;
		}

		m_states[getParameterHandle(it.first)] = state;
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
		for (auto& it : m_states)
		{
			const float value = it.second->getValue();
			m_valueSet.set(it.first, value);
		}

		// Prepare all sources for a new state.
		for (auto& it : m_sources)
			it.second->prepare(m_T, dT);

		// Input value set by updating all sources.
		for (auto& it : m_sources)
		{
			const float value = it.second->read(m_T, dT);

			// Reset idle timer when value change.
			const float currentValue = m_valueSet.get(it.first);
			if (std::abs(currentValue - value) > FUZZY_EPSILON)
				m_idleTimer = 0.0f;

			m_valueSet.set(it.first, value);
		}

		// Update states.
		for (auto& it : m_states)
			it.second->update(m_valueSet, m_T, dT);

		m_T += dT;
	}
}

void InputMapping::reset()
{
	for (auto& it : m_states)
		it.second->reset();
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
	auto it = m_sources.find(id);
	return it != m_sources.end() ? it->second : nullptr;
}

const SmallMap< handle_t, Ref< IInputSource > >& InputMapping::getSources() const
{
	return m_sources;
}

InputState* InputMapping::getState(handle_t id) const
{
	auto it = m_states.find(id);
	return it != m_states.end() ? it->second : nullptr;
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
