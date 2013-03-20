#include "Spray/EffectLayer.h"
#include "Spray/EffectLayerInstance.h"
#include "Spray/Emitter.h"
#include "Spray/ITrigger.h"
#include "Spray/Sequence.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.EffectLayer", EffectLayer, Object)

EffectLayer::EffectLayer(
	float time,
	float duration,
	Emitter* emitter,
	Sequence* sequence,
	ITrigger* triggerEnable,
	ITrigger* triggerDisable
)
:	m_time(time)
,	m_duration(duration)
,	m_emitter(emitter)
,	m_sequence(sequence)
,	m_triggerEnable(triggerEnable)
,	m_triggerDisable(triggerDisable)
{
}

Ref< EffectLayerInstance > EffectLayer::createInstance() const
{
	Ref< EmitterInstance > emitterInstance;
	Ref< SequenceInstance > sequenceInstance;
	Ref< ITriggerInstance > triggerInstanceEnable;
	Ref< ITriggerInstance > triggerInstanceDisable;

	if (m_emitter)
	{
		emitterInstance = m_emitter->createInstance(m_duration);
		if (!emitterInstance)
			return 0;
	}

	if (m_sequence)
	{
		sequenceInstance = m_sequence->createInstance();
		if (!sequenceInstance)
			return 0;
	}

	if (m_triggerEnable)
	{
		triggerInstanceEnable = m_triggerEnable->createInstance();
		if (!triggerInstanceEnable)
			return 0;
	}

	if (m_triggerDisable)
	{
		triggerInstanceDisable = m_triggerDisable->createInstance();
		if (!triggerInstanceDisable)
			return 0;
	}

	return new EffectLayerInstance(
		this,
		emitterInstance,
		sequenceInstance,
		triggerInstanceEnable,
		triggerInstanceDisable
	);
}

	}
}
