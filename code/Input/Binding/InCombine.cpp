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

InCombine::InCombine()
{
	m_valueMul[0] = m_valueMul[1] = 0.0f;
	m_valueAdd[0] = m_valueAdd[1] = 0.0f;
}

InCombine::InCombine(
	IInputNode* source1, float mul1, float add1,
	IInputNode* source2, float mul2, float add2
)
{
	m_source[0] = source1;
	m_source[1] = source2;
	m_valueMul[0] = mul1;
	m_valueMul[1] = mul2;
	m_valueAdd[0] = add1;
	m_valueAdd[1] = add2;
}

InputValue InCombine::evaluate(const InputValueSet& valueSet, float T, float dT, float currentStateValue) const
{
	InputValue value1 = m_source[0]->evaluate(valueSet, T, dT, currentStateValue);
	InputValue value2 = m_source[1]->evaluate(valueSet, T, dT, currentStateValue);
	
	float sv1 = value1.get() * m_valueMul[0] + m_valueAdd[0];
	float sv2 = value2.get() * m_valueMul[1] + m_valueAdd[1];
	
	return InputValue(
		sv1 + sv2,
		std::min(value1.getTime(), value2.getTime())
	);
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
