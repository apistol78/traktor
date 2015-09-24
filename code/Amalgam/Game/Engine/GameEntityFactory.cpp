#include "Amalgam/Game/Engine/GameEntity.h"
#include "Amalgam/Game/Engine/GameEntityData.h"
#include "Amalgam/Game/Engine/GameEntityFactory.h"
#include "World/IEntityBuilder.h"
#include "World/IEntityComponentData.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.GameEntityFactory", GameEntityFactory, world::IEntityFactory)

GameEntityFactory::GameEntityFactory(resource::IResourceManager* resourceManager)
:	m_resourceManager(resourceManager)
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
	const GameEntityData* gameEntityData = mandatory_non_null_type_cast< const GameEntityData* >(&entityData);
	Ref< GameEntity > gameEntity = new GameEntity();

	// Create controlled entity.
	if (gameEntityData->m_entityData)
	{
		if ((gameEntity->m_entity = builder->create(gameEntityData->m_entityData)) == 0)
			return 0;

		gameEntity->m_entity->setTransform(gameEntityData->getTransform());
	}

	// Create components.
	gameEntity->m_components.resize(gameEntityData->m_components.size());
	for (uint32_t i = 0; i < gameEntityData->m_components.size(); ++i)
	{
		if ((gameEntity->m_components[i] = gameEntityData->m_components[i]->createInstance(gameEntity, m_resourceManager)) == 0)
			return 0;
	}

	return gameEntity;
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
