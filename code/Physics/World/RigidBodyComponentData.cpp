#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Physics/Body.h"
#include "Physics/BodyDesc.h"
#include "Physics/PhysicsManager.h"
#include "Physics/World/RigidBodyComponent.h"
#include "Physics/World/RigidBodyComponentData.h"
#include "World/Entity.h"
#include "World/IEntityBuilder.h"
#include "World/IEntityEvent.h"
#include "World/IEntityEventData.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.physics.RigidBodyComponentData", 0, RigidBodyComponentData, world::IEntityComponentData)

Ref< RigidBodyComponent > RigidBodyComponentData::createComponent(
	world::Entity* owner,
	const world::IEntityBuilder* entityBuilder,
	world::IEntityEventManager* eventManager,
	resource::IResourceManager* resourceManager,
	PhysicsManager* physicsManager
) const
{
	Transform transform;
	if (!owner->getTransform(transform))
		return 0;

	Ref< Body > body = physicsManager->createBody(resourceManager, m_bodyDesc);
	if (!body)
		return 0;

	body->setTransform(transform);
	body->setEnable(true);

	Ref< world::IEntityEvent > eventCollide;
	if (m_eventCollide)
	{
		eventCollide = entityBuilder->create(m_eventCollide);
		if (!eventCollide)
			return 0;
	}

	return new RigidBodyComponent(owner, body, eventManager, eventCollide);
}

void RigidBodyComponentData::serialize(ISerializer& s)
{
	s >> MemberRef< BodyDesc >(L"bodyDesc", m_bodyDesc);
	s >> MemberRef< world::IEntityEventData >(L"eventCollide", m_eventCollide);
}

	}
}
