#include "Amalgam/Engine/GameEntity.h"
#include "Core/Misc/SafeDestroy.h"
#include "World/IEntityEventManager.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.GameEntity", GameEntity, world::Entity)

GameEntity::GameEntity(
	const std::wstring& tag,
	Object* object,
	world::Entity* entity,
	world::EntityEventSet* eventSet,
	world::IEntityEventManager* eventManager
)
:	m_tag(tag)
,	m_object(object)
,	m_entity(entity)
,	m_eventSet(eventSet)
,	m_eventManager(eventManager)
{
}

void GameEntity::destroy()
{
	safeDestroy(m_entity);
	m_object = 0;
	m_eventSet = 0;
	m_eventManager = 0;
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
	if (m_entity)
		m_entity->update(update);
}

world::IEntityEventInstance* GameEntity::raiseEvent(const std::wstring& eventId)
{
	if (m_eventManager)
		return m_eventManager->raise(m_eventSet, eventId, this);
	else
		return 0;
}

world::IEntityEventInstance* GameEntity::raiseEvent(const std::wstring& eventId, const Transform& Toffset)
{
	if (m_eventManager)
		return m_eventManager->raise(m_eventSet, eventId, this, Toffset);
	else
		return 0;
}

	}
}
