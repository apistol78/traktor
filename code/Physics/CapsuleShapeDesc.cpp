#include "Physics/CapsuleShapeDesc.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.physics.CapsuleShapeDesc", CapsuleShapeDesc, ShapeDesc)

CapsuleShapeDesc::CapsuleShapeDesc()
:	m_radius(0.0f)
,	m_length(0.0f)
{
}

void CapsuleShapeDesc::setRadius(float radius)
{
	m_radius = radius;
}

float CapsuleShapeDesc::getRadius() const
{
	return m_radius;
}

void CapsuleShapeDesc::setLength(float length)
{
	m_length = length;
}

float CapsuleShapeDesc::getLength() const
{
	return m_length;
}

bool CapsuleShapeDesc::serialize(Serializer& s)
{
	if (!ShapeDesc::serialize(s))
		return false;

	s >> Member< float >(L"radius", m_radius);
	s >> Member< float >(L"length", m_length);

	return true;
}

	}
}
