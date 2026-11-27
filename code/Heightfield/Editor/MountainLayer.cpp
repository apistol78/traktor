/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Heightfield/Editor/MountainLayer.h"

#include "Core/Math/MathUtils.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Heightfield/Editor/Noise.h"
#include "Heightfield/Editor/TileStorage.h"
#include "Heightfield/Heightfield.h"

#include <cmath>

namespace traktor::hf
{
namespace
{

const int32_t c_coverageOctaves = 3;

}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.hf.MountainLayer", 0, MountainLayer, HeightfieldAssetLayer)

bool MountainLayer::apply(const TileStorage* storage, int32_t layer, const Vector4& worldExtent, int32_t tileX, int32_t tileZ, Heightfield* inoutTile) const
{
	if (m_scale <= 0.0f || m_coverageScale <= 0.0f)
		return true;

	const int32_t size = inoutTile->getSize();
	AlignedVector< uint16_t > mask;
	if (!storage->readStoredMask(layer, tileX, tileZ, size, mask))
	{
		mask.resize(0);
		mask.resize((size_t)size * size, 65535);
	}

	for (int32_t z = 0; z < size; ++z)
	{
		for (int32_t x = 0; x < size; ++x)
		{
			const uint16_t m = mask[x + (size_t)z * size];
			if (m == 0)
				continue;

			float worldX, worldZ;
			inoutTile->gridToWorld(x, z, worldX, worldZ);
			const float coverage = clamp(m_coverage + m_coverageVariation * fbmNoise(worldX / m_coverageScale, worldZ / m_coverageScale, c_coverageOctaves, (uint32_t)m_coverageSeed), 0.0f, 1.0f);
			const float ridge = ridgedNoise(worldX / m_scale, worldZ / m_scale, m_octaves, (uint32_t)m_seed);
			const float below = inoutTile->unitToWorld(inoutTile->getGridHeightNearestUnsafe(x, z));

			float height = below + m_height * coverage * ridge * ridge;
			if (height > m_capHeight)
				height = (m_capSoftness > 0.0f) ? m_capHeight + m_capSoftness * std::tanh((height - m_capHeight) / m_capSoftness) : m_capHeight;

			inoutTile->setGridHeight(x, z, inoutTile->worldToUnit(lerp(below, height, m / 65535.0f)));
		}
	}

	return true;
}

Ref< Heightfield > MountainLayer::read(const TileStorage* storage, int32_t layer, const Vector4& worldExtent, int32_t tileX, int32_t tileZ, AlignedVector< uint16_t >* outMask) const
{
	// Mountains are added onto layers below, thus layer alone is an empty tile.
	Ref< Heightfield > tile = storage->createTile(worldExtent, tileX, tileZ);
	if (!tile)
		return nullptr;

	const int32_t size = tile->getSize();
	if (outMask && !storage->readStoredMask(layer, tileX, tileZ, size, *outMask))
	{
		outMask->resize(0);
		outMask->resize((size_t)size * size, 65535);
	}

	return tile;
}

bool MountainLayer::haveMask() const
{
	return true;
}

void MountainLayer::serialize(ISerializer& s)
{
	HeightfieldAssetLayer::serialize(s);
	s >> Member< int32_t >(L"seed", m_seed);
	s >> Member< float >(L"scale", m_scale, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));
	s >> Member< int32_t >(L"octaves", m_octaves, AttributeRange(1, 12));
	s >> Member< float >(L"height", m_height, AttributeUnit(UnitType::Metres));
	s >> Member< int32_t >(L"coverageSeed", m_coverageSeed);
	s >> Member< float >(L"coverageScale", m_coverageScale, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"coverage", m_coverage, AttributeRange(0.0f, 1.0f));
	s >> Member< float >(L"coverageVariation", m_coverageVariation, AttributeRange(0.0f, 1.0f));
	s >> Member< float >(L"capHeight", m_capHeight, AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"capSoftness", m_capSoftness, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));
}

}
