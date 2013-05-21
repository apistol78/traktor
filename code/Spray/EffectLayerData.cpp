#include "Spray/EffectLayer.h"
#include "Spray/EffectLayerData.h"
#include "Spray/EmitterData.h"
#include "Spray/ITriggerData.h"
#include "Spray/SequenceData.h"
#include "Spray/TrailData.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRef.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.EffectLayerData", 3, EffectLayerData, ISerializable)

EffectLayerData::EffectLayerData()
:	m_time(0.0f)
,	m_duration(0.0f)
{
}

Ref< EffectLayer > EffectLayerData::createEffectLayer(resource::IResourceManager* resourceManager) const
{
	Ref< Emitter > emitter;
	Ref< Trail > trail;
	Ref< Sequence > sequence;
	Ref< ITrigger > triggerEnable;
	Ref< ITrigger > triggerDisable;

	if (m_emitter)
	{
		emitter = m_emitter->createEmitter(resourceManager);
		if (!emitter)
			return 0;
	}

	if (m_trail)
	{
		trail = m_trail->createTrail(resourceManager);
		if (!trail)
			return 0;
	}

	if (m_sequence)
	{
		sequence = m_sequence->createSequence(resourceManager);
		if (!sequence)
			return 0;
	}

	if (m_triggerEnable)
	{
		triggerEnable = m_triggerEnable->createTrigger(resourceManager);
		if (!triggerEnable)
			return 0;
	}

	if (m_triggerDisable)
	{
		triggerDisable = m_triggerDisable->createTrigger(resourceManager);
		if (triggerDisable)
			return 0;
	}

	return new EffectLayer(
		m_time,
		m_duration,
		emitter,
		trail,
		sequence,
		triggerEnable,
		triggerDisable
	);
}

void EffectLayerData::serialize(ISerializer& s)
{
	if (s.getVersion() >= 1)
		s >> Member< std::wstring >(L"name", m_name);

	s >> Member< float >(L"time", m_time);
	s >> Member< float >(L"duration", m_duration);
	s >> MemberRef< EmitterData >(L"emitter", m_emitter);

	if (s.getVersion() >= 3)
		s >> MemberRef< TrailData >(L"trail", m_trail);

	s >> MemberRef< SequenceData >(L"sequence", m_sequence);

	if (s.getVersion() >= 2)
	{
		s >> MemberRef< ITriggerData >(L"triggerEnable", m_triggerEnable);
		s >> MemberRef< ITriggerData >(L"triggerDisable", m_triggerDisable);
	}
}

	}
}
