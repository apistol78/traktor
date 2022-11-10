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
#include "Core/Math/Plane.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Winding2;

/*! 3D point winding.
 * \ingroup Core
 */
class T_DLLCLASS Winding3
{
public:
	typedef AlignedVector< Vector4 > points_t;

	enum Classification
	{
		CfFront = 1,
		CfBack = 2,
		CfSpan = (CfFront | CfBack),
		CfCoplanar = 4
	};

	Winding3();

	explicit Winding3(uint32_t size);

	explicit Winding3(const points_t& points);

	explicit Winding3(const Vector4* points, size_t npoints);

	explicit Winding3(const Vector4& p1, const Vector4& p2, const Vector4& p3);

	/*! Clear all points. */
	void clear();

	/*! Add another point to winding. */
	void push(const Vector4& p);

	/*! */
	bool angleIndices(uint32_t& outI1, uint32_t& outI2, uint32_t& outI3) const;

	/*! Calculate 2D projection of winding. */
	bool getProjection(Winding2& outProjection, Vector4& outU, Vector4& outV) const;

	/*! Get plane of winding. */
	bool getPlane(Plane& outPlane) const;

	/*! Split winding by plane. */
	void split(const Plane& plane, Winding3& outFront, Winding3& outBack) const;

	/*! Classify split of winding and plane. */
	int classify(const Plane& plane) const;

	/*! Calculate area of winding. */
	float area() const;

	/*! Calculate center of winding. */
	Vector4 center() const;

	/*! Calculate intersection of ray and this winding. */
	bool rayIntersection(
		const Vector4& origin,
		const Vector4& direction,
		Scalar& outK,
		Vector4* outPoint = 0
	) const;

	/*! Flip order of this winding. */
	void flip();

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
	Vector4& operator [] (uint32_t index) { return m_points[index]; }

	/*! Get point by index. */
	const Vector4& operator [] (uint32_t index) const { return m_points[index]; }

private:
	points_t m_points;
};

}
