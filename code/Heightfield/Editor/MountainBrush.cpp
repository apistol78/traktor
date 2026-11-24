/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Heightfield/Editor/MountainBrush.h"

#include "Core/Math/Float.h"
#include "Core/Math/MathUtils.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

#include <cmath>

namespace traktor::hf
{
namespace
{

const int32_t c_maxOctaves = 16;
const float c_gradients[][2] = { { 1.0f, 0.0f }, { -1.0f, 0.0f }, { 0.0f, 1.0f }, { 0.0f, -1.0f }, { 0.7071f, 0.7071f }, { -0.7071f, 0.7071f }, { 0.7071f, -0.7071f }, { -0.7071f, -0.7071f } };

uint32_t hash(int32_t x, int32_t z, uint32_t seed)
{
	uint32_t h = seed ^ ((uint32_t)x * 0x8da6b343u) ^ ((uint32_t)z * 0xd8163841u);
	h ^= h >> 13;
	h *= 0x5bd1e995u;
	h ^= h >> 15;
	return h;
}

//! Gradient noise, -1 to 1.
float gradientNoise(float x, float z, uint32_t seed)
{
	const float fx = std::floor(x);
	const float fz = std::floor(z);
	const float dx = x - fx;
	const float dz = z - fz;

	float n[4];
	for (int32_t i = 0; i < 4; ++i)
	{
		const int32_t ox = i & 1;
		const int32_t oz = i >> 1;
		const float* g = c_gradients[hash((int32_t)fx + ox, (int32_t)fz + oz, seed) & 7];
		n[i] = g[0] * (dx - ox) + g[1] * (dz - oz);
	}

	const float u = dx * dx * dx * (dx * (dx * 6.0f - 15.0f) + 10.0f);
	const float v = dz * dz * dz * (dz * (dz * 6.0f - 15.0f) + 10.0f);
	return lerp(lerp(n[0], n[1], u), lerp(n[2], n[3], u), v) * 1.4142f;
}

//! Ridged multifractal noise, 0 to 1, octaves down to twice spacing; finer octaves are weighted by coarser so valleys stay smooth.
float ridgedNoise(float x, float z, float scale, float spacing, uint32_t seed)
{
	float frequency = 1.0f / scale;
	float amplitude = 1.0f;
	float weight = 1.0f;
	float sum = 0.0f;
	float total = 0.0f;
	for (int32_t i = 0; i < c_maxOctaves && (i == 0 || frequency * spacing <= 0.5f); ++i)
	{
		float n = 1.0f - std::abs(gradientNoise(x * frequency, z * frequency, seed + i));
		n *= n * weight;
		weight = clamp(n * 2.0f, 0.0f, 1.0f);
		sum += n * amplitude;
		total += amplitude;
		amplitude *= 0.5f;
		frequency *= 2.0f;
	}
	return sum / total;
}

}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.hf.MountainBrush", 0, MountainBrush, ProceduralBrush)

void MountainBrush::serialize(ISerializer& s)
{
	s >> Member< float >(L"height", m_height, AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"width", m_width, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"scale", m_scale, AttributeRange(1.0f) | AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"roughness", m_roughness, AttributeRange(0.0f, 1.0f));
	s >> Member< uint32_t >(L"seed", m_seed);
}

float MountainBrush::sample(float worldX, float worldZ, float spacing, float height, float distance, float layerHeight) const
{
	// Mountain rise smoothly from edge of layer.
	const float t = (m_width > 0.0f) ? clamp(distance / m_width, 0.0f, 1.0f) : 1.0f;
	const float envelope = t * t * (3.0f - 2.0f * t);
	const float ridges = ridgedNoise(worldX, worldZ, std::max(m_scale, 1.0f), spacing, m_seed);
	return height + m_height * envelope * lerp(1.0f, ridges, m_roughness);
}

}
