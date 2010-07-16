#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "Input/Binding/InCombine.h"
#include "Input/Binding/InputValueSet.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.InCombine", 0, InCombine, IInputNode)

float InCombine::evaluate(const InputValueSet& valueSet, float currentStateValue) const
{
	float value1 = m_source[0]->evaluate(valueSet, currentStateValue);
	float value2 = m_source[1]->evaluate(valueSet, currentStateValue);
	
	value1 = value1 * m_valueMul[0] + m_valueAdd[0];
	value2 = value2 * m_valueMul[1] + m_valueAdd[1];
	
	return value1 + value2;
}

bool InCombine::serialize(ISerializer& s)
{
	s >> MemberStaticArray< Ref< IInputNode >, 2, MemberRef< IInputNode > >(L"source", m_source);
	s >> MemberStaticArray< float, 2 >(L"valueMul", m_valueMul);
	s >> MemberStaticArray< float, 2 >(L"valueAdd", m_valueAdd);
	return true;
}
	
	}
}
