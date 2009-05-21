#ifndef traktor_Winding_H
#define traktor_Winding_H

#include "Core/Math/Vector4.h"
#include "Core/Math/Plane.h"
#include "Core/Containers/AlignedVector.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Point winding.
 * \ingroup Core
 */
class T_DLLCLASS Winding
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

	Winding();

	Winding(const AlignedVector< Vector4 >& points);

	Winding(const Vector4* points, size_t npoints);

	bool angleIndices(uint32_t& outI1, uint32_t& outI2, uint32_t& outI3) const;

	bool getPlane(Plane& outPlane) const;

	void split(const Plane& plane, Winding& outFront, Winding& outBack) const;

	int classify(const Plane& plane) const;
};

}

#endif	// traktor_Winding_H
