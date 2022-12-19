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
#include "Terrain/Editor/IFallOff.h"
#include "Terrain/Editor/SmoothBrush.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.SmoothBrush", SmoothBrush, IBrush)

SmoothBrush::SmoothBrush(const resource::Proxy< hf::Heightfield >& heightfield)
:	m_heightfield(heightfield)
,	m_radius(0)
,	m_fallOff(0)
,	m_strength(0.0f)
{
}

uint32_t SmoothBrush::begin(float x, float y, const State& state)
{
	m_radius = state.radius;
	m_fallOff = state.falloff;
	m_strength = powf(abs(state.strength), 3.0f);
	return MdHeight;
}

void SmoothBrush::apply(float x, float y)
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

			float height = 0.0f;
			for (int32_t iiy = -1; iiy <= 1; ++iiy)
			{
				for (int32_t iix = -1; iix <= 1; ++iix)
				{
					height += m_heightfield->getGridHeightNearest(x + ix + iix, y + iy + iiy);
				}
			}
			height /= 9.0f;

			const float h = m_heightfield->getGridHeightNearest(x + ix, y + iy);
			m_heightfield->setGridHeight(x + ix, y + iy, lerp(h, height, a));
		}
	}
}

void SmoothBrush::end(float x, float y)
{
}

	}
}
