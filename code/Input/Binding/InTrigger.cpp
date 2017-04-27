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
#include "Core/Serialization/MemberEnum.h"
#include "Core/Serialization/MemberRef.h"
#include "Input/Binding/InTrigger.h"
#include "Input/Binding/InputValueSet.h"
#include "Input/Binding/ValueDigital.h"

namespace traktor
{
	namespace input
	{
		namespace
		{

struct InTriggerInstance : public RefCountImpl< IInputNode::Instance >
{
	Ref< IInputNode::Instance > sourceInstance;
	float previousValue;
	float pulseEnd;
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.InTrigger", 0, InTrigger, IInputNode)

InTrigger::InTrigger()
:	m_flank(FlPositive)
,	m_duration(0.0f)
{
}

InTrigger::InTrigger(IInputNode* source, Flank flank, float duration)
:	m_source(source)
,	m_flank(flank)
,	m_duration(duration)
{
}

Ref< IInputNode::Instance > InTrigger::createInstance() const
{
	Ref< InTriggerInstance > instance = new InTriggerInstance();
	instance->sourceInstance = m_source->createInstance();
	instance->previousValue = 0.0f;
	instance->pulseEnd = 0.0f;
	return instance;
}

float InTrigger::evaluate(
	Instance* instance,
	const InputValueSet& valueSet,
	float T,
	float dT
) const
{
	InTriggerInstance* iti = static_cast< InTriggerInstance* >(instance);

	float V = m_source->evaluate(iti->sourceInstance, valueSet, T, dT);
	float dV = V - iti->previousValue;
	
	iti->previousValue = V;
	
	bool pulse = false;

	if (m_flank == FlPositive && dV > FUZZY_EPSILON)
		pulse = true;
	else if (m_flank == FlNegative && dV < -FUZZY_EPSILON)
		pulse = true;
		
	if (pulse)
		iti->pulseEnd = T + m_duration;
		
	if (T < iti->pulseEnd)
		return asFloat(true);
	else
		return asFloat(false);
}

void InTrigger::serialize(ISerializer& s)
{
	const MemberEnum< Flank >::Key c_Flank_Keys[] =
	{
		{ L"FlPositive", FlPositive },
		{ L"FlNegative", FlNegative },
		{ 0 }
	};

	s >> MemberRef< IInputNode >(L"source", m_source);
	s >> MemberEnum< Flank >(L"flank", m_flank, c_Flank_Keys);
	s >> Member< float >(L"duration", m_duration);
}
	
	}
}
