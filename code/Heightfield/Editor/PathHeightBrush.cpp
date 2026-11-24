/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Heightfield/Editor/PathHeightBrush.h"

#include "Core/Math/Float.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor::hf
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.hf.PathHeightBrush", 0, PathHeightBrush, ProceduralBrush)

void PathHeightBrush::serialize(ISerializer& s)
{
	s >> Member< float >(L"falloff", m_falloff, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));
}

float PathHeightBrush::getReach() const
{
	return m_falloff;
}

float PathHeightBrush::sample(float worldX, float worldZ, float spacing, float height, float distance, float layerHeight) const
{
	return lerp(height, layerHeight, falloffWeight(distance, m_falloff));
}

}
