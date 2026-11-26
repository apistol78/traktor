/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Heightfield/Editor/TerraceLayer.h"

#include "Core/Math/MathUtils.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Heightfield/Editor/TileStorage.h"
#include "Heightfield/Heightfield.h"

#include <algorithm>
#include <cmath>

namespace traktor::hf
{
namespace
{

const int32_t c_noiseOctaves = 3;

//! Riser in middle of step and flat treads at either end, steeper by exponent; one keep t.
float gain(float t, float exponent)
{
	return (t < 0.5f) ? 0.5f * std::pow(2.0f * t, exponent) : 1.0f - 0.5f * std::pow(2.0f - 2.0f * t, exponent);
}

//! Pseudo random value, in [-1, 1], of lattice point.
float lattice(int32_t x, int32_t z)
{
	uint32_t h = (uint32_t)x * 0x8da6b343u ^ (uint32_t)z * 0xd8163841u;
	h = (h ^ (h >> 13)) * 0x85ebca6bu;
	h ^= h >> 16;
	return (h & 0xffffff) / 8388607.5f - 1.0f;
}

//! Smooth value noise, in [-1, 1], of octaves which halve in size and amplitude; largest octave has one lattice point per unit.
float noise(float x, float z)
{
	float sum = 0.0f;
	float amplitude = 1.0f;
	float amplitudes = 0.0f;
	for (int32_t octave = 0; octave < c_noiseOctaves; ++octave)
	{
		const float fx = std::floor(x);
		const float fz = std::floor(z);
		const int32_t ix = (int32_t)fx + octave * 1013;
		const int32_t iz = (int32_t)fz;
		const float tx = (x - fx) * (x - fx) * (3.0f - 2.0f * (x - fx));
		const float tz = (z - fz) * (z - fz) * (3.0f - 2.0f * (z - fz));
		sum += amplitude * lerp(lerp(lattice(ix, iz), lattice(ix + 1, iz), tx), lerp(lattice(ix, iz + 1), lattice(ix + 1, iz + 1), tx), tz);
		amplitudes += amplitude;
		amplitude *= 0.5f;
		x *= 2.0f;
		z *= 2.0f;
	}
	return sum / amplitudes;
}

}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.hf.TerraceLayer", 1, TerraceLayer, HeightfieldAssetLayer)

bool TerraceLayer::apply(const TileStorage* storage, int32_t layer, const Vector4& worldExtent, int32_t tileX, int32_t tileZ, Heightfield* inoutTile) const
{
	if (m_stepHeight <= 0.0f || m_sharpness <= 0.0f)
		return true;

	const int32_t size = inoutTile->getSize();
	AlignedVector< uint8_t > mask;
	if (!storage->readStoredMask(layer, tileX, tileZ, size, mask))
	{
		mask.resize(0);
		mask.resize((size_t)size * size, 255);
	}

	const float exponent = 1.0f / std::max(1.0f - m_sharpness, 0.01f);

	for (int32_t z = 0; z < size; ++z)
	{
		for (int32_t x = 0; x < size; ++x)
		{
			const uint8_t m = mask[x + (size_t)z * size];
			if (m == 0)
				continue;

			// Active within range of heights, faded out over one step outside of range.
			const float height = inoutTile->unitToWorld(inoutTile->getGridHeightNearestUnsafe(x, z));
			const float inside = clamp(1.0f - std::max(m_minHeight - height, height - m_maxHeight) / m_stepHeight, 0.0f, 1.0f);
			if (inside <= 0.0f)
				continue;

			// Terrace levels are shifted by noise at sample's position in world.
			float shift = 0.0f;
			if (m_noise > 0.0f && m_noiseScale > 0.0f)
			{
				float worldX, worldZ;
				inoutTile->gridToWorld(x, z, worldX, worldZ);
				shift = m_noise * noise(worldX / m_noiseScale, worldZ / m_noiseScale);
			}

			// Height within step, from one terrace level to next, is reshaped.
			const float step = (height + shift - m_offset) / m_stepHeight;
			const float level = std::floor(step);
			const float terraced = m_offset + (level + gain(step - level, exponent)) * m_stepHeight - shift;
			const float weight = inside * inside * (3.0f - 2.0f * inside) * m / 255.0f;
			inoutTile->setGridHeight(x, z, inoutTile->worldToUnit(lerp(height, terraced, weight)));
		}
	}

	return true;
}

Ref< Heightfield > TerraceLayer::read(const TileStorage* storage, int32_t layer, const Vector4& worldExtent, int32_t tileX, int32_t tileZ, AlignedVector< uint8_t >* outMask) const
{
	// Terraces depend on layers below, thus layer alone is an empty tile.
	Ref< Heightfield > tile = storage->createTile(worldExtent, tileX, tileZ);
	if (!tile)
		return nullptr;

	const int32_t size = tile->getSize();
	if (outMask && !storage->readStoredMask(layer, tileX, tileZ, size, *outMask))
	{
		outMask->resize(0);
		outMask->resize((size_t)size * size, 255);
	}

	return tile;
}

bool TerraceLayer::haveMask() const
{
	return true;
}

void TerraceLayer::serialize(ISerializer& s)
{
	HeightfieldAssetLayer::serialize(s);
	s >> Member< float >(L"stepHeight", m_stepHeight, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"offset", m_offset, AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"sharpness", m_sharpness, AttributeRange(0.0f, 1.0f));

	if (s.getVersion< TerraceLayer >() >= 1)
	{
		s >> Member< float >(L"noise", m_noise, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));
		s >> Member< float >(L"noiseScale", m_noiseScale, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));
		s >> Member< float >(L"minHeight", m_minHeight, AttributeUnit(UnitType::Metres));
		s >> Member< float >(L"maxHeight", m_maxHeight, AttributeUnit(UnitType::Metres));
	}
}

}
