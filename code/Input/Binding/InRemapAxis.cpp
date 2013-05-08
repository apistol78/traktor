#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "Input/Binding/InRemapAxis.h"
#include "Input/Binding/InputValueSet.h"
#include "Input/Binding/ValueDigital.h"

namespace traktor
{
	namespace input
	{
		namespace
		{

struct InRemapAxisInstance : public RefCountImpl< IInputNode::Instance >
{
	Ref< IInputNode::Instance > sourceInstance;
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.InRemapAxis", 0, InRemapAxis, IInputNode)

InRemapAxis::InRemapAxis()
{
	m_limit[0] = -0.5f;
	m_limit[1] = 0.5f;
	m_output[0] = -1.0f;
	m_output[1] = 0.0f;
	m_output[2] = 1.0f;
}

InRemapAxis::InRemapAxis(IInputNode* source, float limitMin, float limitMax, float outputMin, float outputMid, float outputMax)
:	m_source(source)
{
	m_limit[0] = limitMin;
	m_limit[1] = limitMax;
	m_output[0] = outputMin;
	m_output[1] = outputMid;
	m_output[2] = outputMax;
}

Ref< IInputNode::Instance > InRemapAxis::createInstance() const
{
	Ref< InRemapAxisInstance > instance = new InRemapAxisInstance();
	instance->sourceInstance = m_source->createInstance();
	return instance;
}

float InRemapAxis::evaluate(
	Instance* instance,
	const InputValueSet& valueSet,
	float T,
	float dT
) const
{
	InRemapAxisInstance* ihi = static_cast< InRemapAxisInstance* >(instance);

	float V = m_source->evaluate(ihi->sourceInstance, valueSet, T, dT);

	if (V < m_limit[0])
		return m_output[0];
	if (V > m_limit[1])
		return m_output[2];

	return m_output[1];
}

void InRemapAxis::serialize(ISerializer& s)
{
	s >> MemberRef< IInputNode >(L"source", m_source);
	s >> MemberStaticArray< float, 2 >(L"limit", m_limit);
	s >> MemberStaticArray< float, 3 >(L"output", m_output);
}
	
	}
}
