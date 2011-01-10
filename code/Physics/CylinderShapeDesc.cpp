#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Physics/CylinderShapeDesc.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.CylinderShapeDesc", 1, CylinderShapeDesc, ShapeDesc)

CylinderShapeDesc::CylinderShapeDesc()
:	m_radius(0.0f)
,	m_length(0.0f)
{
}

void CylinderShapeDesc::setRadius(float radius)
{
	m_radius = radius;
}

float CylinderShapeDesc::getRadius() const
{
	return m_radius;
}

void CylinderShapeDesc::setLength(float length)
{
	m_length = length;
}

float CylinderShapeDesc::getLength() const
{
	return m_length;
}

bool CylinderShapeDesc::serialize(ISerializer& s)
{
	if (!ShapeDesc::serialize(s))
		return false;

	s >> Member< float >(L"radius", m_radius, AttributeRange(0.0f));
	s >> Member< float >(L"length", m_length, AttributeRange(0.0f));

	return true;
}

	}
}
