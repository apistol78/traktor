#include "Core/Misc/SafeDestroy.h"
#include "Physics/Body.h"
#include "Physics/CollisionListener.h"
#include "Physics/World/RigidEntity.h"
#include "World/IEntityEvent.h"
#include "World/IEntityEventManager.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.RigidEntity", RigidEntity, world::Entity)

RigidEntity::RigidEntity(
	Body* body,
	world::Entity* entity,
	world::IEntityEventManager* eventManager,
	world::IEntityEvent* eventCollide
)
:	m_body(body)
,	m_entity(entity)
,	m_eventManager(eventManager)
,	m_eventCollide(eventCollide)
{
	m_body->addCollisionListener(physics::createCollisionListener(this, &RigidEntity::collisionListener));
}

RigidEntity::~RigidEntity()
{
}

void RigidEntity::destroy()
{
	safeDestroy(m_body);
	safeDestroy(m_entity);
	m_eventManager = 0;
	m_eventCollide = 0;
}

void RigidEntity::update(const world::UpdateParams& update)
{
	if (m_entity)
	{
		if (m_body)
			m_entity->setTransform(m_body->getTransform());

		m_entity->update(update);
	}
}

void RigidEntity::setTransform(const Transform& transform)
{
	if (m_body)
		m_body->setTransform(transform);
	if (m_entity)
		m_entity->setTransform(transform);
}

bool RigidEntity::getTransform(Transform& outTransform) const
{
	if (m_body)
	{
		outTransform = m_body->getTransform();
		return true;
	}

	if (m_entity)
		return m_entity->getTransform(outTransform);

	return false;
}

Aabb3 RigidEntity::getBoundingBox() const
{
	return m_entity ? m_entity->getBoundingBox() : Aabb3();
}

void RigidEntity::collisionListener(const physics::CollisionInfo& collisionInfo)
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

	Transform T;
	getTransform(T);

	m_eventManager->raise(m_eventCollide, this, T.inverse() * Tworld);
}

	}
}
