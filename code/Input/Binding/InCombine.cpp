#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "Input/Binding/InCombine.h"
#include "Input/Binding/InputValueSet.h"

namespace traktor
{
	namespace input
	{
		namespace
		{

struct InCombineInstance : public RefCountImpl< IInputNode::Instance >
{
	Ref< IInputNode::Instance > sourceInstance[2];
};
		
		}

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

Ref< IInputNode::Instance > InCombine::createInstance() const
{
	Ref< InCombineInstance > instance = new InCombineInstance();
	instance->sourceInstance[0] = m_source[0] ? m_source[0]->createInstance() : 0;
	instance->sourceInstance[1] = m_source[1] ? m_source[1]->createInstance() : 0;
	return instance;
}

float InCombine::evaluate(
	Instance* instance,
	const InputValueSet& valueSet,
	float T,
	float dT
) const
{
	InCombineInstance* ici = static_cast< InCombineInstance* >(instance);
	
	float value1 = m_source[0] ? m_source[0]->evaluate(ici->sourceInstance[0], valueSet, T, dT) : 0.0f;
	float value2 = m_source[1] ? m_source[1]->evaluate(ici->sourceInstance[1], valueSet, T, dT) : 0.0f;
	
	float cv1 = value1 * m_valueMul[0] + m_valueAdd[0];
	float cv2 = value2 * m_valueMul[1] + m_valueAdd[1];
	
	return cv1 + cv2;
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
