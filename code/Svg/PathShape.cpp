#include "Svg/PathShape.h"

namespace traktor
{
	namespace svg
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.svg.PathShape", PathShape, Shape)

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
