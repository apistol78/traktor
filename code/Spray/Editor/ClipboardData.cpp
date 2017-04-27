/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Spray/EffectLayerData.h"
#include "Spray/Editor/ClipboardData.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.ClipboardData", 0, ClipboardData, ISerializable)

void ClipboardData::addLayer(const EffectLayerData* layer)
{
	m_layers.push_back(layer);
}

const RefArray< const EffectLayerData >& ClipboardData::getLayers() const
{
	return m_layers;
}

void ClipboardData::serialize(ISerializer& s)
{
	s >> MemberRefArray< const EffectLayerData >(L"layers", m_layers);
}

	}
}
