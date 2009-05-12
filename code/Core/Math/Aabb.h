#ifndef traktor_Aabb_H
#define traktor_Aabb_H

#include "Core/Config.h"
#include "Core/Math/MathConfig.h"
#include "Core/Math/Vector4.h"
#include "Core/Math/Matrix44.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Axis aligned bounding box.
 * \ingroup Core
 */
class T_DLLCLASS Aabb
{
public:
	Vector4 mn;
	Vector4 mx;

	Aabb();
	
	Aabb(const Aabb& aabb);
	
	explicit Aabb(const Vector4& mn_, const Vector4& mx_);

	void getExtents(Vector4 extents[8]) const;

	bool inside(const Vector4& pt) const;

	bool intersect(const Vector4& p1, const Vector4& p2, Scalar& outDistance) const;

	Aabb transform(const Matrix44& m) const;

	/*! \brief Return 6 faces, 4 indices each (24 integers total). */
	static const int* getFaces();

	/*! \brief Return 12 edges, 2 indices to vertex each (24 integers total). */
	static const int* getEdges();

	/*! \brief Return 12 edges, 2 indices to face each (24 integers total). */
	static const int* getEdgeAdjacency();

	/*! \brief Return 6 normals. */
	static const Vector4* getNormals();

	/*! \brief Expand bounding box to contain point. */
	T_MATH_INLINE Aabb& contain(const Vector4& pt)
	{
		mn = min(mn, pt);
		mx = max(mx, pt);
		return *this;
	}

	/*! \brief Expand bounding box to contain sphere. */
	T_MATH_INLINE Aabb& contain(const Vector4& center, const Scalar& radius)
	{
		mn = min(mn, center - radius);
		mx = max(mx, center + radius);
		return *this;
	}

	/*! \brief Expand bounding box to contain bounding box. */
	T_MATH_INLINE Aabb& contain(const Aabb& aabb)
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

#endif	// traktor_Aabb_H
