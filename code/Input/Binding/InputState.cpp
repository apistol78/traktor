/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Input/Binding/IInputNode.h"
#include "Input/Binding/InputState.h"
#include "Input/Binding/InputStateData.h"
#include "Input/Binding/InputValueSet.h"
#include "Input/Binding/ValueDigital.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.InputState", InputState, ISerializable)

InputState::InputState()
:	m_active(false)
,	m_previousValue(0.0f)
,	m_currentValue(0.0f)
{
}

bool InputState::create(const InputStateData* data)
{
	m_data = data;
	m_instance = m_data->getSource()->createInstance();
	return true;
}

void InputState::update(const InputValueSet& valueSet, float T, float dT)
{
	m_previousValue = m_currentValue;
	
	// Evaluate source nodes to get new state value.
	m_currentValue = m_data->getSource()->evaluate(
		m_instance,
		valueSet,
		T,
		dT
	);

	// Enable boolean input as soon as both values has been released.
	if (!asBoolean(m_previousValue) && !asBoolean(m_currentValue))
		m_active = true;
}

void InputState::reset()
{
	m_active = false;
	m_previousValue =
	m_currentValue = 0.0f;
}

bool InputState::isDown() const
{
	return m_active && asBoolean(m_currentValue);
}

bool InputState::isUp() const
{
	return !m_active || !isDown();
}

bool InputState::isPressed() const
{
	return m_active && isDown() && !asBoolean(m_previousValue);
}

bool InputState::isReleased() const
{
	return m_active && !isDown() && asBoolean(m_previousValue);
}

bool InputState::hasChanged() const
{
	return m_active && (isPressed() || isReleased());
}

	}
}
