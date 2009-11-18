#include "Physics/ShapeDesc.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.ShapeDesc", ShapeDesc, ISerializable)

ShapeDesc::ShapeDesc()
:	m_localTransform(Transform::identity())
{
}

void ShapeDesc::setLocalTransform(const Transform& localTransform)
{
	m_localTransform = localTransform;
}

const Transform& ShapeDesc::getLocalTransform() const
{
	return m_localTransform;
}

bool ShapeDesc::serialize(ISerializer& s)
{
	return s >> MemberComposite< Transform >(L"localTransform", m_localTransform);
}

	}
}
