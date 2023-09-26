/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include <numeric>
#include "Core/Math/Winding2.h"
#include "Core/Math/Winding3.h"

namespace traktor
{

Winding3::Winding3()
{
}

Winding3::Winding3(uint32_t size)
:	m_points(size)
{
}

Winding3::Winding3(const points_t& points)
:	m_points(points)
{
}

Winding3::Winding3(const Vector4* points, size_t npoints)
:	m_points(&points[0], &points[npoints])
{
}

Winding3::Winding3(const Vector4& p1, const Vector4& p2, const Vector4& p3)
:	m_points(3)
{
	m_points[0] = p1;
	m_points[1] = p2;
	m_points[2] = p3;
}

void Winding3::clear()
{
	m_points.clear();
}

void Winding3::push(const Vector4& p)
{
	m_points.push_back(p);
}

bool Winding3::angleIndices(uint32_t& outI1, uint32_t& outI2, uint32_t& outI3) const
{
	if (m_points.size() < 3)
		return false;

	for (size_t p = m_points.size() - 1, i = 0; i < m_points.size(); p = i++)
	{
		size_t n = (i + 1) % m_points.size();

		const Vector4& p1 = m_points[p];
		const Vector4& p2 = m_points[i];
		const Vector4& p3 = m_points[n];

		Vector4 e1 = p1 - p2;
		Vector4 e2 = p3 - p2;

		const Scalar ln1 = e1.length();
		const Scalar ln2 = e2.length();

		if (ln1 > FUZZY_EPSILON && ln2 > FUZZY_EPSILON)
		{
			e1 /= ln1;
			e2 /= ln2;

			const Scalar phi = dot3(e1, e2);
			if (abs(phi) < 1.0f - FUZZY_EPSILON)
			{
				outI1 = uint32_t(p);
				outI2 = uint32_t(i);
				outI3 = uint32_t(n);
				return true;
			}
		}
	}

	return false;
}

bool Winding3::getProjection(Winding2& outProjection, Vector4& outU, Vector4& outV) const
{
	Plane plane;
	Vector4 p;

	if (!getPlane(plane))
		return false;

	Vector4 u, v;
	orthogonalFrame(plane.normal(), u, v);

	outProjection.resize((uint32_t)m_points.size());
	for (uint32_t i = 0; i < (uint32_t)m_points.size(); ++i)
	{
		outProjection[i] = Vector2(
			dot3(u, m_points[i]),
			dot3(v, m_points[i])
		);
	}

	outU = u;
	outV = v;

	return true;
}

bool Winding3::getPlane(Plane& outPlane) const
{
	// Get indices to points which form a good frame.
	uint32_t i1, i2, i3;
	if (!angleIndices(i1, i2, i3))
		return false;

	// Create plane from indexed points.
	outPlane = Plane(m_points[i1], m_points[i2], m_points[i3]);
	return true;
}

void Winding3::split(const Plane& plane, Winding3& outFront, Winding3& outBack) const
{
	for (size_t i = 0, j = m_points.size() - 1; i < m_points.size(); j = i++)
	{
		const Vector4& a = m_points[i];
		const Vector4& b = m_points[j];

		const Scalar da = plane.distance(a);
		const Scalar db = plane.distance(b);

		if ((da <= -FUZZY_EPSILON && db >= FUZZY_EPSILON) || (da >= FUZZY_EPSILON && db <= -FUZZY_EPSILON))
		{
			Scalar k;
			plane.intersectSegment(a, b, k);
			T_ASSERT(k >= 0.0_simd && k <= 1.0_simd);

			const Vector4 p = lerp(a, b, k);
			outFront.m_points.push_back(p);
			outBack.m_points.push_back(p);
		}

		if (da >= -FUZZY_EPSILON)
			outFront.m_points.push_back(a);
		if (da <= FUZZY_EPSILON)
			outBack.m_points.push_back(a);
	}
}

int Winding3::classify(const Plane& plane) const
{
	int32_t side[2] = { 0, 0 };
	for (size_t i = 0; i < m_points.size(); ++i)
	{
		const Scalar d = plane.distance(m_points[i]);
		if (d >= FUZZY_EPSILON)
			side[0]++;
		else if (d <= -FUZZY_EPSILON)
			side[1]++;
	}

	if (side[0] && !side[1])
		return CfFront;
	else if (!side[0] && side[1])
		return CfBack;
	else if (!side[0] && !side[1])
		return CfCoplanar;

	T_ASSERT(side[0] && side[1]);
	return CfSpan;
}

float Winding3::area() const
{
	const int32_t n = int32_t(m_points.size());
	if (n <= 2)
		return 0.0f;

	Plane plane;
	if (!getPlane(plane))
		return 0.0f;

	float area = 0.0f;
	int32_t i, j, k;

	const Vector4 N = plane.normal();
	const Vector4 A = N.absolute();
	const int32_t coord = majorAxis3(A);

	for (i = 1, j = 2, k = 0; i <= n; i++, j++, k++)
	{
		const int32_t ii = i % n;
		const int32_t jj = j % n;
		const int32_t kk = k % n;

		switch (coord)
		{
		case 0:
			area += (m_points[ii].y() * (m_points[jj].z() - m_points[kk].z()));
			break;

		case 1:
			area += (m_points[ii].x() * (m_points[jj].z() - m_points[kk].z()));
			break;

		case 2:
			area += (m_points[ii].x() * (m_points[jj].y() - m_points[kk].y()));
			break;
		}
	}

	const float Aln = A.length();
	switch (coord)
	{
	case 0:
		area *= (Aln / (2.0f * A.x()));
		break;

	case 1:
		area *= (Aln / (2.0f * A.y()));
		break;

	case 2:
		area *= (Aln / (2.0f * A.z()));
		break;
	}

	return area;
}

Vector4 Winding3::center() const
{
	Vector4 acc = Vector4::zero();
	for (points_t::const_iterator i = m_points.begin(); i != m_points.end(); ++i)
		acc += *i;
	acc /= Scalar(float(m_points.size()));
	return acc.xyz1();
}

bool Winding3::rayIntersection(
	const Vector4& origin,
	const Vector4& direction,
	Scalar& outK,
	Vector4* outPoint
) const
{
	Plane plane;
	Vector4 p;

	if (!getPlane(plane))
		return false;

	if (!plane.intersectRay(origin, direction, outK, p) || outK <= 0.0_simd)
		return false;

	Vector4 u, v;
	orthogonalFrame(plane.normal(), u, v);

	// Project all points onto 2d plane.
	Vector2 projected[32];
	T_ASSERT(m_points.size() <= sizeof_array(projected));
	for (size_t i = 0; i < m_points.size(); ++i)
	{
		projected[i].x = dot3(u, m_points[i]);
		projected[i].y = dot3(v, m_points[i]);
	}

	// Use odd-even rule to determine if point is in polygon.
	const Vector2 pnt(
		dot3(u, p),
		dot3(v, p)
	);

	bool pass = false;
	for (size_t i = 0, j = m_points.size() - 1; i < m_points.size(); j = i++)
	{
		const float dx = projected[j].x - projected[i].x;
		const float dy = projected[j].y - projected[i].y;

		if (abs(dy) <= FUZZY_EPSILON)
			continue;

		const float mny = min(projected[i].y, projected[j].y);
		const float mxy = max(projected[i].y, projected[j].y);

		const float x = projected[i].x + dx * (pnt.y - projected[i].y) / dy;
		if (
			(pnt.y >= mny && pnt.y <= mxy) &&
			(pnt.x < x)
		)
			pass = !pass;
	}

	if (!pass)
		return false;

	if (outPoint)
		*outPoint = p;

	return true;
}

void Winding3::flip()
{
	std::reverse(m_points.begin(), m_points.end());
}

}
