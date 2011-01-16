#include "Physics/World/RigidEntity.h"
#include "Physics/Body.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.RigidEntity", RigidEntity, world::SpatialEntity)

RigidEntity::RigidEntity(
	Body* body,
	world::SpatialEntity* entity
)
:	m_body(body)
,	m_entity(entity)
{
}

RigidEntity::~RigidEntity()
{
	destroy();
}

void RigidEntity::destroy()
{
	if (m_body)
	{
		m_body->destroy();
		m_body = 0;
	}
	if (m_entity)
	{
		m_entity->destroy();
		m_entity = 0;
	}
}

void RigidEntity::update(const world::EntityUpdate* update)
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

	}
}
