/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <numeric>
#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "Drawing/Image.h"
#include "Heightfield/Heightfield.h"
#include "Terrain/Editor/MaterialBrush.h"
#include "Terrain/Editor/IFallOff.h"

namespace traktor::terrain
{
	namespace
	{

const int32_t c_others[4][3] =
{
	{ 1, 2, 3 },
	{ 0, 2, 3 },
	{ 0, 1, 3 },
	{ 0, 1, 2 }
};

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.MaterialBrush", MaterialBrush, IBrush)

MaterialBrush::MaterialBrush(const resource::Proxy< hf::Heightfield >& heightfield, drawing::Image* splatImage)
:	m_heightfield(heightfield)
,	m_splatImage(splatImage)
,	m_radius(0)
,	m_fallOff(0)
,	m_strength(0.0f)
,	m_inverse(false)
{
}

uint32_t MaterialBrush::begin(float x, float y, const State& state)
{
	m_radius = state.radius;
	m_fallOff = state.falloff;
	m_strength = abs(state.strength) / 50.0f;
	m_material = state.material;
	m_inverse = (bool)(state.strength < 0.0f);
	return MdMaterial;
}

void MaterialBrush::apply(float x, float y)
{
	float T_MATH_ALIGN16 weights[4];
	Color4f targetColor;

	for (int32_t iy = -m_radius; iy <= m_radius; ++iy)
	{
		for (int32_t ix = -m_radius; ix <= m_radius; ++ix)
		{
			const float fx = float(ix) / m_radius;
			const float fy = float(iy) / m_radius;

			const float a = m_fallOff->evaluate(fx, fy) * m_strength;

			/*
			Vector4 normal = m_heightfield->normalAt(x + ix, y + iy);
			float slope = abs(acos(normal.y()) / HALF_PI);

			slope = slope * 2.0f - 0.5f;

			float fact = 1.0f / (1.0f + exp(-8.0f * (slope - 0.5f)));
			if (!m_inverse)
				a *= clamp(1.0f - fact, 0.0f, 1.0f);
			else
				a *= clamp(fact, 0.0f, 1.0f);
			*/

			if (abs(a) <= FUZZY_EPSILON)
				continue;

			m_splatImage->getPixel(x + ix, y + iy, targetColor);

			targetColor.storeAligned(weights);

			const float w = clamp(weights[m_material] + a, 0.0f, 1.0f);

			weights[m_material] = w;

			float s[3], st = 0.0f;
			for (int32_t i = 0; i < 3; ++i)
			{
				float& other = weights[c_others[m_material][i]];
				st += (s[i] = other);
			}
			if (st > FUZZY_EPSILON)
			{
				for (int32_t i = 0; i < 3; ++i)
				{
					float& other = weights[c_others[m_material][i]];
					other = clamp(other, 0.0f, (s[i] / st) * (1.0f - w));
				}
			}

			m_splatImage->setPixel(x + ix, y + iy, Color4f(weights));
		}
	}
}

void MaterialBrush::end(float x, float y)
{
}

}
