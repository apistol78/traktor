/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Spray/EffectLayer.h"
#include "Spray/EffectLayerInstance.h"
#include "Spray/Emitter.h"
#include "Spray/Sequence.h"
#include "Spray/Trail.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.EffectLayer", EffectLayer, Object)

EffectLayer::EffectLayer(
	float time,
	float duration,
	Emitter* emitter,
	Trail* trail,
	Sequence* sequence,
	const world::IEntityEvent* triggerEnable,
	const world::IEntityEvent* triggerDisable
)
:	m_time(time)
,	m_duration(duration)
,	m_emitter(emitter)
,	m_trail(trail)
,	m_sequence(sequence)
,	m_triggerEnable(triggerEnable)
,	m_triggerDisable(triggerDisable)
{
}

Ref< EffectLayerInstance > EffectLayer::createInstance() const
{
	Ref< EmitterInstance > emitterInstance;
	Ref< TrailInstance > trailInstance;
	Ref< SequenceInstance > sequenceInstance;

	if (m_emitter)
	{
		emitterInstance = m_emitter->createInstance(m_duration);
		if (!emitterInstance)
			return 0;
	}

	if (m_trail)
	{
		trailInstance = m_trail->createInstance();
		if (!trailInstance)
			return 0;
	}

	if (m_sequence)
	{
		sequenceInstance = m_sequence->createInstance();
		if (!sequenceInstance)
			return 0;
	}

	return new EffectLayerInstance(
		this,
		emitterInstance,
		trailInstance,
		sequenceInstance
	);
}

	}
}
