#include "World/IEntityBuilder.h"
#include "World/Entity/GroupEntity.h"
#include "Amalgam/Editor/Prefab/PrefabEntityData.h"
#include "Amalgam/Editor/Prefab/PrefabEntityFactory.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.PrefabEntityFactory", PrefabEntityFactory, world::IEntityFactory)

const TypeInfoSet PrefabEntityFactory::getEntityTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< PrefabEntityData >());
	return typeSet;
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
	
	Ref< world::GroupEntity > batchEntity = new world::GroupEntity(prefabEntityData->getTransform());

	const RefArray< world::EntityData >& childEntityData = prefabEntityData->getEntityData();
	for (RefArray< world::EntityData >::const_iterator i = childEntityData.begin(); i != childEntityData.end(); ++i)
	{
		Ref< world::Entity > childEntity = builder->create(*i);
		if (childEntity)
			batchEntity->addEntity(childEntity);
	}

	return batchEntity;
}

Ref< world::IEntityEvent > PrefabEntityFactory::createEntityEvent(const world::IEntityBuilder* builder, const world::IEntityEventData& entityEventData) const
{
	return 0;
}

Ref< world::IEntityComponent > PrefabEntityFactory::createEntityComponent(const world::IEntityBuilder* builder, const world::IEntityComponentData& entityComponentData) const
{
	return 0;
}

	}
}
