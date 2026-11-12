/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Const.h"
#include "Core/Math/Float.h"
#include "Terrain/Editor/IFallOff.h"
#include "Terrain/Editor/SmoothBrush.h"

namespace traktor::terrain
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.SmoothBrush", SmoothBrush, IBrush)

SmoothBrush::SmoothBrush(TerrainEditData* data)
:	m_data(data)
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
	const int32_t size = m_data->getSize();

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
			if (abs(a) <= 0.0f)
				continue;

			const float h = m_data->getGridHeightNearest(gx, gy);

			// Get the average height.
			float height = 0.0f;
			for (int32_t iiy = -1; iiy <= 1; ++iiy)
			{
				for (int32_t iix = -1; iix <= 1; ++iix)
					height += m_data->getGridHeightNearest(gx + iix, gy + iiy);
			}
			height /= 9.0f;

			m_data->setGridHeight(gx, gy, lerp(h, height, a));
		}
	}
}

void SmoothBrush::end(float x, float y)
{
}

}
