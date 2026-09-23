/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cfloat>
#include <cmath>
#include <limits>
#include <new>
#include "Core/Math/Float.h"
#include "Core/Math/Triangulator.h"

namespace traktor
{
	namespace
	{

/*! Polygons up to this size are triangulated without heap allocations; model polygons have at most 32 vertices. */
const int32_t c_maxLocalPoints = 32;

/*! Points closer than this, relative to magnitude of coordinates, are considered coincident or on a line; coordinates aren't more precise than that. */
const float c_toleranceScale = 8.0f * FLT_EPSILON;

/*! Ear clipping state.
 *
 * Remaining corners are kept in a circular doubly linked list in index order. All
 * orientation tests are made relative to the polygon's own winding, so the polygon
 * is never reversed and emitted triangles keep the polygon's winding.
 */
struct EarClipper
{
	const Vector2* points;
	int32_t* prev;
	int32_t* next;
	float* turn;		//!< Twice the signed area of (prev, corner, next) in polygon winding; positive if convex, zero if flat.
	float* badness;		//!< Ear badness, lower is better; negative if corner isn't an ear.
	float winding;		//!< 1 if polygon is counter clockwise, -1 if clockwise.
	float tolerance;
};

bool equal(const Vector2& a, const Vector2& b)
{
	return a.x == b.x && a.y == b.y;
}

float calculateSignedTurn(const EarClipper& ec, int32_t corner)
{
	const Vector2& p = ec.points[ec.prev[corner]];
	const Vector2& c = ec.points[corner];
	const Vector2& n = ec.points[ec.next[corner]];
	return ec.winding * determinant(c - p, n - c);
}

/*! Signed turn of corner, zero if corner is flat, ie. within tolerance of the line through its neighbours. */
float calculateTurn(const EarClipper& ec, int32_t corner)
{
	const float turn = calculateSignedTurn(ec, corner);
	const float baseLength2 = (ec.points[ec.next[corner]] - ec.points[ec.prev[corner]]).length2();
	return (turn * turn > ec.tolerance * ec.tolerance * baseLength2) ? turn : 0.0f;
}

/*! Check if corner is an ear, ie. convex and no other corner inside its triangle.
 *
 * Only corners which aren't convex need to be checked since, in a simple polygon, such
 * a corner always intrudes if any corner does. A corner on (or within tolerance of) the
 * triangle's boundary also intrudes; otherwise cutting the ear would leave a reflex
 * corner touching the new diagonal, a self touching polygon which cannot be properly
 * clipped. Duplicates of the triangle's own corners don't intrude.
 *
 * In relaxed mode, only used when there are no ears, a corner must be strictly inside
 * the triangle to intrude.
 */
bool isEar(const EarClipper& ec, int32_t corner, bool relaxed)
{
	if (!(ec.turn[corner] > 0.0f))
		return false;

	const int32_t ip = ec.prev[corner];
	const int32_t in = ec.next[corner];

	const Vector2& p = ec.points[ip];
	const Vector2& c = ec.points[corner];
	const Vector2& n = ec.points[in];

	const Vector2 e0 = c - p;
	const Vector2 e1 = n - c;
	const Vector2 e2 = p - n;

	const float tolerance = relaxed ? 0.0f : ec.tolerance;
	const float tolerance2 = tolerance * tolerance;
	const float l0 = tolerance2 * e0.length2();
	const float l1 = tolerance2 * e1.length2();
	const float l2 = tolerance2 * e2.length2();

	const Vector2 margin(tolerance, tolerance);
	const Vector2 mn = min(min(p, c), n) - margin;
	const Vector2 mx = max(max(p, c), n) + margin;

	for (int32_t i = ec.next[in]; i != ip; i = ec.next[i])
	{
		if (ec.turn[i] > 0.0f)
			continue;

		const Vector2& q = ec.points[i];
		if (q.x < mn.x || q.y < mn.y || q.x > mx.x || q.y > mx.y)
			continue;

		const float d0 = ec.winding * determinant(e0, q - p);
		const float d1 = ec.winding * determinant(e1, q - c);
		const float d2 = ec.winding * determinant(e2, q - n);

		if (relaxed)
		{
			if (d0 > 0.0f && d1 > 0.0f && d2 > 0.0f)
				return false;
			continue;
		}

		// Distance to edge is d / |e|, compare squared to not have to normalize.
		if ((d0 < 0.0f && d0 * d0 > l0) || (d1 < 0.0f && d1 * d1 > l1) || (d2 < 0.0f && d2 * d2 > l2))
			continue;

		if ((q - p).length2() <= tolerance2 || (q - c).length2() <= tolerance2 || (q - n).length2() <= tolerance2)
			continue;

		return false;
	}

	return true;
}

/*! Badness of an ear, twice the sum of the cotangents of the ear triangle's angles.
 *
 * Calculated as sum of squared edge lengths divided by twice the area; minimal (2*sqrt(3))
 * for an equilateral triangle and grows without bound as any angle approaches zero.
 * Since corner is convex it's never NaN, so ordering by it is a strict weak ordering.
 */
float calculateBadness(const EarClipper& ec, int32_t corner)
{
	const Vector2& p = ec.points[ec.prev[corner]];
	const Vector2& c = ec.points[corner];
	const Vector2& n = ec.points[ec.next[corner]];
	return ((p - c).length2() + (n - c).length2() + (n - p).length2()) / ec.turn[corner];
}

void updateEar(EarClipper& ec, int32_t corner)
{
	ec.badness[corner] = isEar(ec, corner, false) ? calculateBadness(ec, corner) : -1.0f;
}

/*! Select ear to cut; best ear first in index order, or simply first ear if not sorted. */
int32_t selectEar(const EarClipper& ec, int32_t first, Triangulator::Mode mode)
{
	int32_t selected = -1;
	float selectedBadness = 0.0f;

	int32_t i = first;
	do
	{
		const float badness = ec.badness[i];
		if (badness >= 0.0f)
		{
			if (mode != Triangulator::Mode::Sorted)
				return i;
			if (selected < 0 || badness < selectedBadness)
			{
				selected = i;
				selectedBadness = badness;
			}
		}
		i = ec.next[i];
	}
	while (i != first);

	return selected;
}

/*! Select corner to cut when there are no ears, only happens with self intersecting or touching polygons, or precision issues.
 *
 * Prefer a corner with no other corner strictly inside its triangle, else any convex corner.
 * Return -1 if no convex corner remains, ie. remaining polygon is degenerate.
 */
int32_t selectFallback(const EarClipper& ec, int32_t first, Triangulator::Mode mode)
{
	for (int32_t pass = 0; pass < 2; ++pass)
	{
		int32_t selected = -1;
		float selectedBadness = 0.0f;

		int32_t i = first;
		do
		{
			if (ec.turn[i] > 0.0f && (pass > 0 || isEar(ec, i, true)))
			{
				if (mode != Triangulator::Mode::Sorted)
					return i;
				const float badness = calculateBadness(ec, i);
				if (selected < 0 || badness < selectedBadness)
				{
					selected = i;
					selectedBadness = badness;
				}
			}
			i = ec.next[i];
		}
		while (i != first);

		if (selected >= 0)
			return selected;
	}
	return -1;
}

/*! Metric the previous implementation sorted ears on, NaN if dot product of the corner's (unnormalized) edges is outside [-1, 1]. */
float calculateLegacyMetric(const EarClipper& ec, int32_t corner)
{
	const Vector2& c = ec.points[corner];
	const float d = dot(ec.points[ec.prev[corner]] - c, ec.points[ec.next[corner]] - c);
	if (!(abs(d) <= 1.0f))
		return std::numeric_limits< float >::quiet_NaN();	// Same as acosf, without its slow domain error path.
	return 3.0f * abs(acosf(d) - PI / 3.0f);
}

/*! Triangulate convex polygon cutting ears in the same order as the previous implementation.
 *
 * Any order triangulates a convex polygon properly so the previous order is kept for
 * these, the vast majority of polygons, to not change existing triangulations. Every
 * corner which isn't flat is an ear; they were sorted on the legacy metric using a
 * comparator which, as built, found NaN less than anything and the first was cut. Since
 * std::sort uses insertion sort for up to 32 elements the first ear is the result of a
 * linear pass. Clockwise polygons were reversed first, so corners are visited in reverse
 * index order for those. Expects signed turn of each corner.
 *
 * Flat corners were never cut, so a degenerate remainder of only flat corners could be
 * left. Unless those are at no more than two distinct points some of them are left as
 * T-junctions; nothing is emitted and false returned then.
 */
bool triangulateLegacyConvex(EarClipper& ec, int32_t npoints, const Triangulator::fn_callback_t& callback)
{
	int32_t* forward = (ec.winding > 0.0f) ? ec.next : ec.prev;
	int32_t first = (ec.winding > 0.0f) ? 0 : npoints - 1;

	int32_t triangles[3 * (c_maxLocalPoints - 2)];
	int32_t ntriangles = 0;

	for (int32_t i = 0; i < npoints; ++i)
		ec.badness[i] = calculateLegacyMetric(ec, i);

	for (int32_t remaining = npoints; remaining >= 3; --remaining)
	{
		int32_t cut = -1;
		int32_t i = first;
		do
		{
			if (ec.turn[i] > 0.0f && (cut < 0 || isNan(ec.badness[i]) || isNan(ec.badness[cut]) || ec.badness[i] < ec.badness[cut]))
				cut = i;
			i = forward[i];
		}
		while (i != first);

		if (cut < 0)
		{
			const Vector2& p0 = ec.points[first];
			const Vector2* p1 = nullptr;
			for (int32_t j = forward[first]; j != first; j = forward[j])
			{
				const Vector2& p = ec.points[j];
				if (equal(p, p0) || (p1 != nullptr && equal(p, *p1)))
					continue;
				if (p1 != nullptr)
					return false;
				p1 = &p;
			}
			break;
		}

		const int32_t ip = ec.prev[cut];
		const int32_t in = ec.next[cut];

		triangles[ntriangles * 3 + 0] = ip;
		triangles[ntriangles * 3 + 1] = cut;
		triangles[ntriangles * 3 + 2] = in;
		++ntriangles;

		ec.next[ip] = in;
		ec.prev[in] = ip;
		if (cut == first)
			first = forward[cut];

		if (remaining > 3)
		{
			ec.turn[ip] = calculateSignedTurn(ec, ip);
			ec.turn[in] = calculateSignedTurn(ec, in);
			ec.badness[ip] = calculateLegacyMetric(ec, ip);
			ec.badness[in] = calculateLegacyMetric(ec, in);
		}
	}

	for (int32_t i = 0; i < ntriangles; ++i)
		callback(triangles[i * 3 + 0], triangles[i * 3 + 1], triangles[i * 3 + 2]);

	return true;
}

void linkCorners(EarClipper& ec, int32_t npoints)
{
	for (int32_t i = 0; i < npoints; ++i)
	{
		ec.prev[i] = (i > 0) ? i - 1 : npoints - 1;
		ec.next[i] = (i < npoints - 1) ? i + 1 : 0;
	}
}

void triangulate(const Vector2* points, int32_t npoints, Triangulator::Mode mode, const Triangulator::fn_callback_t& callback, int32_t* indices, float* values)
{
	// Determine winding; relative to first point to keep precision of small polygons far from origin.
	float area = 0.0f;
	float magnitude = 0.0f;
	for (int32_t i = 0; i < npoints; ++i)
	{
		magnitude = max(magnitude, max(abs(points[i].x), abs(points[i].y)));
		if (i >= 2)
			area += determinant(points[i - 1] - points[0], points[i] - points[0]);
	}

	EarClipper ec;
	ec.points = points;
	ec.prev = indices;
	ec.next = indices + npoints;
	ec.turn = values;
	ec.badness = values + npoints;
	ec.winding = (area < 0.0f) ? -1.0f : 1.0f;
	ec.tolerance = c_toleranceScale * magnitude;

	linkCorners(ec, npoints);

	// Convex polygons keep the triangulation of the previous implementation.
	if (mode == Triangulator::Mode::Sorted && npoints <= c_maxLocalPoints)
	{
		bool convex = true;
		for (int32_t i = 0; i < npoints && convex; ++i)
		{
			ec.turn[i] = calculateSignedTurn(ec, i);
			convex = (ec.turn[i] >= 0.0f);
		}
		if (convex)
		{
			if (triangulateLegacyConvex(ec, npoints, callback))
				return;
			linkCorners(ec, npoints);
		}
	}

	for (int32_t i = 0; i < npoints; ++i)
		ec.turn[i] = calculateTurn(ec, i);
	if (npoints > 3)
	{
		for (int32_t i = 0; i < npoints; ++i)
			updateEar(ec, i);
	}

	// First remaining corner, ie. lowest remaining index.
	int32_t first = 0;

	for (int32_t remaining = npoints; remaining > 3; --remaining)
	{
		bool recalculate = false;

		int32_t cut = selectEar(ec, first, mode);
		if (cut < 0)
		{
			cut = selectFallback(ec, first, mode);
			if (cut < 0)
				return;
			recalculate = true;
		}

		const int32_t ip = ec.prev[cut];
		const int32_t in = ec.next[cut];
		callback(ip, cut, in);

		// Cut the ear.
		ec.next[ip] = in;
		ec.prev[in] = ip;
		if (cut == first)
			first = in;

		const bool convexPrev = ec.turn[ip] > 0.0f;
		const bool convexNext = ec.turn[in] > 0.0f;
		ec.turn[ip] = calculateTurn(ec, ip);
		ec.turn[in] = calculateTurn(ec, in);

		if (remaining - 1 <= 3)
			break;

		// Cutting an ear only makes its neighbours more convex, and a neighbour which stops being
		// reflex can no longer intrude other ears; anything else, due to fallback or precision,
		// means ears must be recalculated.
		const bool unblocked = (!convexPrev && ec.turn[ip] > 0.0f) || (!convexNext && ec.turn[in] > 0.0f);
		if (!recalculate)
			recalculate = (convexPrev && !(ec.turn[ip] > 0.0f)) || (convexNext && !(ec.turn[in] > 0.0f));

		if (recalculate || unblocked)
		{
			int32_t i = first;
			do
			{
				if (recalculate || i == ip || i == in || ec.badness[i] < 0.0f)
					updateEar(ec, i);
				i = ec.next[i];
			}
			while (i != first);
		}
		else
		{
			updateEar(ec, ip);
			updateEar(ec, in);
		}
	}

	// Emit last triangle unless it's degenerate or inverted.
	int32_t corner = first;
	for (int32_t i = 0; i < 3; ++i, corner = ec.next[corner])
	{
		if (ec.turn[corner] > 0.0f)
		{
			callback(ec.prev[corner], corner, ec.next[corner]);
			break;
		}
	}
}

void triangulate(const Vector2* points, int32_t npoints, Triangulator::Mode mode, const Triangulator::fn_callback_t& callback)
{
	if (npoints < 3)
		return;

	if (npoints <= c_maxLocalPoints)
	{
		int32_t indices[2 * c_maxLocalPoints];
		float values[2 * c_maxLocalPoints];
		triangulate(points, npoints, mode, callback, indices, values);
	}
	else
	{
		AlignedVector< int32_t > indices(2 * npoints);
		AlignedVector< float > values(2 * npoints);
		triangulate(points, npoints, mode, callback, indices.ptr(), values.ptr());
	}
}

	}

void Triangulator::freeze(
	const AlignedVector< Vector2 >& points,
	Mode mode,
	const fn_callback_t& callback
)
{
	triangulate(points.c_ptr(), int32_t(points.size()), mode, callback);
}

void Triangulator::freeze(
	const AlignedVector< Vector4 >& points,
	const Vector4& normal,
	Mode mode,
	const fn_callback_t& callback
)
{
	// Use maximum axis to determine projection plane.
	Vector4 u(0.0f, 0.0f, 0.0f), v(0.0f, 0.0f, 0.0f);
	if (abs(normal.x()) > abs(normal.y()))
	{
		if (abs(normal.x()) > abs(normal.z()))	// X major
		{
			u = normal.x() > 0.0f ? Vector4(0.0f, 0.0f, 1.0f) : Vector4(0.0f, 0.0f, -1.0f);
			v = Vector4(0.0f, 1.0f, 0.0f);
		}
		else	// Z major
		{
			u = normal.z() > 0.0f ? Vector4(-1.0f, 0.0f, 0.0f) : Vector4(1.0f, 0.0f, 0.0f);
			v = Vector4(0.0f, 1.0f, 0.0f);
		}
	}
	else
	{
		if (abs(normal.y()) > abs(normal.z()))	// Y major
		{
			u = normal.y() > 0.0f ? Vector4(0.0f, 0.0f, -1.0f) : Vector4(0.0f, 0.0f, 1.0f);
			v = Vector4(1.0f, 0.0f, 0.0f);
		}
		else	// Z major
		{
			u = normal.z() > 0.0f ? Vector4(-1.0f, 0.0f, 0.0f) : Vector4(1.0f, 0.0f, 0.0f);
			v = Vector4(0.0f, 1.0f, 0.0f);
		}
	}

	// Project all points onto 2d plane and freeze 2d polygon.
	const int32_t npoints = int32_t(points.size());
	if (npoints <= c_maxLocalPoints)
	{
		// Construct points in place since default constructing (zeroing) an array is measurably slower.
		alignas(Vector2) uint8_t storage[c_maxLocalPoints * sizeof(Vector2)];
		Vector2* projected = reinterpret_cast< Vector2* >(storage);
		for (int32_t i = 0; i < npoints; ++i)
			new (&projected[i]) Vector2(dot3(u, points[i]), dot3(v, points[i]));
		triangulate(projected, npoints, mode, callback);
	}
	else
	{
		AlignedVector< Vector2 > projected;
		projected.reserve(npoints);
		for (int32_t i = 0; i < npoints; ++i)
			projected.push_back(Vector2(dot3(u, points[i]), dot3(v, points[i])));
		triangulate(projected.c_ptr(), npoints, mode, callback);
	}
}

}
