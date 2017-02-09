#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Physics/SphereShapeDesc.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.SphereShapeDesc", ShapeDesc::Version, SphereShapeDesc, ShapeDesc)

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

void SphereShapeDesc::serialize(ISerializer& s)
{
	ShapeDesc::serialize(s);
	s >> Member< float >(L"radius", m_radius, AttributeRange(0.0f));
}

	}
}
