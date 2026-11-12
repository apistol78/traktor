/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Terrain/Editor/FlattenBrush.h"

#include "Core/Math/Const.h"
#include "Core/Math/Float.h"
#include "Terrain/Editor/IFallOff.h"

namespace traktor::terrain
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.FlattenBrush", FlattenBrush, IBrush)

FlattenBrush::FlattenBrush(TerrainEditData* data)
	: m_data(data)
	, m_radius(0)
	, m_fallOff(0)
	, m_strength(0.0f)
	, m_height(0.0f)
	, m_alternative(false)
	, m_explicit(false)
{
}

uint32_t FlattenBrush::begin(float x, float y, const State& state)
{
	m_radius = state.radius;
	m_fallOff = state.falloff;
	m_strength = abs(state.strength) * 0.05f;
	m_alternative = (state.strength < 0.0f);

	if (!m_explicit)
		m_height = m_data->getGridHeightNearest(x, y);

	return MdHeight;
}

void FlattenBrush::apply(float x, float y)
{
	for (int32_t iy = -m_radius; iy <= m_radius; ++iy)
	{
		for (int32_t ix = -m_radius; ix <= m_radius; ++ix)
		{
			const float fx = float(ix) / m_radius;
			const float fy = float(iy) / m_radius;

			const float a = m_fallOff->evaluate(fx, fy) * m_strength;
			if (abs(a) <= FUZZY_EPSILON)
				continue;

			const float gx = x + ix;
			const float gy = y + iy;
			const float h = m_data->getGridHeightNearest(gx, gy);

			if (!m_alternative || h < m_height)
				m_data->setGridHeight(gx, gy, lerp(h, m_height, a));
		}
	}
}

void FlattenBrush::end(float x, float y)
{
}

void FlattenBrush::setHeight(float height)
{
	m_explicit = true;
	m_height = m_data->worldToUnit(height);
}

}
