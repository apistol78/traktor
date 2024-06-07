/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Const.h"
#include "Drawing/Image.h"
#include "Heightfield/Heightfield.h"
#include "Terrain/Editor/ElevateBrush.h"
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

ElevateBrush::ElevateBrush(const resource::Proxy< hf::Heightfield >& heightfield, drawing::Image* splatImage)
:	m_heightfield(heightfield)
,	m_splatImage(splatImage)
,	m_radius(0)
,	m_fallOff(0)
,	m_strength(0.0)
,	m_attribute(-1)
{
}

uint32_t ElevateBrush::begin(float x, float y, const State& state)
{
	m_radius = state.radius;
	m_fallOff = state.falloff;
	m_strength = state.strength * 0.0256f / m_heightfield->getWorldExtent().y();
	m_attribute = state.attribute;
	return MdHeight;
}

void ElevateBrush::apply(float x, float y)
{
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

			const double h = m_heightfield->getGridHeightBilinear(gx, gy);

			double wl = 1.0 - fraction(gx);
			double wt = 1.0 - fraction(gy);

			m_heightfield->setGridHeight(igx, igy, h + a * (wl * wt));
		}
	}
}

void ElevateBrush::end(float x, float y)
{
}

}
