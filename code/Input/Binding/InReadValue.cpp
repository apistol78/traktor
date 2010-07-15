#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Input/Binding/InReadValue.h"
#include "Input/Binding/InputValueSet.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.InReadValue", 0, InReadValue, IInputNode)

float InReadValue::evaluate(const InputValueSet& valueSet, float currentStateValue) const
{
	return valueSet.get(m_valueId).get();
}

bool InReadValue::serialize(ISerializer& s)
{
	return s >> Member< std::wstring >(L"valueId", m_valueId);
}
	
	}
}
