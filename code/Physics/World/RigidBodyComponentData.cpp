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

RigidBodyComponentData::RigidBodyComponentData()
{
}

RigidBodyComponentData::RigidBodyComponentData(BodyDesc* bodyDesc)
:	m_bodyDesc(bodyDesc)
{
}

RigidBodyComponentData::RigidBodyComponentData(BodyDesc* bodyDesc, world::IEntityEventData* eventCollide)
:	m_bodyDesc(bodyDesc)
,	m_eventCollide(eventCollide)
{
}

Ref< RigidBodyComponent > RigidBodyComponentData::createComponent(
	const world::IEntityBuilder* entityBuilder,
	world::IEntityEventManager* eventManager,
	resource::IResourceManager* resourceManager,
	PhysicsManager* physicsManager
) const
{
	Ref< Body > body = physicsManager->createBody(resourceManager, m_bodyDesc);
	if (!body)
		return 0;

	body->setEnable(false);

	Ref< world::IEntityEvent > eventCollide;
	if (m_eventCollide)
	{
		eventCollide = entityBuilder->create(m_eventCollide);
		if (!eventCollide)
			return 0;
	}

	return new RigidBodyComponent(body, eventManager, eventCollide);
}

void RigidBodyComponentData::serialize(ISerializer& s)
{
	s >> MemberRef< BodyDesc >(L"bodyDesc", m_bodyDesc);
	s >> MemberRef< world::IEntityEventData >(L"eventCollide", m_eventCollide);
}

	}
}
