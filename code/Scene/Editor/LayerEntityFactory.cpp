#include "Scene/Editor/LayerEntityFactory.h"
#include "World/Entity.h"
#include "World/IEntityBuilder.h"
#include "World/Editor/LayerEntityData.h"
#include "World/Entity/GroupComponent.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.LayerEntityFactory", LayerEntityFactory, world::IEntityFactory)

const TypeInfoSet LayerEntityFactory::getEntityTypes() const
{
	return makeTypeInfoSet< world::LayerEntityData >();
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
	const world::LayerEntityData* layerData = mandatory_non_null_type_cast< const world::LayerEntityData* >(&entityData);
	
	Ref< world::Entity > entity = new world::Entity(layerData->getTransform());
	Ref< world::GroupComponent > group = new world::GroupComponent();
	entity->setComponent(group);

	for (auto childEntityData : layerData->getEntityData())
	{
		Ref< world::Entity > childEntity = builder->create(childEntityData);
		if (childEntity)
			group->addEntity(childEntity);
	}

	return entity;
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
