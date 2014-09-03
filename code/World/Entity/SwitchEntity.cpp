#include "World/Entity/SwitchEntity.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.SwitchEntity", SwitchEntity, Entity)

SwitchEntity::SwitchEntity()
:	m_transform(Transform::identity())
,	m_active(0)
{
}

SwitchEntity::SwitchEntity(const Transform& transform, int32_t active)
:	m_transform(transform)
,	m_active(active)
{
}

void SwitchEntity::destroy()
{
	for (RefArray< Entity >::iterator i = m_entities.begin(); i != m_entities.end(); ++i)
	{
		if (*i)
			(*i)->destroy();
	}
	m_entities.resize(0);
}

void SwitchEntity::setActive(int32_t active)
{
	if (m_active != active)
	{
		m_active = active;

		Entity* activeEntity = getActiveEntity();
		if (activeEntity)
			activeEntity->setTransform(m_transform);
	}
}

int32_t SwitchEntity::getActive() const
{
	return m_active;
}

Entity* SwitchEntity::getActiveEntity() const
{
	if (m_active >= 0 && m_active < int32_t(m_entities.size()))
		return m_entities[m_active];
	else
		return 0;
}

void SwitchEntity::update(const UpdateParams& update)
{
	Entity* activeEntity = getActiveEntity();
	if (activeEntity)
		activeEntity->update(update);
}

void SwitchEntity::setTransform(const Transform& transform)
{
	Entity* activeEntity = getActiveEntity();
	if (activeEntity)
		activeEntity->setTransform(transform);

	m_transform = transform;
}

bool SwitchEntity::getTransform(Transform& outTransform) const
{
	outTransform = m_transform;
	return true;
}

Aabb3 SwitchEntity::getBoundingBox() const
{
	Entity* activeEntity = getActiveEntity();
	if (activeEntity)
		return activeEntity->getBoundingBox();
	else
		return Aabb3();
}

	}
}
