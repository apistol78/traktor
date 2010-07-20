#include "Input/Binding/IInputNode.h"
#include "Input/Binding/InputState.h"
#include "Input/Binding/InputStateData.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.InputState", InputState, ISerializable)

InputState::InputState(const InputStateData* data)
:	m_data(data)
,	m_previousValue(0.0f)
,	m_currentValue(0.0f)
{
}

void InputState::update(const InputValueSet& valueSet, float T, float dT)
{
	m_previousValue = m_currentValue;
	m_currentValue = m_data->getSource()->evaluate(valueSet, T, dT, m_currentValue).getValue();
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
