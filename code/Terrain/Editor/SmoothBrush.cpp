/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
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

namespace traktor::terrain
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
	hf::Heightfield* hf = m_heightfield;

	const hf::height_t* hm = hf->getHeights();
	const int32_t size = hf->getSize();

	for (int32_t iy = -m_radius; iy <= m_radius; ++iy)
	{
		const int32_t gy = iy + y;
		if (gy < 1 || gy > size - 2)
			continue;

		const float fy = float(iy) / m_radius;

		for (int32_t ix = -m_radius; ix <= m_radius; ++ix)
		{
			const int32_t gx = ix + x;
			if (gx < 1 || gx > size - 2)
				continue;

			const float fx = float(ix) / m_radius;

			// Evaluate falloff function.
			const float a = m_fallOff->evaluate(fx, fy) * m_strength;
			if (abs(a) <= FUZZY_EPSILON)
				continue;

			const float h = hm[gx + gy * size] / 65535.0f;

			// Get the average height.
			float height = 0.0f;
			for (int32_t iiy = -1; iiy <= 1; ++iiy)
			{
				for (int32_t iix = -1; iix <= 1; ++iix)
				{
					height += hm[(gx + iix) + (gy + iiy) * size] / 65535.0f;
				}
			}
			height /= 9.0f;

			hf->setGridHeight(gx, gy, lerp(h, height, a));
		}
	}
}

void SmoothBrush::end(float x, float y)
{
}

}
