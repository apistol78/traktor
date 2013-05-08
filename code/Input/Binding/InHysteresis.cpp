#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "Input/Binding/InHysteresis.h"
#include "Input/Binding/InputValueSet.h"
#include "Input/Binding/ValueDigital.h"

namespace traktor
{
	namespace input
	{
		namespace
		{

struct InHysteresisInstance : public RefCountImpl< IInputNode::Instance >
{
	Ref< IInputNode::Instance > sourceInstance;
	float currentValue;
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.InHysteresis", 0, InHysteresis, IInputNode)

InHysteresis::InHysteresis()
{
	m_limit[0] = -1.0f;
	m_limit[1] = 1.0f;
	m_output[0] = -1.0f;
	m_output[1] = 1.0f;
}

InHysteresis::InHysteresis(IInputNode* source, float limitMin, float limitMax, float outputMin, float outputMax)
:	m_source(source)
{
	m_limit[0] = limitMin;
	m_limit[1] = limitMax;
	m_output[0] = outputMin;
	m_output[1] = outputMax;
}

Ref< IInputNode::Instance > InHysteresis::createInstance() const
{
	Ref< InHysteresisInstance > instance = new InHysteresisInstance();
	instance->sourceInstance = m_source->createInstance();
	instance->currentValue = 0.0f;
	return instance;
}

float InHysteresis::evaluate(
	Instance* instance,
	const InputValueSet& valueSet,
	float T,
	float dT
) const
{
	InHysteresisInstance* ihi = static_cast< InHysteresisInstance* >(instance);

	float V = m_source->evaluate(ihi->sourceInstance, valueSet, T, dT);

	if (V > m_limit[1])
		ihi->currentValue = m_output[1];
	if (V < m_limit[0])
		ihi->currentValue = m_output[0];

	return ihi->currentValue;
}

void InHysteresis::serialize(ISerializer& s)
{
	s >> MemberRef< IInputNode >(L"source", m_source);
	s >> MemberStaticArray< float, 2 >(L"limit", m_limit);
	s >> MemberStaticArray< float, 2 >(L"output", m_output);
}
	
	}
}
