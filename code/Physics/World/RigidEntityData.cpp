#include "Physics/World/RigidEntityData.h"
#include "Physics/World/RigidEntity.h"
#include "Physics/PhysicsManager.h"
#include "Physics/BodyDesc.h"
#include "Physics/Body.h"
#include "World/Entity/SpatialEntityData.h"
#include "World/Entity/EntityBuilder.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/MemberRef.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.physics.RigidEntityData", RigidEntityData, world::SpatialEntityData)

RigidEntity* RigidEntityData::createEntity(
	world::EntityBuilder* builder,
	PhysicsManager* physicsManager
) const
{
	if (!m_bodyDesc && !m_entityData)
		return 0;

	Ref< Body > body = physicsManager->createBody(m_bodyDesc);
	if (!body)
		return 0;

	body->setTransform(getTransform());

	Ref< world::SpatialEntity > entity;
	if (m_entityData)
	{
		m_entityData->setTransform(getTransform());

		entity = checked_type_cast< world::SpatialEntity* >(builder->build(m_entityData));
		if (!entity)
			return 0;

		entity->setTransform(getTransform());
	}

	return gc_new< RigidEntity >(
		body,
		entity
	);
}

void RigidEntityData::setTransform(const Matrix44& transform)
{
	if (m_entityData)
		m_entityData->setTransform(transform);

	world::SpatialEntityData::setTransform(transform);
}

bool RigidEntityData::serialize(Serializer& s)
{
	if (!world::SpatialEntityData::serialize(s))
		return false;

	s >> MemberRef< BodyDesc >(L"bodyDesc", m_bodyDesc);
	s >> MemberRef< world::SpatialEntityData >(L"entityData", m_entityData);

	return true;
}

	}
}
