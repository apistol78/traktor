#pragma once

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Spark/Polygon.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spark
	{

/*! Trapezoid triangulation.
 * \ingroup Spark
 */
class T_DLLCLASS Triangulator : public Object
{
	T_RTTI_CLASS;

public:
	void triangulate(const AlignedVector< Segment >& segments, uint16_t currentFillStyle, bool oddEven, AlignedVector< Triangle >& outTriangles);

private:
	AlignedVector< Segment > m_segments;
	AlignedVector< Segment > m_slabs;
};

	}
}

