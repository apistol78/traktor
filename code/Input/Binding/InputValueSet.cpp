#include "Input/Binding/InputValueSet.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.InputValueSet", InputValueSet, Object)

void InputValueSet::set(const std::wstring& valueId, const InputValue& value)
{
	m_valueMap[valueId] = value;
}

InputValue InputValueSet::get(const std::wstring& valueId) const
{
	std::map< std::wstring, InputValue >::const_iterator i = m_valueMap.find(valueId);
	return i != m_valueMap.end() ? i->second : InputValue(0.0f);
}

	}
}
