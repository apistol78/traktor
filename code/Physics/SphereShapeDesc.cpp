#include "Physics/SphereShapeDesc.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.physics.SphereShapeDesc", SphereShapeDesc, ShapeDesc)

SphereShapeDesc::SphereShapeDesc()
:	m_radius(1.0f)
{
}

void SphereShapeDesc::setRadius(float radius)
{
	m_radius = radius;
}

float SphereShapeDesc::getRadius() const
{
	return m_radius;
}

bool SphereShapeDesc::serialize(Serializer& s)
{
	if (!ShapeDesc::serialize(s))
		return false;

	return s >> Member< float >(L"radius", m_radius);
}

	}
}
