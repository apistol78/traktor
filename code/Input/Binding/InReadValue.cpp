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

Ref< IInputNode::Instance > InReadValue::createInstance() const
{
	return 0;
}

float InReadValue::evaluate(
	Instance* instance,
	const InputValueSet& valueSet,
	float T,
	float dT
) const
{
	return valueSet.get(m_valueId);
}

void InReadValue::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"valueId", m_valueId);
}
	
	}
}
