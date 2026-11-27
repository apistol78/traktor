/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace traktor::hf
{

/*! Pseudo random hash of lattice point.
 * \ingroup Heightfield
 */
inline uint32_t noiseHash(int32_t x, int32_t z, uint32_t seed)
{
	uint32_t h = (uint32_t)x * 0x8da6b343u ^ (uint32_t)z * 0xd8163841u ^ seed * 0xcb1ab31fu;
	h ^= h >> 13;
	h *= 0x85ebca6bu;
	h ^= h >> 16;
	return h;
}

/*! Gradient noise, roughly in [-0.7, 0.7], with one lattice point per unit.
 * \ingroup Heightfield
 */
inline float gradientNoise(float x, float z, uint32_t seed)
{
	const float fx = std::floor(x);
	const float fz = std::floor(z);
	const int32_t ix = (int32_t)fx;
	const int32_t iz = (int32_t)fz;
	const float dx = x - fx;
	const float dz = z - fz;
	const float u = dx * dx * dx * (dx * (dx * 6.0f - 15.0f) + 10.0f);
	const float v = dz * dz * dz * (dz * (dz * 6.0f - 15.0f) + 10.0f);

	float g[4];
	const int32_t cx[] = { 0, 1, 0, 1 };
	const int32_t cz[] = { 0, 0, 1, 1 };
	for (int32_t i = 0; i < 4; ++i)
	{
		const float a = (noiseHash(ix + cx[i], iz + cz[i], seed) & 0xffff) * (6.2831853f / 65536.0f);
		g[i] = std::cos(a) * (dx - cx[i]) + std::sin(a) * (dz - cz[i]);
	}

	const float ab = g[0] + (g[1] - g[0]) * u;
	const float cd = g[2] + (g[3] - g[2]) * u;
	return ab + (cd - ab) * v;
}

/*! Fractal sum of gradient noise, in [-1, 1], of octaves which halve in amplitude.
 * \ingroup Heightfield
 */
inline float fbmNoise(float x, float z, int32_t octaves, uint32_t seed)
{
	float sum = 0.0f, amplitude = 1.0f, amplitudes = 0.0f;
	for (int32_t i = 0; i < octaves; ++i)
	{
		sum += amplitude * gradientNoise(x, z, seed + i * 17);
		amplitudes += amplitude;
		amplitude *= 0.5f;
		x = x * 2.03f + 13.7f;
		z = z * 2.03f + 7.3f;
	}
	return (amplitudes > 0.0f) ? std::clamp(1.4f * sum / amplitudes, -1.0f, 1.0f) : 0.0f;
}

/*! Ridged fractal noise, in [0, 1], sharp ridges where gradient noise cross zero; finer octaves weighted by coarser.
 * \ingroup Heightfield
 */
inline float ridgedNoise(float x, float z, int32_t octaves, uint32_t seed)
{
	float sum = 0.0f, amplitude = 1.0f, amplitudes = 0.0f, weight = 1.0f;
	for (int32_t i = 0; i < octaves; ++i)
	{
		float n = std::clamp(1.0f - std::abs(gradientNoise(x, z, seed + i * 31) * 1.4f), 0.0f, 1.0f);
		n = n * n * weight;
		weight = std::clamp(n * 1.8f, 0.0f, 1.0f);
		sum += n * amplitude;
		amplitudes += amplitude;
		amplitude *= 0.5f;
		x = x * 2.07f + 3.1f;
		z = z * 2.07f + 9.4f;
	}
	return (amplitudes > 0.0f) ? sum / amplitudes : 0.0f;
}

}
