#ifndef traktor_spark_Triangulator_H
#define traktor_spark_Triangulator_H

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Vector2i.h"

namespace traktor
{
	namespace spark
	{

/*! \brief Trapezoid triangulation.
 * \ingroup Flash
 */
class Triangulator : public Object
{
	T_RTTI_CLASS;

public:
	struct Segment
	{
		bool curve;
		Vector2i v[2];
		Vector2i c;
	};

	enum TriangleType
	{
		TcFill = 0,
		TcIn = 1,
		TcOut = 2
	};

	struct Triangle
	{
		TriangleType type;
		Vector2i v[3];
	};

	void triangulate(const AlignedVector< Segment >& segments, AlignedVector< Triangle >& outTriangles);

private:
	AlignedVector< Segment > m_segments;
	AlignedVector< Segment > m_slabs;
};

	}
}

#endif	// traktor_spark_Triangulator_H
