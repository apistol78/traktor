/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Heightfield/Editor/SetHeightBrush.h"

#include "Core/Math/Float.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor::hf
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.hf.SetHeightBrush", 0, SetHeightBrush, ProceduralBrush)

void SetHeightBrush::serialize(ISerializer& s)
{
	s >> Member< float >(L"height", m_height, AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"falloff", m_falloff, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));
}

float SetHeightBrush::getReach() const
{
	return m_falloff;
}

float SetHeightBrush::sample(float worldX, float worldZ, float spacing, float height, float distance, float layerHeight) const
{
	return lerp(height, m_height, falloffWeight(distance, m_falloff));
}

}
