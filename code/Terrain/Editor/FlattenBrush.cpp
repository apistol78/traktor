/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Const.h"
#include "Core/Math/Float.h"
#include "Heightfield/Heightfield.h"
#include "Terrain/Editor/FlattenBrush.h"
#include "Terrain/Editor/IFallOff.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.FlattenBrush", FlattenBrush, IBrush)

FlattenBrush::FlattenBrush(const resource::Proxy< hf::Heightfield >& heightfield)
:	m_heightfield(heightfield)
,	m_radius(0)
,	m_fallOff(0)
,	m_strength(0.0f)
,	m_height(0.0f)
,	m_explicit(false)
{
}

uint32_t FlattenBrush::begin(float x, float y, const State& state)
{
	m_radius = state.radius;
	m_fallOff = state.falloff;
	m_strength = abs(state.strength) * 0.5f;

	if (!m_explicit)
		m_height = m_heightfield->getGridHeightNearest(x, y);

	return MdHeight;
}

void FlattenBrush::apply(float x, float y)
{
	for (int32_t iy = -m_radius; iy <= m_radius; ++iy)
	{
		for (int32_t ix = -m_radius; ix <= m_radius; ++ix)
		{
			float fx = float(ix) / m_radius;
			float fy = float(iy) / m_radius;

			float a = m_fallOff->evaluate(fx, fy) * m_strength;
			if (abs(a) <= FUZZY_EPSILON)
				continue;

			float gx = x + ix;
			float gy = y + iy;

			float h = m_heightfield->getGridHeightNearest(gx, gy);
			m_heightfield->setGridHeight(gx, gy, lerp(h, m_height, a));
		}
	}
}

void FlattenBrush::end(float x, float y)
{
}

void FlattenBrush::setHeight(float height)
{
	m_explicit = true;
	m_height = m_heightfield->worldToUnit(height);
}

	}
}
