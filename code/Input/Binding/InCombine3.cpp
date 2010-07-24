#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "Input/Binding/InCombine3.h"
#include "Input/Binding/InputValueSet.h"

namespace traktor
{
	namespace input
	{
		namespace
		{

struct InCombine3Instance : public RefCountImpl< IInputNode::Instance >
{
	Ref< IInputNode::Instance > sourceInstance[3];
};
		
		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.InCombine3", 0, InCombine3, IInputNode)

InCombine3::InCombine3()
{
	m_valueMul[0] = m_valueMul[1] = m_valueMul[2] = 0.0f;
	m_valueAdd[0] = m_valueAdd[1] = m_valueAdd[2] = .0f;
}

InCombine3::InCombine3(
	IInputNode* source1, float mul1, float add1,
	IInputNode* source2, float mul2, float add2,
	IInputNode* source3, float mul3, float add3
)
{
	m_source[0] = source1;
	m_source[1] = source2;
	m_source[2] = source3;
	m_valueMul[0] = mul1;
	m_valueMul[1] = mul2;
	m_valueMul[2] = mul3;
	m_valueAdd[0] = add1;
	m_valueAdd[1] = add2;
	m_valueAdd[2] = add3;
}

Ref< IInputNode::Instance > InCombine3::createInstance() const
{
	Ref< InCombine3Instance > instance = new InCombine3Instance();
	instance->sourceInstance[0] = m_source[0] ? m_source[0]->createInstance() : 0;
	instance->sourceInstance[1] = m_source[1] ? m_source[1]->createInstance() : 0;
	instance->sourceInstance[2] = m_source[2] ? m_source[2]->createInstance() : 0;
	return instance;
}

float InCombine3::evaluate(
	Instance* instance,
	const InputValueSet& valueSet,
	float T,
	float dT
) const
{
	InCombine3Instance* ici = static_cast< InCombine3Instance* >(instance);
	
	float value1 = m_source[0] ? m_source[0]->evaluate(ici->sourceInstance[0], valueSet, T, dT) : 0.0f;
	float value2 = m_source[1] ? m_source[1]->evaluate(ici->sourceInstance[1], valueSet, T, dT) : 0.0f;
	float value3 = m_source[2] ? m_source[2]->evaluate(ici->sourceInstance[2], valueSet, T, dT) : 0.0f;
	
	float cv1 = value1 * m_valueMul[0] + m_valueAdd[0];
	float cv2 = value2 * m_valueMul[1] + m_valueAdd[1];
	float cv3 = value3 * m_valueMul[2] + m_valueAdd[2];
	
	return cv1 + cv2 + cv3;
}

bool InCombine3::serialize(ISerializer& s)
{
	s >> MemberStaticArray< Ref< IInputNode >, 3, MemberRef< IInputNode > >(L"source", m_source);
	s >> MemberStaticArray< float, 3 >(L"valueMul", m_valueMul);
	s >> MemberStaticArray< float, 3 >(L"valueAdd", m_valueAdd);
	return true;
}
	
	}
}
