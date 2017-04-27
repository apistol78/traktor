/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Math/Const.h"
#include "Core/Math/Envelope.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "Input/Binding/InEnvelope.h"
#include "Input/Binding/InputValueSet.h"

namespace traktor
{
	namespace input
	{
		namespace
		{

struct InEnvelopeInstance : public RefCountImpl< IInputNode::Instance >
{
	Ref< IInputNode::Instance > sourceInstance;
	Envelope< float, HermiteEvaluator< float > > envelope;
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.InEnvelope", 0, InEnvelope, IInputNode)

InEnvelope::InEnvelope()
:	m_center(0.5f)
,	m_width(0.5f)
{
	m_keys[0] = 0.0f;
	m_keys[1] = 1.0f * (0.5f - 0.5f / 2.0f);
	m_keys[2] = 1.0f * (0.5f + 0.5f / 2.0f);
	m_keys[3] = 1.0f;
}

Ref< IInputNode::Instance > InEnvelope::createInstance() const
{
	Ref< InEnvelopeInstance > instance = new InEnvelopeInstance();
	instance->sourceInstance = m_source->createInstance();
	instance->envelope.addKey(0.0f, m_keys[0]);
	instance->envelope.addKey(m_center - m_width / 2.0f, m_keys[1]);
	instance->envelope.addKey(m_center + m_width / 2.0f, m_keys[2]);
	instance->envelope.addKey(1.0f, m_keys[3]);
	return instance;
}

float InEnvelope::evaluate(
	Instance* instance,
	const InputValueSet& valueSet,
	float T,
	float dT
) const
{
	InEnvelopeInstance* iei = static_cast< InEnvelopeInstance* >(instance);
	float V = m_source->evaluate(iei->sourceInstance, valueSet, T, dT);
	return iei->envelope(V);
}

void InEnvelope::serialize(ISerializer& s)
{
	s >> MemberRef< IInputNode >(L"source", m_source);
	s >> MemberStaticArray< float, 4 >(L"keys", m_keys);
	s >> Member< float >(L"center", m_center);
	s >> Member< float >(L"width", m_width);
}
	
	}
}
