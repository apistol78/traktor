/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include <limits>
#include "Core/Math/Line2.h"
#include "Core/Math/Winding2.h"

namespace traktor
{
	namespace
	{

struct ChainSortPred
{
	T_MATH_INLINE bool operator () (const Vector2& lh, const Vector2& rh) const
	{
		return lh.x < rh.x || (lh.x == rh.x && lh.y < rh.y);
	}
};

T_MATH_INLINE float isLeft(const Vector2& P0, const Vector2& P1, const Vector2& P2)
{
	return (P1.x - P0.x) * (P2.y - P0.y) - (P2.x - P0.x) * (P1.y - P0.y);
}

	}

Winding2::Winding2()
{
}

Winding2::Winding2(uint32_t size)
:	m_points(size)
{
}

Winding2::Winding2(const points_t& points)
:	m_points(points)
{
}

Winding2::Winding2(const Vector2* points, size_t npoints)
:	m_points(&points[0], &points[npoints])
{
}

Winding2::Winding2(const Vector2& p1, const Vector2& p2, const Vector2& p3)
:	m_points(3)
{
	m_points[0] = p1;
	m_points[1] = p1;
	m_points[2] = p1;
}

void Winding2::clear()
{
	m_points.clear();
}

void Winding2::push(const Vector2& p)
{
	m_points.push_back(p);
}

Winding2 Winding2::convexHull(const Vector2* pnts, int npnts)
{
	Winding2 hull;

	// Sort points in increasing x- and y-coordinates.
	AlignedVector< Vector2 > P(npnts);
	for (int i = 0; i < npnts; ++i)
		P[i] = pnts[i];
	std::sort(P.begin(), P.end(), ChainSortPred());

	hull.resize(2 * npnts);

	int k = 0;
	for (int i = 0; i < npnts; ++i)
	{
		while (k >= 2 && isLeft(hull[k - 2], hull[k - 1], P[i]) <= 0.0f)
			--k;
		hull[k++] = P[i];
	}
	for (int i = npnts - 2, t = k + 1; i >= 0; --i)
	{
		while (k >= t && isLeft(hull[k - 2], hull[k - 1], P[i]) <= 0.0f)
			--k;
		hull[k++] = P[i];
	}

	if (k > 0)
		--k;

	hull.resize(k);
	return hull;
}

Winding2 Winding2::convexHull(const AlignedVector< Vector2 >& pnts)
{
	return convexHull(&pnts[0], int(pnts.size()));
}

bool Winding2::inside(const Vector2& pnt) const
{
	bool c = false;
	for (int32_t i = 0, j = int32_t(m_points.size()) - 1; i < int32_t(m_points.size()); j = i++)
	{
		const Vector2& pi = m_points[i];
		const Vector2& pj = m_points[j];
		if ((((pnt.y >= pi.y) && (pnt.y < pj.y)) || ((pnt.y >= pj.y) && (pnt.y < pi.y))) && (pnt.x < (pj.x - pi.x) * (pnt.y - pi.y) / (pj.y - pi.y) + pi.x))
			c = !c;
	}
	return c;
}

Vector2 Winding2::closest(const Vector2& pnt) const
{
	// Check if point is inside winding.
	if (inside(pnt))
		return pnt;

	float minD2 = std::numeric_limits< float >::max();
	Vector2 minP(0.0f, 0.0f);

	for (int32_t i = 0, j = int32_t(m_points.size()) - 1; i < int32_t(m_points.size()); j = i++)
	{
		const Vector2& pi = m_points[i];
		const Vector2& pj = m_points[j];
		const Line2 ln(pi, pj);

		// Project point onto edge line.
		Vector2 pc = ln.project(pnt);

		// Clamp projected point within edge.
		const float k = dot(pc - pi, ln.delta()) / (ln.length() * ln.length());
		if (k < 0.0f)
			pc = pi;
		else if (k > 1.0f)
			pc = pj;

		// Check if projected point is closest.
		const float d2 = (pc - pnt).length2();
		if (d2 < minD2)
		{
			minP = pc;
			minD2 = d2;
		}
	}

	return minP;
}

}
