#include "Core/Misc/SafeDestroy.h"
#include "Physics/Body.h"
#include "Physics/CollisionListener.h"
#include "Physics/World/RigidBodyComponent.h"
#include "World/Entity.h"
#include "World/IEntityEventManager.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.RigidBodyComponent", RigidBodyComponent, world::IEntityComponent)

RigidBodyComponent::RigidBodyComponent(
	Body* body,
	world::IEntityEventManager* eventManager,
	world::IEntityEvent* eventCollide
)
:	m_owner(0)
,	m_body(body)
,	m_eventManager(eventManager)
,	m_eventCollide(eventCollide)
{
	if (m_body && m_eventCollide)
		m_body->addCollisionListener(physics::createCollisionListener(this, &RigidBodyComponent::collisionListener));
}

void RigidBodyComponent::destroy()
{
	m_eventCollide = 0;
	m_eventManager = 0;
	safeDestroy(m_body);
	m_owner = 0;
}

void RigidBodyComponent::setOwner(world::Entity* owner)
{
	if ((m_owner = owner) != 0)
	{
		Transform transform;
		if (m_owner->getTransform(transform))
		{
			m_body->setTransform(transform);
			m_body->setEnable(true);
		}
	}
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

void RigidBodyComponent::collisionListener(const physics::CollisionInfo& collisionInfo)
{
	Vector4 position = Vector4::zero();
	Vector4 normal = Vector4::zero();

	for (AlignedVector< physics::CollisionContact >::const_iterator i = collisionInfo.contacts.begin(); i != collisionInfo.contacts.end(); ++i)
	{
		position += i->position;
		normal += i->normal;
	}

	position = position / Scalar(float(collisionInfo.contacts.size()));
	normal = normal.normalized();

	Transform Tworld(
		position,
		Quaternion(Vector4(0.0f, 1.0f, 0.0f, 0.0f), normal)
	);

	Transform T = m_body->getTransform();
	m_eventManager->raise(m_eventCollide, m_owner, T.inverse() * Tworld);
}

	}
}
