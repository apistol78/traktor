#include "Input/Binding/InputValueSet.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.InputValueSet", InputValueSet, Object)

void InputValueSet::set(const std::wstring& valueId, float value)
{
	m_valueMap[valueId] = value;
}

float InputValueSet::get(const std::wstring& valueId) const
{
	std::map< std::wstring, float >::const_iterator i = m_valueMap.find(valueId);
	return i != m_valueMap.end() ? i->second : 0.0f;
}

	}
}
