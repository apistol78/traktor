#include "Illuminate/Editor/IlluminateEntityData.h"
#include "Illuminate/Editor/IlluminateEntityFactory.h"
#include "World/IEntityBuilder.h"
#include "World/Entity/GroupEntity.h"

namespace traktor
{
	namespace illuminate
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.illuminate.IlluminateEntityFactory", IlluminateEntityFactory, world::IEntityFactory)

const TypeInfoSet IlluminateEntityFactory::getEntityTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< IlluminateEntityData >());
	return typeSet;
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
	
	Ref< world::GroupEntity > batchEntity = new world::GroupEntity(illumEntityData->getTransform());

	const RefArray< world::EntityData >& childEntityData = illumEntityData->getEntityData();
	for (RefArray< world::EntityData >::const_iterator i = childEntityData.begin(); i != childEntityData.end(); ++i)
	{
		Ref< world::Entity > childEntity = builder->create(*i);
		if (childEntity)
			batchEntity->addEntity(childEntity);
	}

	return batchEntity;
}

Ref< world::IEntityEvent > IlluminateEntityFactory::createEntityEvent(const world::IEntityBuilder* builder, const world::IEntityEventData& entityEventData) const
{
	return 0;
}

Ref< world::IEntityComponent > IlluminateEntityFactory::createEntityComponent(const world::IEntityBuilder* builder, const world::IEntityComponentData& entityComponentData) const
{
	return 0;
}

	}
}
