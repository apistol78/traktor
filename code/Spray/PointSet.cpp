#include "Spray/PointSet.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.PointSet", PointSet, ISerializable)

PointSet::PointSet()
{
}

PointSet::PointSet(const AlignedVector< Point >& points)
:	m_points(points)
{
}

bool PointSet::serialize(ISerializer& s)
{
	return s >> MemberAlignedVector< Point, MemberComposite< Point > >(L"points", m_points);
}

bool PointSet::Point::serialize(ISerializer& s)
{
	s >> Member< Vector4 >(L"position", position);
	s >> Member< Vector4 >(L"normal", normal);
	s >> Member< Vector4 >(L"color", color);
	return true;
}

	}
}
