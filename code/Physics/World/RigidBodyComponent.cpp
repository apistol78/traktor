#include "Core/Misc/SafeDestroy.h"
#include "Physics/Body.h"
#include "Physics/World/RigidBodyComponent.h"
#include "World/Entity.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.RigidBodyComponent", RigidBodyComponent, world::IEntityComponent)

RigidBodyComponent::RigidBodyComponent(
	world::Entity* owner,
	Body* body,
	world::IEntityEventManager* eventManager,
	world::IEntityEvent* eventCollide
)
:	m_owner(owner)
,	m_body(body)
,	m_eventManager(eventManager)
,	m_eventCollide(eventCollide)
{
}

void RigidBodyComponent::destroy()
{
	m_eventCollide = 0;
	m_eventManager = 0;
	safeDestroy(m_body);
	m_owner = 0;
}

void RigidBodyComponent::setTransform(const Transform& transform)
{
	m_body->setTransform(transform);
}

Aabb3 RigidBodyComponent::getBoundingBox() const
{
	return Aabb3();
}

void RigidBodyComponent::update(const world::UpdateParams& update)
{
	m_owner->setTransform(m_body->getTransform());
}

	}
}
