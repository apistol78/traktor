#include "Spark/Editor/Shape/PathShape.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.PathShape", PathShape, Shape)

PathShape::PathShape(const Path& path)
:	m_path(path)
{
}

const Path& PathShape::getPath() const
{
	return m_path;
}

	}
}
