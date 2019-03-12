#include "Illuminate/Editor/IlluminateEntityData.h"
#include "Illuminate/Editor/IlluminateEntityFactory.h"
#include "Mesh/MeshComponentData.h"
#include "World/IEntityBuilder.h"
#include "World/Entity/ComponentEntityData.h"
#include "World/Entity/GroupEntity.h"

namespace traktor
{
	namespace illuminate
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.illuminate.IlluminateEntityFactory", IlluminateEntityFactory, world::IEntityFactory)

const TypeInfoSet IlluminateEntityFactory::getEntityTypes() const
{
	return makeTypeInfoSet< IlluminateEntityData >();
}

const TypeInfoSet IlluminateEntityFactory::getEntityEventTypes() const
{
	return TypeInfoSet();
}

const TypeInfoSet IlluminateEntityFactory::getEntityComponentTypes() const
{
	return TypeInfoSet();
}

Ref< world::Entity > IlluminateEntityFactory::createEntity(const world::IEntityBuilder* builder, const world::EntityData& entityData) const
{
	const IlluminateEntityData* illumEntityData = checked_type_cast< const IlluminateEntityData* >(&entityData);

	/*
	\tbd Create an IlluminateEntity which can toggle between realtime and baked.
	*/

	/*
	Ref< world::GroupEntity > batchEntity = new world::GroupEntity(illumEntityData->getTransform());
	for (auto childEntityData : illumEntityData->getEntityData())
	{
		Ref< world::Entity > childEntity = builder->create(childEntityData);
		if (childEntity)
			batchEntity->addEntity(childEntity);
	}
	return batchEntity;
	*/

	Ref< world::ComponentEntityData > meshEntityData = new world::ComponentEntityData();
	meshEntityData->setName(illumEntityData->getName());
	meshEntityData->setComponent(new mesh::MeshComponentData(
		resource::Id< mesh::IMesh >(illumEntityData->getSeedGuid().permutate(1))
	));
	return builder->getCompositeEntityBuilder()->create(meshEntityData);

}

Ref< world::IEntityEvent > IlluminateEntityFactory::createEntityEvent(const world::IEntityBuilder* builder, const world::IEntityEventData& entityEventData) const
{
	return nullptr;
}

Ref< world::IEntityComponent > IlluminateEntityFactory::createEntityComponent(const world::IEntityBuilder* builder, const world::IEntityComponentData& entityComponentData) const
{
	return nullptr;
}

	}
}
