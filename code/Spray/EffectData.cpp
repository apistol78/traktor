/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/AttributePrivate.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Spray/Effect.h"
#include "Spray/EffectData.h"
#include "Spray/EffectLayerData.h"

namespace traktor::spray
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.spray.EffectData", 0, EffectData, ISerializable)

EffectData::EffectData(
	float duration,
	float loopStart,
	float loopEnd,
	const RefArray< EffectLayerData >& layers
)
:	m_duration(duration)
,	m_loopStart(loopStart)
,	m_loopEnd(loopEnd)
,	m_layers(layers)
{
}

Ref< Effect > EffectData::createEffect(resource::IResourceManager* resourceManager, const world::IEntityFactory* entityFactory) const
{
	RefArray< EffectLayer > effectLayers;
	effectLayers.reserve(m_layers.size());
	
	for (auto layer : m_layers)
	{
		Ref< EffectLayer > effectLayer = layer->createEffectLayer(resourceManager, entityFactory);
		if (effectLayer)
			effectLayers.push_back(effectLayer);
	}

	return new Effect(
		m_duration,
		m_loopStart,
		m_loopEnd,
		effectLayers
	);
}

void EffectData::addLayer(EffectLayerData* layer)
{
	m_layers.push_back(layer);
}

void EffectData::serialize(ISerializer& s)
{
	s >> Member< float >(L"duration", m_duration, AttributeUnit(UnitType::Seconds));
	s >> Member< float >(L"loopStart", m_loopStart, AttributeUnit(UnitType::Seconds));
	s >> Member< float >(L"loopEnd", m_loopEnd, AttributeUnit(UnitType::Seconds));
	s >> MemberRefArray< EffectLayerData >(L"layers", m_layers, AttributePrivate());
}

}
