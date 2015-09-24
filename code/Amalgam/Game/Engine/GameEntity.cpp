#include "Amalgam/Game/Engine/GameEntity.h"
#include "Core/Misc/SafeDestroy.h"
#include "World/IEntityComponent.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.GameEntity", GameEntity, world::Entity)

GameEntity::GameEntity()
:	m_visible(true)
{
}

void GameEntity::destroy()
{
	safeDestroy(m_entity);
}

void GameEntity::setTransform(const Transform& transform)
{
	if (m_entity)
		m_entity->setTransform(transform);
}

bool GameEntity::getTransform(Transform& outTransform) const
{
	if (m_entity)
		return m_entity->getTransform(outTransform);
	else
		return false;
}

Aabb3 GameEntity::getBoundingBox() const
{
	if (m_entity)
		return m_entity->getBoundingBox();
	else
		return Aabb3();
}

void GameEntity::update(const world::UpdateParams& update)
{
	for (RefArray< world::IEntityComponent >::const_iterator i = m_components.begin(); i != m_components.end(); ++i)
		(*i)->update(update);

	if (m_entity)
		m_entity->update(update);
}

	}
}
