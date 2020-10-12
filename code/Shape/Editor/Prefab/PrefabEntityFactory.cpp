#include "World/IEntityBuilder.h"
#include "World/Entity.h"
#include "World/Entity/GroupComponent.h"
#include "Shape/Editor/Prefab/PrefabEntityData.h"
#include "Shape/Editor/Prefab/PrefabEntityFactory.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.PrefabEntityFactory", PrefabEntityFactory, world::IEntityFactory)

const TypeInfoSet PrefabEntityFactory::getEntityTypes() const
{
	return makeTypeInfoSet< PrefabEntityData >();
}

const TypeInfoSet PrefabEntityFactory::getEntityEventTypes() const
{
	return TypeInfoSet();
}

const TypeInfoSet PrefabEntityFactory::getEntityComponentTypes() const
{
	return TypeInfoSet();
}

Ref< world::Entity > PrefabEntityFactory::createEntity(const world::IEntityBuilder* builder, const world::EntityData& entityData) const
{
	const PrefabEntityData* prefabEntityData = checked_type_cast< const PrefabEntityData* >(&entityData);

	Ref< world::GroupComponent > batchGroup = new world::GroupComponent();
	for (auto childEntityData : prefabEntityData->getEntityData())
	{
		Ref< world::Entity > childEntity = builder->create(childEntityData);
		if (childEntity)
			batchGroup->addEntity(childEntity);
	}

	Ref< world::Entity > batchEntity = new world::Entity(prefabEntityData->getName(), prefabEntityData->getTransform());
	batchEntity->setComponent(batchGroup);
	return batchEntity;
}

Ref< world::IEntityEvent > PrefabEntityFactory::createEntityEvent(const world::IEntityBuilder* builder, const world::IEntityEventData& entityEventData) const
{
	return nullptr;
}

Ref< world::IEntityComponent > PrefabEntityFactory::createEntityComponent(const world::IEntityBuilder* builder, const world::IEntityComponentData& entityComponentData) const
{
	return nullptr;
}

	}
}
