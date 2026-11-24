/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Heightfield/Editor/ElevateBrush.h"

#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor::hf
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.hf.ElevateBrush", 0, ElevateBrush, ProceduralBrush)

void ElevateBrush::serialize(ISerializer& s)
{
	s >> Member< float >(L"height", m_height, AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"falloff", m_falloff, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));
}

float ElevateBrush::getReach() const
{
	return m_falloff;
}

float ElevateBrush::sample(float worldX, float worldZ, float spacing, float height, float distance, float layerHeight) const
{
	return height + m_height * falloffWeight(distance, m_falloff);
}

}
