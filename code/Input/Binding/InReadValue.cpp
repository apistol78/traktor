#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Input/Binding/InReadValue.h"
#include "Input/Binding/InputValueSet.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.InReadValue", 0, InReadValue, IInputNode)

InReadValue::InReadValue()
{
}

InReadValue::InReadValue(const std::wstring& valueId)
:	m_valueId(valueId)
{
}

InputValue InReadValue::evaluate(const InputValueSet& valueSet, float T, float dT, float currentStateValue) const
{
	return valueSet.get(m_valueId);
}

bool InReadValue::serialize(ISerializer& s)
{
	return s >> Member< std::wstring >(L"valueId", m_valueId);
}
	
	}
}
