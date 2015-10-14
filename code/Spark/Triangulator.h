#ifndef traktor_spark_Triangulator_H
#define traktor_spark_Triangulator_H

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Vector2i.h"

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

class Path;

/*! \brief Trapezoid triangulation.
 * \ingroup Spark
 */
class T_DLLCLASS Triangulator : public Object
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

	void triangulate(const Path* path, AlignedVector< Triangle >& outTriangles);

	void triangulate(const AlignedVector< Segment >& segments, AlignedVector< Triangle >& outTriangles);

private:
	AlignedVector< Segment > m_segments;
	AlignedVector< Segment > m_slabs;
};

	}
}

#endif	// traktor_spark_Triangulator_H
