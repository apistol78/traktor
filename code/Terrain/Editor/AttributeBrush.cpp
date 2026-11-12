/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Terrain/Editor/AttributeBrush.h"

#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "Terrain/Editor/IFallOff.h"

namespace traktor::terrain
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.AttributeBrush", AttributeBrush, IBrush)

AttributeBrush::AttributeBrush(TerrainEditData* data)
	: m_data(data)
	, m_radius(0)
	, m_fallOff(0)
	, m_strength(0.0f)
	, m_attribute(0)
{
}

uint32_t AttributeBrush::begin(float x, float y, const State& state)
{
	m_radius = state.radius;
	m_fallOff = state.falloff;
	m_strength = state.strength;
	m_attribute = (uint8_t)state.attribute;
	return MdAttribute;
}

void AttributeBrush::apply(float x, float y)
{
	const int32_t gx = (int32_t)x;
	const int32_t gz = (int32_t)y;

	for (int32_t iy = -m_radius; iy <= m_radius; ++iy)
	{
		for (int32_t ix = -m_radius; ix <= m_radius; ++ix)
		{
			const float fx = float(ix) / m_radius;
			const float fy = float(iy) / m_radius;

			// Strength is the peak density of the stroke, the falloff it's profile.
			const float a = m_fallOff->evaluate(fx, fy) * m_strength;
			if (abs(a) <= FUZZY_EPSILON)
				continue;

			const float density = m_data->getGridAttributeDensity(gx + ix, gz + iy, m_attribute) / 255.0f;

			// Approach the profile rather than accumulate onto it, thus a stroke lay
			// down the shape of the falloff instead of saturating where it overlap
			// itself. An inverted stroke carve the density back down the same way.
			const float target = (a >= 0.0f) ? max(density, a) : min(density, 1.0f + a);

			m_data->setGridAttribute(gx + ix, gz + iy, m_attribute, (uint8_t)(clamp(target, 0.0f, 1.0f) * 255.0f + 0.5f));
		}
	}
}

void AttributeBrush::end(float x, float y)
{
}

}
