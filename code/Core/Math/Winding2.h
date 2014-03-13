#ifndef traktor_Winding2_H
#define traktor_Winding2_H

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

class T_DLLCLASS Winding2
{
public:
	AlignedVector< Vector2 > points;

	/*! \brief Calculate convex hull from a random point set. */
	static Winding2 convexHull(const Vector2* pnts, int npnts);

	/*! \brief Calculate convex hull from a random point set. */
	static Winding2 convexHull(const AlignedVector< Vector2 >& pnts);

	/*! \brief Check if point is within winding. */
	bool inside(const Vector2& pnt) const;
};

}

#endif	// traktor_Winding2_H
