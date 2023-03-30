/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Test/CaseWinding.h"
#include "Core/Math/Winding2.h"
#include "Core/Math/Winding3.h"

namespace traktor::test
{
	namespace
	{

bool compareEqual(const Vector4& a, const Vector4& b)
{
	for (int i = 0; i < 4; ++i)
	{
		if (std::fabs(a[i] - b[i]) > FUZZY_EPSILON)
			return false;
	}
	return true;
}

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.test.CaseWinding", 0, CaseWinding, Case)

void CaseWinding::run()
{
	// Build 2D convex hull.
	{
		const Vector2 points[] =
		{
			Vector2(-2.0f, 0.0f),
			Vector2(2.0f, 0.0f),
			Vector2(0.0f, -3.0f),
			Vector2(0.0f, 3.0f),
			Vector2(0.0f, 0.0f)
		};

		Winding2 hull = Winding2::convexHull(points, sizeof_array(points));
		CASE_ASSERT_EQUAL (hull.size(), 4);
	}

	// Find angle indices.
	{
		const Vector4 points_1[] =
		{
			Vector4(1.0f, 0.0f, 0.0f, 1.0f),
			Vector4(2.0f, 0.0f, 0.0f, 1.0f),
			Vector4(1.0f, 1.0f, 0.0f, 1.0f),
			Vector4(0.0f, 0.0f, 0.0f, 1.0f)
		};

		Winding3 winding_1(points_1, sizeof_array(points_1));

		uint32_t i1, i2, i3;
		CASE_ASSERT (winding_1.angleIndices(i1, i2, i3));
		CASE_ASSERT_EQUAL (i1, 0);
		CASE_ASSERT_EQUAL (i2, 1);
		CASE_ASSERT_EQUAL (i3, 2);

		Plane plane;
		CASE_ASSERT (winding_1.getPlane(plane));
		CASE_ASSERT_COMPARE (plane.normal(), Vector4(0.0f, 0.0f, -1.0f, 0.0f), compareEqual);
		CASE_ASSERT_EQUAL (plane.distance(), 0.0f);
	}

	// All points on a line.
	{
		const Vector4 points_1[] =
		{
			Vector4(1.0f, 0.0f, 0.0f, 1.0f),
			Vector4(2.0f, 0.0f, 0.0f, 1.0f),
			Vector4(3.0f, 0.0f, 0.0f, 1.0f),
			Vector4(4.0f, 0.0f, 0.0f, 1.0f)
		};

		Winding3 winding_1(points_1, sizeof_array(points_1));

		uint32_t i1 = 0, i2 = 0, i3 = 0;
		CASE_ASSERT (!winding_1.angleIndices(i1, i2, i3));
		CASE_ASSERT_EQUAL (i1, 0);
		CASE_ASSERT_EQUAL (i2, 0);
		CASE_ASSERT_EQUAL (i3, 0);

		Plane plane;
		CASE_ASSERT (!winding_1.getPlane(plane));
	}
}

}
