#include "Input/Binding/InputValue.h"

namespace traktor
{
	namespace input
	{

InputValue::InputValue(float value)
:	m_value(value)
{
}

void InputValue::set(float value)
{
	m_value = value;
}

float InputValue::get() const
{
	return m_value;
}
	
	}
}
