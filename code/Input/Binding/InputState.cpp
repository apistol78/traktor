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
:	m_previousValue(0.0f)
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
}

void InputState::reset()
{
	m_previousValue =
	m_currentValue = 0.0f;
}

bool InputState::isDown() const
{
	return asBoolean(m_currentValue);
}

bool InputState::isUp() const
{
	return !isDown();
}

bool InputState::isPressed() const
{
	return isDown() && !asBoolean(m_previousValue);
}

bool InputState::isReleased() const
{
	return !isDown() && asBoolean(m_previousValue);
}

	}
}
