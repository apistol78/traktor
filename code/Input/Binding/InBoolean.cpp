#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "Input/Binding/InBoolean.h"
#include "Input/Binding/InputValueSet.h"

namespace traktor
{
	namespace input
	{
		namespace
		{

struct InBooleanInstance : public RefCountImpl< IInputNode::Instance >
{
	Ref< IInputNode::Instance > sourceInstance[2];
};
		
		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.InBoolean", 0, InBoolean, IInputNode)

InBoolean::InBoolean()
:	m_op(OpAnd)
{
}

InBoolean::InBoolean(
	IInputNode* source1,
	IInputNode* source2,
	Operator op
)
:	m_op(op)
{
	m_source[0] = source1;
	m_source[1] = source2;
}

Ref< IInputNode::Instance > InBoolean::createInstance() const
{
	Ref< InBooleanInstance > instance = new InBooleanInstance();
	instance->sourceInstance[0] = m_source[0]->createInstance();
	instance->sourceInstance[1] = m_source[1]->createInstance();
	return instance;
}

float InBoolean::evaluate(
	Instance* instance,
	const InputValueSet& valueSet,
	float T,
	float dT
) const
{
	InBooleanInstance* ibi = static_cast< InBooleanInstance* >(instance);
	
	bool value1 = m_source[0]->evaluate(ibi->sourceInstance[0], valueSet, T, dT) > 0.5f;
	bool value2 = m_source[1]->evaluate(ibi->sourceInstance[1], valueSet, T, dT) > 0.5f;
	
	bool result = false;
	switch (m_op)
	{
	case OpAnd:
		result = value1 & value2;
		break;
		
	case OpOr:
		result = value1 | value2;
		break;
		
	case OpXor:
		result = value1 ^ value2;
		break;
	}
	
	if (result)
		return 1.0f;
	else
		return 0.0f;
}

bool InBoolean::serialize(ISerializer& s)
{
	const MemberEnum< Operator >::Key c_Operator_Keys[] =
	{
		{ L"OpAnd", OpAnd },
		{ L"OpOr", OpOr },
		{ L"OpXor", OpXor },
		0
	};
	
	s >> MemberStaticArray< Ref< IInputNode >, 2, MemberRef< IInputNode > >(L"source", m_source);
	s >> MemberEnum< Operator >(L"op", m_op, c_Operator_Keys);
	
	return true;
}
	
	}
}
