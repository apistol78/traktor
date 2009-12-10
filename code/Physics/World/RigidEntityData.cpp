#include "Physics/World/RigidEntityData.h"
#include "Physics/World/RigidEntity.h"
#include "Physics/PhysicsManager.h"
#include "Physics/BodyDesc.h"
#include "Physics/Body.h"
#include "World/Entity/SpatialEntityData.h"
#include "World/Entity/EntityInstance.h"
#include "World/Entity/IEntityBuilder.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"

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
	if (!m_bodyDesc && !m_instance)
		return 0;

	if (!m_bodyDesc->bind(resourceManager))
		return 0;

	Ref< Body > body = physicsManager->createBody(m_bodyDesc);
	if (!body)
		return 0;

	body->setTransform(getTransform());

	Ref< world::SpatialEntity > entity;
	if (m_instance)
	{
		entity = checked_type_cast< world::SpatialEntity* >(builder->build(m_instance));
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
	s >> MemberRef< world::EntityInstance >(L"instance", m_instance);

	return true;
}

	}
}
