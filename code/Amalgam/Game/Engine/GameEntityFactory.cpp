#include "Amalgam/Game/Engine/GameEntity.h"
#include "Amalgam/Game/Engine/GameEntityData.h"
#include "Amalgam/Game/Engine/GameEntityFactory.h"
#include "Core/Serialization/DeepClone.h"
#include "World/EntityEventSetData.h"
#include "World/IEntityBuilder.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.GameEntityFactory", GameEntityFactory, world::IEntityFactory)

GameEntityFactory::GameEntityFactory(world::IEntityEventManager* eventManager)
:	m_eventManager(eventManager)
{
}

const TypeInfoSet GameEntityFactory::getEntityTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< GameEntityData >());
	return typeSet;
}

const TypeInfoSet GameEntityFactory::getEntityEventTypes() const
{
	return TypeInfoSet();
}

Ref< world::Entity > GameEntityFactory::createEntity(
	const world::IEntityBuilder* builder,
	const world::EntityData& entityData
) const
{
	const GameEntityData* gameEntityData = checked_type_cast< const GameEntityData*, false >(&entityData);

	Ref< Object > object;
	if (gameEntityData->m_object)
	{
		if ((object = DeepClone(gameEntityData->m_object).create()) == 0)
			return 0;
	}

	Ref< world::Entity > entity;
	if (gameEntityData->m_entityData)
	{
		if ((entity = builder->create(gameEntityData->m_entityData)) == 0)
			return 0;

		entity->setTransform(gameEntityData->getTransform());
	}

	Ref< world::EntityEventSet > eventSet;
	if (gameEntityData->m_eventSetData)
	{
		if ((eventSet = gameEntityData->m_eventSetData->create(builder)) == 0)
			return 0;
	}

	return new GameEntity(
		gameEntityData->m_tag,
		object,
		entity,
		eventSet,
		m_eventManager
	);
}

Ref< world::IEntityEvent > GameEntityFactory::createEntityEvent(
	const world::IEntityBuilder* builder,
	const world::IEntityEventData& entityEventData
) const
{
	return 0;
}

	}
}
