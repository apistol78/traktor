#include "Physics/ShapeDesc.h"
#include "Core/Serialization/AttributeHex.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.ShapeDesc", ShapeDesc, ISerializable)

ShapeDesc::ShapeDesc()
:	m_localTransform(Transform::identity())
,	m_collisionGroup(1)
,	m_collisionMask(~0UL)
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

void ShapeDesc::setCollisionGroup(uint32_t collisionGroup)
{
	m_collisionGroup = collisionGroup;
}

uint32_t ShapeDesc::getCollisionGroup() const
{
	return m_collisionGroup;
}

void ShapeDesc::setCollisionMask(uint32_t collisionMask)
{
	m_collisionMask = collisionMask;
}

uint32_t ShapeDesc::getCollisionMask() const
{
	return m_collisionMask;
}

bool ShapeDesc::serialize(ISerializer& s)
{
	s >> MemberComposite< Transform >(L"localTransform", m_localTransform);
	s >> Member< uint32_t >(L"collisionGroup", m_collisionGroup, AttributeHex());
	s >> Member< uint32_t >(L"collisionMask", m_collisionMask, AttributeHex());
	return true;
}

	}
}
