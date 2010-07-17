#include "Input/Binding/InputValue.h"

namespace traktor
{
	namespace input
	{

InputValue::InputValue(float value, float T)
:	m_value(value)
,	m_T(T)
{
}

float InputValue::get() const
{
	return m_value;
}

float InputValue::getTime() const
{
	return m_T;
}
	
	}
}
