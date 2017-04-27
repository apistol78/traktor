/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Spray/Effect.h"
#include "Spray/EffectData.h"
#include "Spray/EffectLayerData.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.spray.EffectData", 0, EffectData, ISerializable)

EffectData::EffectData()
:	m_duration(0.0f)
,	m_loopStart(0.0f)
,	m_loopEnd(0.0f)
{
}

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

Ref< Effect > EffectData::createEffect(resource::IResourceManager* resourceManager, const world::IEntityBuilder* entityBuilder) const
{
	RefArray< EffectLayer > effectLayers;
	for (RefArray< EffectLayerData >::const_iterator i = m_layers.begin(); i != m_layers.end(); ++i)
	{
		Ref< EffectLayer > effectLayer = (*i)->createEffectLayer(resourceManager, entityBuilder);
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
	s >> Member< float >(L"duration", m_duration);
	s >> Member< float >(L"loopStart", m_loopStart);
	s >> Member< float >(L"loopEnd", m_loopEnd);
	s >> MemberRefArray< EffectLayerData >(L"layers", m_layers);
}

	}
}
