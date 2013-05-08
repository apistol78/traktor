#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Physics/Body.h"
#include "Physics/BodyDesc.h"
#include "Physics/PhysicsManager.h"
#include "Physics/World/RigidEntity.h"
#include "Physics/World/RigidEntityData.h"
#include "World/EntityData.h"
#include "World/IEntityBuilder.h"
#include "World/IEntityEventData.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.physics.RigidEntityData", 1, RigidEntityData, world::EntityData)

Ref< RigidEntity > RigidEntityData::createEntity(
	const world::IEntityBuilder* entityBuilder,
	world::IEntityEventManager* eventManager,
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
		entity = entityBuilder->create(m_entityData);
		if (!entity)
			return 0;

		entity->setTransform(getTransform());
	}

	Ref< world::IEntityEvent > eventCollide;
	if (m_eventCollide)
	{
		eventCollide = m_eventCollide->create(entityBuilder);
		if (!eventCollide)
			return 0;
	}

	return new RigidEntity(
		body,
		entity,
		eventManager,
		eventCollide
	);
}

void RigidEntityData::setTransform(const Transform& transform)
{
	if (m_entityData)
		m_entityData->setTransform(transform);

	world::EntityData::setTransform(transform);
}

void RigidEntityData::serialize(ISerializer& s)
{
	world::EntityData::serialize(s);

	s >> MemberRef< BodyDesc >(L"bodyDesc", m_bodyDesc);
	s >> MemberRef< world::EntityData >(L"entityData", m_entityData);

	if (s.getVersion() >= 1)
		s >> MemberRef< world::IEntityEventData >(L"eventCollide", m_eventCollide);
}

	}
}
