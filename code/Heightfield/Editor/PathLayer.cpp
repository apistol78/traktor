/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Heightfield/Editor/PathLayer.h"

#include "Core/Math/Float.h"
#include "Core/Math/MathUtils.h"
#include "Core/Math/Vector2.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAlignedVector.h"
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

}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.hf.PathLayer", 0, PathLayer, ProceduralLayer)

void PathLayer::evaluate(const Heightfield* heightfield, int32_t gridZ, float reach, float* outDistances, float* outHeights) const
{
	const int32_t size = heightfield->getSize();

	std::fill(outDistances, outDistances + size, -reach);
	std::fill(outHeights, outHeights + size, 0.0f);

	const int32_t nkeys = (int32_t)m_keys.size();
	if (nkeys < 2)
		return;

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

	const float halfWidth = m_width * 0.5f;
	const float radius = halfWidth + reach;

	float worldX, worldZ;
	heightfield->gridToWorld(0, gridZ, worldX, worldZ);

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
			heightfield->gridToWorld(x, gridZ, worldX, worldZ);

			float t;
			const float distance = halfWidth - segmentDistance(a, b, Vector2(worldX, worldZ), t);
			if (distance > outDistances[x])
			{
				outDistances[x] = distance;
				outHeights[x] = lerp((float)points[i].y(), (float)points[i + 1].y(), t);
			}
		}
	}
}

void PathLayer::serialize(ISerializer& s)
{
	ProceduralLayer::serialize(s);
	s >> MemberAlignedVector< Vector4 >(L"keys", m_keys);
	s >> Member< float >(L"width", m_width, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));
}

}
