/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Spray/EffectLayerData.h"
#include "Spray/Editor/ClipboardData.h"

namespace traktor::spray
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
