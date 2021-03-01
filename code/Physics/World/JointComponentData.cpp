#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Physics/JointDesc.h"
#include "Physics/PhysicsManager.h"
#include "Physics/World/JointComponent.h"
#include "Physics/World/JointComponentData.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.physics.JointComponentData", 0, JointComponentData, world::IEntityComponentData)

JointComponentData::JointComponentData(JointDesc* jointDesc)
:	m_jointDesc(jointDesc)
{
}

Ref< JointComponent > JointComponentData::createComponent(PhysicsManager* physicsManager) const
{
	return new JointComponent(physicsManager, m_jointDesc);
}

void JointComponentData::setTransform(const world::EntityData* owner, const Transform& transform)
{
}

void JointComponentData::serialize(ISerializer& s)
{
	s >> MemberRef< JointDesc >(L"jointDesc", m_jointDesc);
}

	}
}
