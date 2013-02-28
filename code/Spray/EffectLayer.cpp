#include "Spray/EffectLayer.h"
#include "Spray/EffectLayerInstance.h"
#include "Spray/Emitter.h"
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
	Sequence* sequence
)
:	m_time(time)
,	m_duration(duration)
,	m_emitter(emitter)
,	m_sequence(sequence)
{
}

Ref< EffectLayerInstance > EffectLayer::createInstance() const
{
	Ref< EmitterInstance > emitterInstance;
	Ref< SequenceInstance > sequenceInstance;

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

	return new EffectLayerInstance(
		this,
		emitterInstance,
		sequenceInstance
	);
}

	}
}
