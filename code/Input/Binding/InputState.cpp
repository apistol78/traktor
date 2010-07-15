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
{
}

void InputState::update(const InputValueSet& valueSet)
{
	m_previousValue = m_currentValue;
	m_currentValue = m_data->getSource()->evaluate(valueSet, m_currentValue);
}
	
	}
}
