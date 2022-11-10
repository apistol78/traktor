/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Const.h"
#include "Core/Math/Random.h"
#include "Heightfield/Heightfield.h"
#include "Terrain/Editor/NoiseBrush.h"
#include "Terrain/Editor/IFallOff.h"

namespace traktor
{
	namespace terrain
	{
		namespace
		{

Random g_random;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.NoiseBrush", NoiseBrush, IBrush)

NoiseBrush::NoiseBrush(const resource::Proxy< hf::Heightfield >& heightfield)
:	m_heightfield(heightfield)
,	m_radius(0)
,	m_fallOff(0)
,	m_strength(0.0f)
{
}

uint32_t NoiseBrush::begin(float x, float y, const State& state)
{
	m_radius = state.radius;
	m_fallOff = state.falloff;
	m_strength = state.strength / m_heightfield->getWorldExtent().y();
	return MdHeight;
}

void NoiseBrush::apply(float x, float y)
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

			float h = m_heightfield->getGridHeightNearest(x + ix, y + iy);
			m_heightfield->setGridHeight(x + ix, y + iy, h + (g_random.nextFloat() * 2.0f - 1.0f) * a);
		}
	}
}

void NoiseBrush::end(float x, float y)
{
}

	}
}
