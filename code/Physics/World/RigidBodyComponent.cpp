#include "Core/Misc/SafeDestroy.h"
#include "Physics/Body.h"
#include "Physics/CollisionListener.h"
#include "Physics/World/RigidBodyComponent.h"
#include "World/Entity/ComponentEntity.h"
#include "World/EntityEventManager.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.RigidBodyComponent", RigidBodyComponent, world::IEntityComponent)

RigidBodyComponent::RigidBodyComponent(
	Body* body,
	world::EntityEventManager* eventManager,
	world::IEntityEvent* eventCollide
)
:	m_owner(nullptr)
,	m_body(body)
,	m_eventManager(eventManager)
,	m_eventCollide(eventCollide)
{
	if (m_body && m_eventCollide)
		m_body->addCollisionListener(physics::createCollisionListener(this, &RigidBodyComponent::collisionListener));
}

void RigidBodyComponent::destroy()
{
	m_eventCollide = nullptr;
	m_eventManager = nullptr;
	safeDestroy(m_body);
	m_owner = nullptr;
}

void RigidBodyComponent::setOwner(world::ComponentEntity* owner)
{
	if ((m_owner = owner) != nullptr)
	{
		Transform transform = m_owner->getTransform();
		m_body->setTransform(transform);
		m_body->setEnable(true);
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
	// Do not keep updating transform of non-kinematic statics or inactive bodies.
	if (
		!(m_body->isStatic() && !m_body->isKinematic()) &&
		m_body->isActive()
	)
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
