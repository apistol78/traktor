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
	
	Ref< world::GroupEntity > groupEntity = new world::GroupEntity(
		layerData->getTransform(),
		world::EmAll
	);

	for (auto childEntityData : layerData->getEntityData())
	{
		Ref< world::Entity > childEntity = builder->create(childEntityData);
		if (childEntity)
			groupEntity->addEntity(childEntity);
	}

	return groupEntity;
}

Ref< world::IEntityEvent > LayerEntityFactory::createEntityEvent(const world::IEntityBuilder* builder, const world::IEntityEventData& entityEventData) const
{
	return nullptr;
}

Ref< world::IEntityComponent > LayerEntityFactory::createEntityComponent(const world::IEntityBuilder* builder, const world::IEntityComponentData& entityComponentData) const
{
	return nullptr;
}

	}
}
