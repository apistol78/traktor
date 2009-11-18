#include "Physics/SphereShapeDesc.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.SphereShapeDesc", SphereShapeDesc, ShapeDesc)

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

bool SphereShapeDesc::serialize(ISerializer& s)
{
	if (!ShapeDesc::serialize(s))
		return false;

	return s >> Member< float >(L"radius", m_radius, 0.0f);
}

	}
}
