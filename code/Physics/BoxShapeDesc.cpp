#include "Physics/BoxShapeDesc.h"
#include "Core/Serialization/AttributeDirection.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.BoxShapeDesc", 3, BoxShapeDesc, ShapeDesc)

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

void BoxShapeDesc::serialize(ISerializer& s)
{
	ShapeDesc::serialize(s);
	s >> Member< Vector4 >(L"extent", m_extent, AttributeDirection());
}

	}
}
