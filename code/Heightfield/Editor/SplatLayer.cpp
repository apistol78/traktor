/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Heightfield/Editor/SplatLayer.h"

#include "Core/Math/Color4f.h"
#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Heightfield/Editor/TileStorage.h"
#include "Heightfield/Heightfield.h"

#include <algorithm>
#include <cmath>

namespace traktor::hf
{
namespace
{

//! One within range, fading smoothly to zero at blend outside of range.
float rangeWeight(float value, float mn, float mx, float blend)
{
	const float outside = std::max(mn - value, value - mx);
	if (outside <= 0.0f)
		return 1.0f;
	if (outside >= blend)
		return 0.0f;
	const float t = 1.0f - outside / blend;
	return t * t * (3.0f - 2.0f * t);
}

float worldHeight(const Heightfield* heightfield, int32_t x, int32_t z)
{
	return heightfield->unitToWorld(heightfield->getGridHeightNearestUnsafe(x, z));
}

}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.hf.SplatLayer", 0, SplatLayer, HeightfieldAssetLayer)

void SplatLayer::Rule::serialize(ISerializer& s)
{
	s >> Member< int32_t >(L"channel", channel, AttributeRange(0.0f, 3.0f));
	s >> Member< float >(L"minHeight", minHeight, AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"maxHeight", maxHeight, AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"heightBlend", heightBlend, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"minSlope", minSlope, AttributeRange(0.0f, 90.0f) | AttributeUnit(UnitType::Degrees));
	s >> Member< float >(L"maxSlope", maxSlope, AttributeRange(0.0f, 90.0f) | AttributeUnit(UnitType::Degrees));
	s >> Member< float >(L"slopeBlend", slopeBlend, AttributeRange(0.0f) | AttributeUnit(UnitType::Degrees));
}

bool SplatLayer::apply(const TileStorage* storage, int32_t layer, const Vector4& worldExtent, int32_t tileX, int32_t tileZ, Heightfield* inoutTile) const
{
	if (m_rules.empty())
		return true;

	const int32_t size = inoutTile->getSize();
	AlignedVector< uint16_t > mask;
	if (!storage->readStoredMask(layer, tileX, tileZ, size, mask))
	{
		mask.resize(0);
		mask.resize((size_t)size * size, 65535);
	}

	float x0, z0, x1, z1;
	inoutTile->gridToWorld(0, 0, x0, z0);
	inoutTile->gridToWorld(1, 1, x1, z1);
	const float spacingX = x1 - x0;
	const float spacingZ = z1 - z0;

	for (int32_t z = 0; z < size; ++z)
	{
		for (int32_t x = 0; x < size; ++x)
		{
			const uint16_t m = mask[x + (size_t)z * size];
			if (m == 0)
				continue;

			// Slope from central differences, one sided at edges of tile.
			const int32_t xa = std::max(x - 1, 0);
			const int32_t xb = std::min(x + 1, size - 1);
			const int32_t za = std::max(z - 1, 0);
			const int32_t zb = std::min(z + 1, size - 1);
			const float gx = (worldHeight(inoutTile, xb, z) - worldHeight(inoutTile, xa, z)) / ((xb - xa) * spacingX);
			const float gz = (worldHeight(inoutTile, x, zb) - worldHeight(inoutTile, x, za)) / ((zb - za) * spacingZ);
			const float slope = rad2deg(std::atan(std::sqrt(gx * gx + gz * gz)));
			const float height = worldHeight(inoutTile, x, z);

			float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
			float total = 0.0f;
			for (const Rule& rule : m_rules)
			{
				const float w = rangeWeight(height, rule.minHeight, rule.maxHeight, rule.heightBlend) * rangeWeight(slope, rule.minSlope, rule.maxSlope, rule.slopeBlend);
				weights[clamp(rule.channel, 0, 3)] += w;
				total += w;
			}
			if (total <= 0.0f)
				continue;

			// Partly covered samples are blended with their splat, as are partly masked.
			const Color4f splat = Color4f(weights[0], weights[1], weights[2], weights[3]) / Scalar(total);
			const float blend = std::min(total, 1.0f) * m / 65535.0f;
			inoutTile->setGridSplat(x, z, lerp(inoutTile->getGridSplat(x, z), splat, Scalar(blend)));
		}
	}

	return true;
}

Ref< Heightfield > SplatLayer::read(const TileStorage* storage, int32_t layer, const Vector4& worldExtent, int32_t tileX, int32_t tileZ, AlignedVector< uint16_t >* outMask) const
{
	// Splats depend on layers below, thus layer alone is an empty tile.
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

bool SplatLayer::haveMask() const
{
	return true;
}

void SplatLayer::serialize(ISerializer& s)
{
	HeightfieldAssetLayer::serialize(s);
	s >> MemberAlignedVector< Rule, MemberComposite< Rule > >(L"rules", m_rules);
}

}
