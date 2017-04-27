/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRef.h"
#include "Input/Binding/InDelay.h"
#include "Input/Binding/InputValueSet.h"
#include "Input/Binding/ValueDigital.h"

namespace traktor
{
	namespace input
	{
		namespace
		{

struct InDelayInstance : public RefCountImpl< IInputNode::Instance >
{
	Ref< IInputNode::Instance > sourceInstance;
	float previousValue;
	float issueTime;
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.InDelay", 0, InDelay, IInputNode)

InDelay::InDelay()
:	m_delay(0.0f)
{
}

InDelay::InDelay(IInputNode* source, float delay)
:	m_source(source)
,	m_delay(delay)
{
}

Ref< IInputNode::Instance > InDelay::createInstance() const
{
	Ref< InDelayInstance > instance = new InDelayInstance();
	instance->sourceInstance = m_source->createInstance();
	instance->previousValue = 0.0f;
	instance->issueTime = 0.0f;
	return instance;
}

float InDelay::evaluate(
	Instance* instance,
	const InputValueSet& valueSet,
	float T,
	float dT
) const
{
	InDelayInstance* ipi = static_cast< InDelayInstance* >(instance);

	float V = m_source->evaluate(ipi->sourceInstance, valueSet, T, dT);
	float dV = V - ipi->previousValue;

	ipi->previousValue = V;
	
	if (!asBoolean(V))
		return asFloat(false);

	if (dV > FUZZY_EPSILON)
		ipi->issueTime = T;
	
	float T0 = T - ipi->issueTime - m_delay;
	return asFloat(T0 > 0.0f);
}

void InDelay::serialize(ISerializer& s)
{
	s >> MemberRef< IInputNode >(L"source", m_source);
	s >> Member< float >(L"delay", m_delay);
}
	
	}
}
