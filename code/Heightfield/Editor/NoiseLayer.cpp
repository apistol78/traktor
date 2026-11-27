/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Heightfield/Editor/NoiseLayer.h"

#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Heightfield/Editor/Noise.h"
#include "Heightfield/Editor/TileStorage.h"
#include "Heightfield/Heightfield.h"

namespace traktor::hf
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.hf.NoiseLayer", 0, NoiseLayer, HeightfieldAssetLayer)

bool NoiseLayer::apply(const TileStorage* storage, int32_t layer, const Vector4& worldExtent, int32_t tileX, int32_t tileZ, Heightfield* inoutTile) const
{
	if (m_scale <= 0.0f)
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
			const float noise = m_offset + m_amplitude * fbmNoise(worldX / m_scale, worldZ / m_scale, m_octaves, (uint32_t)m_seed);
			const float height = inoutTile->unitToWorld(inoutTile->getGridHeightNearestUnsafe(x, z));
			inoutTile->setGridHeight(x, z, inoutTile->worldToUnit(height + noise * m / 65535.0f));
		}
	}

	return true;
}

Ref< Heightfield > NoiseLayer::read(const TileStorage* storage, int32_t layer, const Vector4& worldExtent, int32_t tileX, int32_t tileZ, AlignedVector< uint16_t >* outMask) const
{
	// Noise is added onto layers below, thus layer alone is an empty tile.
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

bool NoiseLayer::haveMask() const
{
	return true;
}

void NoiseLayer::serialize(ISerializer& s)
{
	HeightfieldAssetLayer::serialize(s);
	s >> Member< int32_t >(L"seed", m_seed);
	s >> Member< float >(L"scale", m_scale, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));
	s >> Member< int32_t >(L"octaves", m_octaves, AttributeRange(1, 12));
	s >> Member< float >(L"amplitude", m_amplitude, AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"offset", m_offset, AttributeUnit(UnitType::Metres));
}

}
