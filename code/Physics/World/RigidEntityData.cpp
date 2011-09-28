#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Physics/Body.h"
#include "Physics/BodyDesc.h"
#include "Physics/PhysicsManager.h"
#include "Physics/World/RigidEntity.h"
#include "Physics/World/RigidEntityData.h"
#include "World/Entity/IEntityBuilder.h"
#include "World/Entity/SpatialEntityData.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.physics.RigidEntityData", 0, RigidEntityData, world::SpatialEntityData)

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

	Ref< world::SpatialEntity > entity;
	if (m_entityData)
	{
		entity = checked_type_cast< world::SpatialEntity* >(builder->create(m_entityData));
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
	if (!world::SpatialEntityData::serialize(s))
		return false;

	s >> MemberRef< BodyDesc >(L"bodyDesc", m_bodyDesc);
	s >> MemberRef< world::SpatialEntityData >(L"entityData", m_entityData);

	return true;
}

	}
}
