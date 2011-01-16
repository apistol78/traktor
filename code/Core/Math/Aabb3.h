#ifndef traktor_Aabb3_H
#define traktor_Aabb3_H

#include "Core/Config.h"
#include "Core/Math/MathConfig.h"
#include "Core/Math/Vector4.h"
#include "Core/Math/Matrix44.h"
#include "Core/Math/Transform.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Axis aligned bounding box.
 * \ingroup Core
 */
class T_MATH_ALIGN16 T_DLLCLASS Aabb3
{
public:
	Vector4 mn;
	Vector4 mx;

	Aabb3();
	
	Aabb3(const Aabb3& aabb);
	
	explicit Aabb3(const Vector4& mn_, const Vector4& mx_);

	/*! \brief Get bounding box corners.
	 *
	 * \param extents Output array of corner points.
	 */
	void getExtents(Vector4 extents[8]) const;

	/*! \brief Return true if point is inside bounding box.
	 *
	 * \param pt Point
	 * \return True if inside.
	 */
	bool inside(const Vector4& pt) const;

	/*! \brief Ray intersection query.
	 *
	 * \param p Start point.
	 * \param d Ray direction.
	 * \param outDistance Distance from start point to enter intersection.
	 * \return True if intersection found.
	 */
	bool intersectRay(const Vector4& p, const Vector4& d, Scalar& outDistance) const;

	/*! \brief Ray intersection query.
	*
	* \param p Start point.
	* \param d Ray direction.
	* \param outDistanceEnter Distance from start point to enter intersection.
	* \param outDistanceExit Distance from start point to exit intersection.
	* \return True if intersection found.
	*/
	bool intersectRay(const Vector4& p, const Vector4& d, Scalar& outDistanceEnter, Scalar& outDistanceExit) const;

	/*! \brief Line segment intersection query.
	 *
	 * \param p1 Start point.
	 * \param p2 End point.
	 * \param outDistance Distance from start point to intersection.
	 * \return True if intersection found.
	 */
	bool intersectSegment(const Vector4& p1, const Vector4& p2, Scalar& outDistance) const;

	/*! \brief Transform bounding box.
	 *
	 * \param m Transformation matrix.
	 * \return Transformed bounding box.
	 */
	Aabb3 transform(const Matrix44& m) const;

	/*! \brief Transform bounding box.
	*
	* \param m Transformation.
	* \return Transformed bounding box.
	*/
	Aabb3 transform(const Transform& tf) const;

	/*! \brief Return 6 faces, 4 indices each (24 integers total). */
	static const int* getFaces();

	/*! \brief Return 12 edges, 2 indices to vertex each (24 integers total). */
	static const int* getEdges();

	/*! \brief Return 12 edges, 2 indices to face each (24 integers total). */
	static const int* getEdgeAdjacency();

	/*! \brief Return 6 normals. */
	static const Vector4* getNormals();

	/*! \brief Expand bounding box to contain point. */
	T_MATH_INLINE Aabb3& contain(const Vector4& pt)
	{
		mn = min(mn, pt);
		mx = max(mx, pt);
		return *this;
	}

	/*! \brief Expand bounding box to contain sphere. */
	T_MATH_INLINE Aabb3& contain(const Vector4& center, const Scalar& radius)
	{
		mn = min(mn, center - radius);
		mx = max(mx, center + radius);
		return *this;
	}

	/*! \brief Expand bounding box to contain bounding box. */
	T_MATH_INLINE Aabb3& contain(const Aabb3& aabb)
	{
		if (!aabb.empty())
		{
			contain(aabb.mn);
			contain(aabb.mx);
		}
		return *this;
	}

	/*! \brief Get center of bounding box. */
	T_MATH_INLINE Vector4 getCenter() const
	{
		return ((mn + mx) * Scalar(0.5f)).xyz1();
	}

	/*! \brief Get extent of bounding box. */
	T_MATH_INLINE Vector4 getExtent() const
	{
		return ((mx - mn) * Scalar(0.5f)).xyz0();
	}

	/*! \brief Check if bounding box is empty. */
	T_MATH_INLINE bool empty() const
	{
		return mx.x() < mn.x();
	}
};

}

#endif	// traktor_Aabb3_H
