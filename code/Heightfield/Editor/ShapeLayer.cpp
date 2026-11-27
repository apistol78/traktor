/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Heightfield/Editor/ShapeLayer.h"

#include "Core/Math/Float.h"
#include "Core/Math/MathUtils.h"
#include "Core/Math/Vector2.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Heightfield/Editor/TileStorage.h"
#include "Heightfield/Heightfield.h"

#include <algorithm>
#include <cmath>

namespace traktor::hf
{
namespace
{

const int32_t c_subdivisions = 16;	//!< Line segments per pair of keys.

//! Catmull-Rom spline between p1 and p2.
Vector4 catmullRom(const Vector4& p0, const Vector4& p1, const Vector4& p2, const Vector4& p3, float t)
{
	const Scalar t1(t);
	const Scalar t2(t * t);
	const Scalar t3(t * t * t);
	return (
		p1 * Scalar(2.0f) +
		(p2 - p0) * t1 +
		(p0 * Scalar(2.0f) - p1 * Scalar(5.0f) + p2 * Scalar(4.0f) - p3) * t2 +
		(p3 - p0 + (p1 - p2) * Scalar(3.0f)) * t3
	) * Scalar(0.5f);
}

//! Distance from point to segment a-b, and fraction along segment of closest point.
float segmentDistance(const Vector2& a, const Vector2& b, const Vector2& p, float& outT)
{
	const Vector2 ab = b - a;
	const float ln2 = dot(ab, ab);
	outT = (ln2 > 0.0f) ? clamp(dot(p - a, ab) / ln2, 0.0f, 1.0f) : 0.0f;
	return (a + ab * outT - p).length();
}

//! Smooth weight; one inside of path, fading to zero at falloff distance outside.
float falloffWeight(float distance, float falloff)
{
	if (distance >= 0.0f)
		return 1.0f;
	const float t = 1.0f + distance / falloff;
	return t * t * (3.0f - 2.0f * t);
}

}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.hf.ShapeLayer", 0, ShapeLayer, HeightfieldAssetLayer)

bool ShapeLayer::apply(const TileStorage* storage, int32_t layer, const Vector4& worldExtent, int32_t tileX, int32_t tileZ, Heightfield* heightfield) const
{
	const int32_t nkeys = (int32_t)m_keys.size();
	if (nkeys < 2)
		return true;

	// Spline through keys is tessellated into line segments.
	AlignedVector< Vector4 > points;
	points.reserve((nkeys - 1) * c_subdivisions + 1);
	for (int32_t i = 0; i < nkeys - 1; ++i)
	{
		const Vector4& p0 = m_keys[std::max(i - 1, 0)];
		const Vector4& p1 = m_keys[i];
		const Vector4& p2 = m_keys[i + 1];
		const Vector4& p3 = m_keys[std::min(i + 2, nkeys - 1)];
		for (int32_t j = 0; j < c_subdivisions; ++j)
			points.push_back(catmullRom(p0, p1, p2, p3, (float)j / c_subdivisions));
	}
	points.push_back(m_keys.back());

	const int32_t size = heightfield->getSize();
	const float halfWidth = m_width * 0.5f;
	const float radius = halfWidth + m_falloff;

	// Signed distance to edge of path, positive inside, and height of path at closest point; one per sample of a row.
	AlignedVector< float > distances(size);
	AlignedVector< float > pathHeights(size);
	for (int32_t z = 0; z < size; ++z)
	{
		std::fill(distances.begin(), distances.end(), -m_falloff);

		float worldX, worldZ;
		heightfield->gridToWorld(0, z, worldX, worldZ);

		for (size_t i = 0; i + 1 < points.size(); ++i)
		{
			const Vector2 a(points[i].x(), points[i].z());
			const Vector2 b(points[i + 1].x(), points[i + 1].z());
			if (worldZ < std::min(a.y, b.y) - radius || worldZ > std::max(a.y, b.y) + radius)
				continue;

			// Only samples within reach of segment are evaluated.
			float gridX0, gridX1, gridZ0;
			heightfield->worldToGrid(std::min(a.x, b.x) - radius, worldZ, gridX0, gridZ0);
			heightfield->worldToGrid(std::max(a.x, b.x) + radius, worldZ, gridX1, gridZ0);
			const int32_t x0 = std::max((int32_t)std::floor(gridX0), 0);
			const int32_t x1 = std::min((int32_t)std::ceil(gridX1), size - 1);

			for (int32_t x = x0; x <= x1; ++x)
			{
				heightfield->gridToWorld(x, z, worldX, worldZ);

				float t;
				const float distance = halfWidth - segmentDistance(a, b, Vector2(worldX, worldZ), t);
				if (distance > distances[x])
				{
					distances[x] = distance;
					pathHeights[x] = lerp((float)points[i].y(), (float)points[i + 1].y(), t);
				}
			}
		}

		for (int32_t x = 0; x < size; ++x)
		{
			if (distances[x] <= -m_falloff)
				continue;

			const float height = heightfield->unitToWorld(heightfield->getGridHeightNearestUnsafe(x, z));
			heightfield->setGridHeight(x, z, heightfield->worldToUnit(lerp(height, pathHeights[x], falloffWeight(distances[x], m_falloff))));
		}
	}

	return true;
}

Ref< Heightfield > ShapeLayer::read(const TileStorage* storage, int32_t layer, const Vector4& worldExtent, int32_t tileX, int32_t tileZ, AlignedVector< uint16_t >* outMask) const
{
	// Path is stroked onto an empty tile; mask cover changed samples.
	Ref< Heightfield > tile = storage->createTile(worldExtent, tileX, tileZ);
	if (!tile)
		return nullptr;

	const height_t flat = tile->getHeights()[0];
	apply(storage, layer, worldExtent, tileX, tileZ, tile);
	tile->updateCellBounds();

	if (outMask)
	{
		const size_t samples = (size_t)tile->getSize() * tile->getSize();
		outMask->resize(samples);
		for (size_t i = 0; i < samples; ++i)
			(*outMask)[i] = (tile->getHeights()[i] != flat) ? 65535 : 0;
	}

	return tile;
}

void ShapeLayer::serialize(ISerializer& s)
{
	HeightfieldAssetLayer::serialize(s);
	s >> MemberAlignedVector< Vector4 >(L"keys", m_keys);
	s >> Member< float >(L"width", m_width, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"falloff", m_falloff, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));
}

}
