#include "Input/Binding/IInputNode.h"
#include "Input/Binding/InputState.h"
#include "Input/Binding/InputStateData.h"
#include "Input/Binding/InputValueSet.h"

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

bool InputState::isDown() const
{
	return m_currentValue > 0.5f;
}

bool InputState::isUp() const
{
	return !isDown();
}

bool InputState::isPressed() const
{
	return isDown() && m_previousValue <= 0.5f;
}

bool InputState::isReleased() const
{
	return !isDown() && m_previousValue > 0.5f;
}

	}
}
