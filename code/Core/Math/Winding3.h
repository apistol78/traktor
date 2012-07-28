#ifndef traktor_Winding3_H
#define traktor_Winding3_H

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

/*! \brief 3d point winding.
 * \ingroup Core
 */
class T_DLLCLASS Winding3
{
public:
	enum Classification
	{
		CfFront,
		CfBack,
		CfCoplanar,
		CfSpan
	};

	AlignedVector< Vector4 > points;

	Winding3();

	Winding3(const AlignedVector< Vector4 >& points);

	Winding3(const Vector4* points, size_t npoints);

	bool angleIndices(uint32_t& outI1, uint32_t& outI2, uint32_t& outI3) const;

	bool getPlane(Plane& outPlane) const;

	void split(const Plane& plane, Winding3& outFront, Winding3& outBack) const;

	int classify(const Plane& plane) const;

	float area() const;

	Vector4 center() const;
	
	bool rayIntersection(
		const Vector4& origin,
		const Vector4& direction,
		Scalar& outK,
		Vector4* outPoint = 0
	) const;
};

}

#endif	// traktor_Winding3_H
