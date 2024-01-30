/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRef.h"
#include "Spray/EffectLayer.h"
#include "Spray/EffectLayerData.h"
#include "Spray/EmitterData.h"
#include "Spray/SequenceData.h"
#include "Spray/TrailData.h"
#include "World/IEntityEventData.h"
#include "World/IEntityFactory.h"

namespace traktor::spray
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.EffectLayerData", 4, EffectLayerData, ISerializable)

Ref< EffectLayer > EffectLayerData::createEffectLayer(resource::IResourceManager* resourceManager, const world::IEntityFactory* entityFactory) const
{
	Ref< Emitter > emitter;
	Ref< Trail > trail;
	Ref< Sequence > sequence;
	Ref< world::IEntityEvent > triggerEnable;
	Ref< world::IEntityEvent > triggerDisable;

	if (m_emitter)
	{
		emitter = m_emitter->createEmitter(resourceManager, entityFactory);
		if (!emitter)
			return nullptr;
	}

	if (m_trail)
	{
		trail = m_trail->createTrail(resourceManager);
		if (!trail)
			return nullptr;
	}

	if (m_sequence && entityFactory)
	{
		sequence = m_sequence->createSequence(entityFactory);
		if (!sequence)
			return nullptr;
	}

	if (m_triggerEnable && entityFactory)
	{
		triggerEnable = entityFactory->createEntityEvent(nullptr, *m_triggerEnable);
		if (!triggerEnable)
			return nullptr;
	}

	if (m_triggerDisable && entityFactory)
	{
		triggerDisable = entityFactory->createEntityEvent(nullptr, *m_triggerDisable);
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
	s >> Member< float >(L"time", m_time, AttributeUnit(UnitType::Seconds));
	s >> Member< float >(L"duration", m_duration, AttributeUnit(UnitType::Seconds));
	s >> MemberRef< EmitterData >(L"emitter", m_emitter);
	s >> MemberRef< TrailData >(L"trail", m_trail);
	s >> MemberRef< SequenceData >(L"sequence", m_sequence);
	s >> MemberRef< world::IEntityEventData >(L"triggerEnable", m_triggerEnable);
	s >> MemberRef< world::IEntityEventData >(L"triggerDisable", m_triggerDisable);
}

}
