#include "Physics/Joint.h"
#include "Physics/World/ArticulatedEntity.h"
#include "Physics/World/RigidEntity.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.ArticulatedEntity", ArticulatedEntity, world::Entity)

ArticulatedEntity::ArticulatedEntity(
	const Transform& transform,
	const RefArray< RigidEntity >& entities,
	const RefArray< Joint >& joints
)
:	m_transform(transform)
,	m_entities(entities)
,	m_joints(joints)
{
}

void ArticulatedEntity::destroy()
{
	for (auto joint : m_joints)
	{
		if (joint)
			joint->destroy();
	}
	m_joints.resize(0);

	for (auto entity : m_entities)
	{
		if (entity)
			entity->destroy();
	}
	m_entities.resize(0);

	world::Entity::destroy();
}

void ArticulatedEntity::update(const world::UpdateParams& update)
{
	for (auto entity : m_entities)
		entity->update(update);
}

void ArticulatedEntity::setTransform(const Transform& transform)
{
	Transform invTransform = m_transform.inverse();
	for (auto entity : m_entities)
	{
		Transform currentTransform = entity->getTransform();
		Transform Tlocal = invTransform * currentTransform;
		Transform Tworld = transform * Tlocal;
		entity->setTransform(Tworld);
	}
	m_transform = transform;
}

Transform ArticulatedEntity::getTransform() const
{
	return m_transform;
}

Aabb3 ArticulatedEntity::getBoundingBox() const
{
	Transform invTransform = m_transform.inverse();

	Aabb3 boundingBox;
	for (auto entity : m_entities)
	{
		Aabb3 childBoundingBox = entity->getWorldBoundingBox();
		if (!childBoundingBox.empty())
			boundingBox.contain(childBoundingBox.transform(invTransform));
	}

	return boundingBox;
}

	}
}
