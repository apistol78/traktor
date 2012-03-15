#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Physics/Body.h"
#include "Physics/BodyDesc.h"
#include "Physics/PhysicsManager.h"
#include "Physics/World/RigidEntity.h"
#include "Physics/World/RigidEntityData.h"
#include "World/Entity/IEntityBuilder.h"
#include "World/Entity/EntityData.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.physics.RigidEntityData", 0, RigidEntityData, world::EntityData)

Ref< RigidEntity > RigidEntityData::createEntity(
	world::IEntityBuilder* builder,
	resource::IResourceManager* resourceManager,
	PhysicsManager* physicsManager
) const
{
	Ref< Body > body = physicsManager->createBody(resourceManager, m_bodyDesc);
	if (!body)
		return 0;

	body->setTransform(getTransform());
	body->setEnable(true);

	Ref< world::Entity > entity;
	if (m_entityData)
	{
		entity = builder->create(m_entityData);
		if (!entity)
			return 0;

		entity->setTransform(getTransform());
	}

	return new RigidEntity(
		body,
		entity
	);
}

bool RigidEntityData::serialize(ISerializer& s)
{
	if (!world::EntityData::serialize(s))
		return false;

	s >> MemberRef< BodyDesc >(L"bodyDesc", m_bodyDesc);
	s >> MemberRef< world::EntityData >(L"entityData", m_entityData);

	return true;
}

	}
}
