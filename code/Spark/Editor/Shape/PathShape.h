#ifndef traktor_spark_PathShape_H
#define traktor_spark_PathShape_H

#include "Spark/Editor/Shape/Path.h"
#include "Spark/Editor/Shape/Shape.h"

namespace traktor
{
	namespace spark
	{

class PathShape : public Shape
{
	T_RTTI_CLASS;

public:
	PathShape(const Path& path);

	const Path& getPath() const;

private:
	Path m_path;
};

	}
}

#endif	// traktor_spark_PathShape_H
