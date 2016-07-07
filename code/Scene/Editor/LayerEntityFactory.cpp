#include "Scene/Editor/LayerEntityFactory.h"
#include "World/IEntityBuilder.h"
#include "World/Editor/LayerEntityData.h"
#include "World/Entity/GroupEntity.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.LayerEntityFactory", LayerEntityFactory, world::IEntityFactory)

const TypeInfoSet LayerEntityFactory::getEntityTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< world::LayerEntityData >());
	return typeSet;
}

const TypeInfoSet LayerEntityFactory::getEntityEventTypes() const
{
	return TypeInfoSet();
}

const TypeInfoSet LayerEntityFactory::getEntityComponentTypes() const
{
	return TypeInfoSet();
}

Ref< world::Entity > LayerEntityFactory::createEntity(const world::IEntityBuilder* builder, const world::EntityData& entityData) const
{
	const world::LayerEntityData* layerData = checked_type_cast< const world::LayerEntityData*, false >(&entityData);

	Ref< world::GroupEntity > groupEntity = new world::GroupEntity(layerData->getTransform());

	const RefArray< world::EntityData >& childEntityData = layerData->getEntityData();
	for (RefArray< world::EntityData >::const_iterator i = childEntityData.begin(); i != childEntityData.end(); ++i)
	{
		Ref< world::Entity > childEntity = builder->create(*i);
		if (childEntity)
			groupEntity->addEntity(childEntity);
	}

	return groupEntity;
}

Ref< world::IEntityEvent > LayerEntityFactory::createEntityEvent(const world::IEntityBuilder* builder, const world::IEntityEventData& entityEventData) const
{
	return 0;
}

Ref< world::IEntityComponent > LayerEntityFactory::createEntityComponent(const world::IEntityBuilder* builder, const world::IEntityComponentData& entityComponentData) const
{
	return 0;
}

	}
}
