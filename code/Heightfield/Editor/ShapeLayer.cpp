/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Heightfield/Editor/ShapeLayer.h"

#include "Core/Math/MathUtils.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Heightfield/Heightfield.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace traktor::hf
{
namespace
{

//! Squared distance from point to segment a-b.
float segmentDistance2(const Vector2& a, const Vector2& b, const Vector2& p)
{
	const Vector2 ab = b - a;
	const float ln2 = dot(ab, ab);
	const float t = (ln2 > 0.0f) ? clamp(dot(p - a, ab) / ln2, 0.0f, 1.0f) : 0.0f;
	return (a + ab * t - p).length2();
}

}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.hf.ShapeLayer", 0, ShapeLayer, ProceduralLayer)

void ShapeLayer::evaluate(const Heightfield* heightfield, int32_t gridZ, float reach, float* outDistances, float* outHeights) const
{
	const int32_t size = heightfield->getSize();

	std::fill(outDistances, outDistances + size, -reach);
	std::fill(outHeights, outHeights + size, 0.0f);

	if (m_points.size() < 3)
		return;

	// Only samples within bounds of shape, extended by reach, are evaluated.
	Vector2 mn = m_points[0];
	Vector2 mx = m_points[0];
	for (const Vector2& point : m_points)
	{
		mn = Vector2(std::min(mn.x, point.x), std::min(mn.y, point.y));
		mx = Vector2(std::max(mx.x, point.x), std::max(mx.y, point.y));
	}

	float worldX, worldZ;
	heightfield->gridToWorld(0, gridZ, worldX, worldZ);
	if (worldZ < mn.y - reach || worldZ > mx.y + reach)
		return;

	float gridX0, gridX1, gridZ0;
	heightfield->worldToGrid(mn.x - reach, worldZ, gridX0, gridZ0);
	heightfield->worldToGrid(mx.x + reach, worldZ, gridX1, gridZ0);
	const int32_t x0 = std::max((int32_t)std::floor(gridX0), 0);
	const int32_t x1 = std::min((int32_t)std::ceil(gridX1), size - 1);

	for (int32_t x = x0; x <= x1; ++x)
	{
		heightfield->gridToWorld(x, gridZ, worldX, worldZ);
		const Vector2 p(worldX, worldZ);

		// Inside by crossing number, distance to closest edge.
		bool inside = false;
		float distance2 = std::numeric_limits< float >::max();
		for (size_t i = 0, j = m_points.size() - 1; i < m_points.size(); j = i++)
		{
			const Vector2& a = m_points[j];
			const Vector2& b = m_points[i];
			if ((a.y > p.y) != (b.y > p.y) && p.x < a.x + (p.y - a.y) * (b.x - a.x) / (b.y - a.y))
				inside = !inside;
			distance2 = std::min(distance2, segmentDistance2(a, b, p));
		}

		const float distance = std::sqrt(distance2);
		outDistances[x] = inside ? distance : std::max(-distance, -reach);
	}
}

void ShapeLayer::serialize(ISerializer& s)
{
	ProceduralLayer::serialize(s);
	s >> MemberAlignedVector< Vector2 >(L"points", m_points);
}

}
