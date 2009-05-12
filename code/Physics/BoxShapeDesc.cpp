#include "Physics/BoxShapeDesc.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.physics.BoxShapeDesc", BoxShapeDesc, ShapeDesc)

BoxShapeDesc::BoxShapeDesc()
:	m_extent(0.0f, 0.0f, 0.0f, 0.0f)
{
}

void BoxShapeDesc::setExtent(const Vector4& extent)
{
	m_extent = extent;
}

const Vector4& BoxShapeDesc::getExtent() const
{
	return m_extent;
}

bool BoxShapeDesc::serialize(Serializer& s)
{
	if (!ShapeDesc::serialize(s))
		return false;

	return s >> Member< Vector4 >(L"extent", m_extent);
}

	}
}
