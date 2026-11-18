/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Terrain/Editor/ColorBrush.h"

#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "Terrain/Editor/IFallOff.h"

namespace traktor::terrain
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.ColorBrush", ColorBrush, IBrush)

ColorBrush::ColorBrush(TerrainEditData* data)
	: m_data(data)
	, m_radius(0)
	, m_fallOff(0)
	, m_strength(0.0f)
{
}

uint32_t ColorBrush::begin(float x, float y, const State& state)
{
	m_radius = state.radius;
	m_fallOff = state.falloff;
	m_strength = powf(abs(state.strength), 2.0f);
	m_color = state.color;
	m_colors.clear();
	return MdColor;
}

void ColorBrush::apply(float x, float y)
{
	const int32_t size = m_data->getSize();
	for (int32_t iy = -m_radius; iy <= m_radius; ++iy)
	{
		for (int32_t ix = -m_radius; ix <= m_radius; ++ix)
		{
			float fx = float(ix) / m_radius;
			float fy = float(iy) / m_radius;

			float a = m_fallOff->evaluate(fx, fy) * m_strength;
			if (abs(a) <= FUZZY_EPSILON)
				continue;

			const int32_t gx = (int32_t)(x + ix);
			const int32_t gy = (int32_t)(y + iy);
			if (gx < 0 || gy < 0 || gx >= size || gy >= size)
				continue;

			// Accumulate unquantized since a step can be smaller than color resolution.
			Color4f targetColor;
			auto it = m_colors.find(gx + gy * size);
			if (it != m_colors.end())
				targetColor = it->second;
			else if (!m_data->getColor(gx, gy, targetColor))
				continue;

			const Scalar alpha = targetColor.getAlpha();
			targetColor = targetColor * Scalar(1.0f - a) + m_color * Scalar(a);
			targetColor.setAlpha(alpha);
			targetColor = targetColor.saturated();

			if (it != m_colors.end())
				it->second = targetColor;
			else
				m_colors.insert(gx + gy * size, targetColor);

			m_data->setColor(gx, gy, targetColor);
		}
	}
}

void ColorBrush::end(float x, float y)
{
	m_colors.clear();
}

}
