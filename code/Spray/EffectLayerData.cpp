#include "Spray/EffectLayer.h"
#include "Spray/EffectLayerData.h"
#include "Spray/EmitterData.h"
#include "Spray/SequenceData.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRef.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.EffectLayerData", 0, EffectLayerData, ISerializable)

EffectLayerData::EffectLayerData()
:	m_time(0.0f)
,	m_duration(0.0f)
{
}

Ref< EffectLayer > EffectLayerData::createEffectLayer(resource::IResourceManager* resourceManager) const
{
	Ref< Emitter > emitter;
	Ref< Sequence > sequence;

	if (m_emitter)
	{
		emitter = m_emitter->createEmitter(resourceManager);
		if (!emitter)
			return 0;
	}

	if (m_sequence)
	{
		sequence = m_sequence->createSequence(resourceManager);
		if (!sequence)
			return 0;
	}

	return new EffectLayer(
		m_time,
		m_duration,
		emitter,
		sequence
	);
}

bool EffectLayerData::serialize(ISerializer& s)
{
	s >> Member< float >(L"time", m_time);
	s >> Member< float >(L"duration", m_duration);
	s >> MemberRef< EmitterData >(L"emitter", m_emitter);
	s >> MemberRef< SequenceData >(L"sequence", m_sequence);
	return true;
}

	}
}
