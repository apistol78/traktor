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
#include "Input/Binding/InThreshold.h"
#include "Input/Binding/InputValueSet.h"
#include "Input/Binding/ValueDigital.h"

namespace traktor
{
	namespace input
	{
		namespace
		{

struct InThresholdInstance : public RefCountImpl< IInputNode::Instance >
{
	Ref< IInputNode::Instance > sourceInstance;
	float issueTime;
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.InThreshold", 0, InThreshold, IInputNode)

InThreshold::InThreshold()
:	m_duration(0.0f)
{
}

InThreshold::InThreshold(IInputNode* source, float duration)
:	m_source(source)
,	m_duration(duration)
{
}

Ref< IInputNode::Instance > InThreshold::createInstance() const
{
	Ref< InThresholdInstance > instance = new InThresholdInstance();
	instance->sourceInstance = m_source->createInstance();
	instance->issueTime = -1.0f;
	return instance;
}

float InThreshold::evaluate(
	Instance* instance,
	const InputValueSet& valueSet,
	float T,
	float dT
) const
{
	InThresholdInstance* ipi = static_cast< InThresholdInstance* >(instance);

	float V = m_source->evaluate(ipi->sourceInstance, valueSet, T, dT);

	if (!asBoolean(V))
	{
		ipi->issueTime = -1.0f;
		return asFloat(false);
	}

	if (ipi->issueTime < 0.0f)
		ipi->issueTime = T;

	float timeSinceIssue = T - ipi->issueTime;
	if (timeSinceIssue >= m_duration)
		return V;
	else
		return 0.0f;
}

void InThreshold::serialize(ISerializer& s)
{
	s >> MemberRef< IInputNode >(L"source", m_source);
	s >> Member< float >(L"duration", m_duration);
}
	
	}
}
