#include "Physics/ShapeDesc.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.ShapeDesc", ShapeDesc, Serializable)

ShapeDesc::ShapeDesc()
:	m_localTransform(Matrix44::identity())
{
}

void ShapeDesc::setLocalTransform(const Matrix44& localTransform)
{
	m_localTransform = localTransform;
}

const Matrix44& ShapeDesc::getLocalTransform() const
{
	return m_localTransform;
}

bool ShapeDesc::serialize(Serializer& s)
{
	return s >> Member< Matrix44 >(L"localTransform", m_localTransform);
}

	}
}
