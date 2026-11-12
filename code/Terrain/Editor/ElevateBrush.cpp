/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Terrain/Editor/ElevateBrush.h"

#include "Core/Math/Const.h"
#include "Terrain/Editor/IFallOff.h"

namespace traktor::terrain
{
namespace
{

double fraction(double n)
{
	return n - std::floor(n);
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.ElevateBrush", ElevateBrush, IBrush)

ElevateBrush::ElevateBrush(TerrainEditData* data)
	: m_data(data)
	, m_radius(0)
	, m_fallOff(0)
	, m_strength(0.0)
	, m_attribute(-1)
{
}

uint32_t ElevateBrush::begin(float x, float y, const State& state)
{
	m_radius = state.radius;
	m_fallOff = state.falloff;
	m_strength = state.strength * 0.0256f / m_data->getWorldExtent().y();
	m_attribute = state.attribute;
	m_heights.clear();
	return MdHeight;
}

void ElevateBrush::apply(float x, float y)
{
	const int32_t size = m_data->getSize();
	for (int32_t iy = -m_radius; iy <= m_radius; ++iy)
	{
		for (int32_t ix = -m_radius; ix <= m_radius; ++ix)
		{
			const double fx = (double)ix / m_radius;
			const double fy = (double)iy / m_radius;

			const double a = m_fallOff->evaluate(fx, fy) * m_strength;

			const double gx = x + ix;
			const double gy = y + iy;

			const int32_t igx = (int32_t)gx;
			const int32_t igy = (int32_t)gy;
			if (gx < 0.0 || gy < 0.0 || igx >= size || igy >= size)
				continue;

			double wl = 1.0 - fraction(gx);
			double wt = 1.0 - fraction(gy);

			// Accumulate unquantized since a step can be smaller than height resolution.
			double value = 0.0;
			auto it = m_heights.find(igx + igy * size);
			if (it != m_heights.end())
			{
				it->second += a * (wl * wt);
				value = it->second;
			}
			else
			{
				value = m_data->getGridHeightNearest(igx, igy) + a * (wl * wt);
				m_heights.insert(igx + igy * size, value);
			}

			m_data->setGridHeight(igx, igy, (float)value);
		}
	}
}

void ElevateBrush::end(float x, float y)
{
	m_heights.clear();
}

}
