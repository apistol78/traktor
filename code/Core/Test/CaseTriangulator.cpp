/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Triangulator.h"
#include "Core/Misc/String.h"
#include "Core/Test/CaseTriangulator.h"
#include "Core/Test/MathCompare.h"

namespace traktor::test
{
	namespace
	{

struct Triangle
{
	size_t indices[3];
};

AlignedVector< Triangle > freeze(const AlignedVector< Vector2 >& points)
{
	AlignedVector< Triangle > triangles;
	Triangulator().freeze(points, Triangulator::Mode::Sorted, [&](size_t i0, size_t i1, size_t i2) {
		triangles.push_back({ { i0, i1, i2 } });
	});
	return triangles;
}

AlignedVector< Triangle > freeze(const AlignedVector< Vector4 >& points, const Vector4& normal)
{
	AlignedVector< Triangle > triangles;
	Triangulator().freeze(points, normal, Triangulator::Mode::Sorted, [&](size_t i0, size_t i1, size_t i2) {
		triangles.push_back({ { i0, i1, i2 } });
	});
	return triangles;
}

std::wstring toString(const AlignedVector< Triangle >& triangles)
{
	std::wstring s;
	for (const auto& t : triangles)
		s += str(L"(%d,%d,%d)", int32_t(t.indices[0]), int32_t(t.indices[1]), int32_t(t.indices[2]));
	return s;
}

/*! Area of each triangle along normal, using model convention triangle normal cross(p2 - p0, p1 - p0). */
AlignedVector< float > areas(const AlignedVector< Vector4 >& points, const AlignedVector< Triangle >& triangles, const Vector4& normal)
{
	AlignedVector< float > result;
	for (const auto& t : triangles)
	{
		const Vector4& p0 = points[t.indices[0]];
		const Vector4& p1 = points[t.indices[1]];
		const Vector4& p2 = points[t.indices[2]];
		result.push_back(0.5f * dot3(cross(p2 - p0, p1 - p0), normal));
	}
	return result;
}

/*! Signed area of each triangle, positive if counter clockwise. */
AlignedVector< float > areas(const AlignedVector< Vector2 >& points, const AlignedVector< Triangle >& triangles)
{
	AlignedVector< float > result;
	for (const auto& t : triangles)
		result.push_back(0.5f * determinant(points[t.indices[1]] - points[t.indices[0]], points[t.indices[2]] - points[t.indices[0]]));
	return result;
}

float sum(const AlignedVector< float >& values)
{
	float s = 0.0f;
	for (float v : values)
		s += v;
	return s;
}

float minimum(const AlignedVector< float >& values)
{
	float m = values.empty() ? 0.0f : values.front();
	for (float v : values)
		m = min(m, v);
	return m;
}

float maximum(const AlignedVector< float >& values)
{
	float m = values.empty() ? 0.0f : values.front();
	for (float v : values)
		m = max(m, v);
	return m;
}

AlignedVector< Vector2 > rectangle(float width, float height)
{
	return { Vector2(0.0f, 0.0f), Vector2(width, 0.0f), Vector2(width, height), Vector2(0.0f, height) };
}

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.test.CaseTriangulator", 0, CaseTriangulator, Case)

void CaseTriangulator::run()
{
	// Concave L-shaped hexagon in XZ plane, model convention normal is +Y; the reflex corner
	// lies on the diagonal of an ear, cutting that ear must not drop part of the polygon.
	{
		const Vector4 hexagon[] =
		{
			Vector4(0.0f, 0.0f, 0.0f, 1.0f),
			Vector4(2.0f, 0.0f, 0.0f, 1.0f),
			Vector4(2.0f, 0.0f, 1.0f, 1.0f),
			Vector4(1.0f, 0.0f, 1.0f, 1.0f),
			Vector4(1.0f, 0.0f, 2.0f, 1.0f),
			Vector4(0.0f, 0.0f, 2.0f, 1.0f)
		};

		const Vector4 up(0.0f, 1.0f, 0.0f, 0.0f);

		for (int32_t start = 0; start < 6; ++start)
		{
			AlignedVector< Vector4 > points;
			for (int32_t i = 0; i < 6; ++i)
				points.push_back(hexagon[(start + i) % 6]);

			// Emitted triangles must keep the polygon's winding even if normal is flipped.
			for (const Vector4& normal : { up, -up })
			{
				const auto triangles = freeze(points, normal);
				const auto triangleAreas = areas(points, triangles, up);
				CASE_ASSERT_EQUAL (triangles.size(), 4);
				CASE_ASSERT_COMPARE (sum(triangleAreas), 3.0f, fuzzyEqual);
				CASE_ASSERT (minimum(triangleAreas) > 0.0f);
			}
		}
	}

	// Small polygon far from origin, winding and convexity must not be lost to precision.
	{
		const AlignedVector< Vector2 > points = { Vector2(101.48f, 100.92f), Vector2(101.484f, 100.92f), Vector2(101.484f, 100.921f), Vector2(101.48f, 100.921f) };
		const auto triangles = freeze(points);
		CASE_ASSERT_EQUAL (triangles.size(), 2);
		CASE_ASSERT (minimum(areas(points, triangles)) > 0.0f);
	}

	// Rectangle with corners along its long edges; all corners must be used, ie. no T-junctions.
	{
		const AlignedVector< Vector2 > points =
		{
			Vector2(0.0f, 0.0f), Vector2(1.0f, 0.0f), Vector2(1.0f, 1.0f), Vector2(1.0f, 2.0f), Vector2(1.0f, 3.0f),
			Vector2(1.0f, 4.0f), Vector2(0.0f, 4.0f), Vector2(0.0f, 3.0f), Vector2(0.0f, 2.0f), Vector2(0.0f, 1.0f)
		};
		const auto triangles = freeze(points);
		const auto triangleAreas = areas(points, triangles);
		CASE_ASSERT_EQUAL (triangles.size(), 8);
		CASE_ASSERT_COMPARE (sum(triangleAreas), 4.0f, fuzzyEqual);
		CASE_ASSERT (minimum(triangleAreas) > 0.0f);
	}

	// Convex polygons are triangulated exactly as by the previous implementation since
	// pipeline outputs are cached by hash.
	{
		CASE_ASSERT_EQUAL (toString(freeze(rectangle(1.0f, 1.0f))), std::wstring(L"(3,0,1)(1,2,3)"));
		CASE_ASSERT_EQUAL (toString(freeze(rectangle(2.0f, 2.0f))), std::wstring(L"(3,0,1)(2,3,1)"));
		CASE_ASSERT_EQUAL (toString(freeze(rectangle(0.5f, 0.5f))), std::wstring(L"(3,0,1)(3,1,2)"));

		const AlignedVector< Vector2 > trapezoid = { Vector2(0.0f, 0.0f), Vector2(4.0f, 0.0f), Vector2(3.0f, 1.0f), Vector2(1.0f, 1.0f) };
		CASE_ASSERT_EQUAL (toString(freeze(trapezoid)), std::wstring(L"(2,3,0)(1,2,0)"));

		const AlignedVector< Vector2 > pentagon = { Vector2(0.0f, 0.0f), Vector2(2.0f, 0.0f), Vector2(3.0f, 1.0f), Vector2(1.0f, 3.0f), Vector2(-1.0f, 1.0f) };
		CASE_ASSERT_EQUAL (toString(freeze(pentagon)), std::wstring(L"(1,2,3)(3,4,0)(1,3,0)"));
	}

	// Clockwise polygon keeps its winding.
	{
		const AlignedVector< Vector2 > points = { Vector2(0.0f, 0.0f), Vector2(0.0f, 2.0f), Vector2(2.0f, 2.0f), Vector2(2.0f, 0.0f) };
		const auto triangles = freeze(points);
		const auto triangleAreas = areas(points, triangles);
		CASE_ASSERT_EQUAL (triangles.size(), 2);
		CASE_ASSERT_COMPARE (sum(triangleAreas), -4.0f, fuzzyEqual);
		CASE_ASSERT (maximum(triangleAreas) < 0.0f);
	}

	// Degenerate polygons.
	{
		const AlignedVector< Vector2 > line = { Vector2(0.0f, 0.0f), Vector2(1.0f, 0.0f), Vector2(2.0f, 0.0f), Vector2(3.0f, 0.0f) };
		CASE_ASSERT_EQUAL (freeze(line).size(), 0);

		const AlignedVector< Vector2 > segment = { Vector2(0.0f, 0.0f), Vector2(1.0f, 0.0f) };
		CASE_ASSERT_EQUAL (freeze(segment).size(), 0);
	}
}

}
