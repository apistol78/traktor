#include "Physics/CapsuleShapeDesc.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.CapsuleShapeDesc", ShapeDesc::Version, CapsuleShapeDesc, ShapeDesc)

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

void CapsuleShapeDesc::serialize(ISerializer& s)
{
	ShapeDesc::serialize(s);

	s >> Member< float >(L"radius", m_radius, AttributeRange(0.0f));
	s >> Member< float >(L"length", m_length, AttributeRange(0.0f));
}

	}
}
