/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Vector2.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! 2D point winding.
 * \ingroup Core
 */
class T_DLLCLASS Winding2
{
public:
	typedef AlignedVector< Vector2 > points_t;

	Winding2();

	explicit Winding2(uint32_t size);

	explicit Winding2(const points_t& points);

	explicit Winding2(const Vector2* points, size_t npoints);

	explicit Winding2(const Vector2& p1, const Vector2& p2, const Vector2& p3);

	/*! Calculate convex hull from a random point set. */
	static Winding2 convexHull(const Vector2* pnts, int npnts);

	/*! Calculate convex hull from a random point set. */
	static Winding2 convexHull(const AlignedVector< Vector2 >& pnts);

	/*! Clear all points. */
	void clear();

	/*! Add another point to winding. */
	void push(const Vector2& p);

	/*! Check if point is within winding. */
	bool inside(const Vector2& pnt) const;

	/*! Calculate closest point. */
	Vector2 closest(const Vector2& pnt) const;

	/*! Reserve enough memory. */
	void reserve(uint32_t capacity) { m_points.reserve(capacity); }

	/*! Resize number of points.  */
	void resize(uint32_t size) { m_points.resize(size); }

	/*! Get number of points. */
	uint32_t size() const { return uint32_t(m_points.size()); }

	/*! Check if winding is empty, ie. no points. */
	bool empty() const { return m_points.empty(); }

	/*! Get container with all points. */
	const points_t& get() const { return m_points; }

	/*! Get point by index. */
	Vector2& operator [] (uint32_t index) { return m_points[index]; }

	/*! Get point by index. */
	const Vector2& operator [] (uint32_t index) const { return m_points[index]; }

private:
	points_t m_points;
};

}

