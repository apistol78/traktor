#pragma once

#include "Core/Config.h"
#include "Core/Math/MathConfig.h"
#include "Core/Math/MathUtils.h"
#include "Core/Math/Vector2.h"
#include "Core/Math/Matrix33.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! 2d axis aligned bounding box.
 * \ingroup Core
 */
class T_MATH_ALIGN16 T_DLLCLASS Aabb2
{
public:
	Vector2 mn;
	Vector2 mx;

	Aabb2();

	Aabb2(const Aabb2& aabb);

	Aabb2(const Vector2& mn_, const Vector2& mx_);

	/*! Get bounding box corners.
	 *
	 * \param extents Output array of corner points.
	 */
	void getExtents(Vector2 extents[4]) const;

	/*! Return true if point is inside bounding box.
	 *
	 * \param pt Point
	 * \return True if inside.
	 */
	bool inside(const Vector2& pt) const;

	/*! Intersect segment with bounding box.
	 *
	 * \param outDistance Distance from start point to intersection.
	 * \return True if intersection found.
	 */
	bool intersectSegment(const Vector2& p1, const Vector2& p2, float& outDistance) const;

	/*! Expand bounding box to contain point. */
	T_MATH_INLINE Aabb2& contain(const Vector2& pt)
	{
		mn = min(mn, pt);
		mx = max(mx, pt);
		return *this;
	}

	/*! Expand bounding box to contain sphere. */
	T_MATH_INLINE Aabb2& contain(const Vector2& center, float radius)
	{
		mn = min(mn, center - radius);
		mx = max(mx, center + radius);
		return *this;
	}

	/*! Expand bounding box to contain bounding box. */
	T_MATH_INLINE Aabb2& contain(const Aabb2& aabb)
	{
		if (!aabb.empty())
		{
			contain(aabb.mn);
			contain(aabb.mx);
		}
		return *this;
	}

	/*! Get overlapped bounding box. */
	T_MATH_INLINE Aabb2 overlapped(const Aabb2& other) const
	{
		float x1 = max(mn.x, other.mn.x);
		float y1 = max(mn.y, other.mn.y);
		float x2 = min(mx.x, other.mx.x);
		float y2 = min(mx.y, other.mx.y);
		if (x1 <= x2 && y1 <= y2)
			return Aabb2(
				Vector2(x1, y1),
				Vector2(x2, y2)
			);
		else
			return Aabb2();
	}

	/*! Get center of bounding box. */
	T_MATH_INLINE Vector2 getCenter() const
	{
		return !empty() ? (mn + mx) * 0.5f : Vector2::zero();
	}

	/*! Get extent of bounding box. */
	T_MATH_INLINE Vector2 getExtent() const
	{
		return !empty() ? (mx - mn) * 0.5f : Vector2::zero();
	}

	/*! Get size of bounding box. */
	T_MATH_INLINE Vector2 getSize() const
	{
		return !empty() ? (mx - mn) : Vector2::zero();
	}

	/*! Check if bounding box is empty. */
	T_MATH_INLINE bool empty() const
	{
		return mx.x <= mn.x || mx.y <= mn.y;
	}

	/*! Check if bounding box are equal. */
	T_MATH_INLINE bool operator == (const Aabb2& rh) const
	{
		return mn == rh.mn && mx == rh.mx;
	}

	/*! Check if bounding box are not equal. */
	T_MATH_INLINE bool operator != (const Aabb2& rh) const
	{
		return mn != rh.mn || mx != rh.mx;
	}

	/*! Transform bounding box.
	 * \note Size of bounding box might change as it's NOT an oriented bounding box.
	 */
	friend T_MATH_INLINE T_DLLCLASS Aabb2 operator * (const Matrix33& m, const Aabb2& b)
	{
		if (!b.empty())
		{
			Vector2 x[4];
			b.getExtents(x);

			Aabb2 r;
			r.contain(m * x[0]);
			r.contain(m * x[1]);
			r.contain(m * x[2]);
			r.contain(m * x[3]);

			return r;
		}
		else
			return Aabb2();
	}
};

}

