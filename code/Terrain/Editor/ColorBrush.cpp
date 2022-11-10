/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "Drawing/Image.h"
#include "Terrain/Editor/ColorBrush.h"
#include "Terrain/Editor/IFallOff.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.ColorBrush", ColorBrush, IBrush)

ColorBrush::ColorBrush(drawing::Image* colorImage)
:	m_colorImage(colorImage)
,	m_radius(0)
,	m_fallOff(0)
,	m_strength(0.0f)
{
}

uint32_t ColorBrush::begin(float x, float y, const State& state)
{
	m_radius = state.radius;
	m_fallOff = state.falloff;
	m_strength = powf(abs(state.strength), 2.0f);
	m_color = state.color;
	return MdColor;
}

void ColorBrush::apply(float x, float y)
{
	Color4f targetColor;
	for (int32_t iy = -m_radius; iy <= m_radius; ++iy)
	{
		for (int32_t ix = -m_radius; ix <= m_radius; ++ix)
		{
			float fx = float(ix) / m_radius;
			float fy = float(iy) / m_radius;

			float a = m_fallOff->evaluate(fx, fy) * m_strength;
			if (abs(a) <= FUZZY_EPSILON)
				continue;

			m_colorImage->getPixel(x + ix, y + iy, targetColor);

			Scalar alpha = targetColor.getAlpha();
			targetColor = targetColor * Scalar(1.0f - a) + m_color * Scalar(a);
			targetColor.setAlpha(alpha);

			m_colorImage->setPixel(x + ix, y + iy, targetColor.saturated());
		}
	}
}

void ColorBrush::end(float x, float y)
{
}

	}
}
