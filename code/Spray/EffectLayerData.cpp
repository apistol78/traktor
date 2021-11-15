#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRef.h"
#include "Spray/EffectLayer.h"
#include "Spray/EffectLayerData.h"
#include "Spray/EmitterData.h"
#include "Spray/SequenceData.h"
#include "Spray/TrailData.h"
#include "World/IEntityBuilder.h"
#include "World/IEntityEventData.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.EffectLayerData", 4, EffectLayerData, ISerializable)

Ref< EffectLayer > EffectLayerData::createEffectLayer(resource::IResourceManager* resourceManager, const world::IEntityBuilder* entityBuilder) const
{
	Ref< Emitter > emitter;
	Ref< Trail > trail;
	Ref< Sequence > sequence;
	Ref< world::IEntityEvent > triggerEnable;
	Ref< world::IEntityEvent > triggerDisable;

	if (m_emitter)
	{
		emitter = m_emitter->createEmitter(resourceManager, entityBuilder);
		if (!emitter)
			return nullptr;
	}

	if (m_trail)
	{
		trail = m_trail->createTrail(resourceManager);
		if (!trail)
			return nullptr;
	}

	if (m_sequence && entityBuilder)
	{
		sequence = m_sequence->createSequence(entityBuilder);
		if (!sequence)
			return nullptr;
	}

	if (m_triggerEnable && entityBuilder)
	{
		triggerEnable = entityBuilder->create(m_triggerEnable);
		if (!triggerEnable)
			return nullptr;
	}

	if (m_triggerDisable && entityBuilder)
	{
		triggerDisable = entityBuilder->create(m_triggerDisable);
		if (triggerDisable)
			return nullptr;
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
	T_FATAL_ASSERT (s.getVersion() >= 4);

	s >> Member< std::wstring >(L"name", m_name);
	s >> Member< float >(L"time", m_time);
	s >> Member< float >(L"duration", m_duration);
	s >> MemberRef< EmitterData >(L"emitter", m_emitter);
	s >> MemberRef< TrailData >(L"trail", m_trail);
	s >> MemberRef< SequenceData >(L"sequence", m_sequence);
	s >> MemberRef< world::IEntityEventData >(L"triggerEnable", m_triggerEnable);
	s >> MemberRef< world::IEntityEventData >(L"triggerDisable", m_triggerDisable);
}

	}
}
