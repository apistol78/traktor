#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberStl.h"
#include "Physics/CollisionSpecification.h"
#include "Physics/ShapeDesc.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.ShapeDesc", ShapeDesc::Version, ShapeDesc, ISerializable)

void ShapeDesc::setLocalTransform(const Transform& localTransform)
{
	m_localTransform = localTransform;
}

const Transform& ShapeDesc::getLocalTransform() const
{
	return m_localTransform;
}

void ShapeDesc::setCollisionGroup(const std::set< resource::Id< CollisionSpecification > >& collisionGroup)
{
	m_collisionGroup = collisionGroup;
}

const std::set< resource::Id< CollisionSpecification > >& ShapeDesc::getCollisionGroup() const
{
	return m_collisionGroup;
}

void ShapeDesc::setCollisionMask(const std::set< resource::Id< CollisionSpecification > >& collisionMask)
{
	m_collisionMask = collisionMask;
}

const std::set< resource::Id< CollisionSpecification > >& ShapeDesc::getCollisionMask() const
{
	return m_collisionMask;
}

void ShapeDesc::setMaterial(int32_t material)
{
	m_material = material;
}

int32_t ShapeDesc::getMaterial() const
{
	return m_material;
}

void ShapeDesc::serialize(ISerializer& s)
{
	T_FATAL_ASSERT(s.getVersion() >= 5);
	s >> MemberComposite< Transform >(L"localTransform", m_localTransform);
	s >> MemberStlSet< resource::Id< CollisionSpecification >, resource::Member< CollisionSpecification > >(L"collisionGroup", m_collisionGroup);
	s >> MemberStlSet< resource::Id< CollisionSpecification >, resource::Member< CollisionSpecification > >(L"collisionMask", m_collisionMask);
	s >> Member< int32_t >(L"material", m_material);
}

	}
}
