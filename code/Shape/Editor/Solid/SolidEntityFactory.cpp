#include "World/IEntityBuilder.h"
#include "World/Entity/GroupEntity.h"
#include "Shape/Editor/Solid/PrimitiveEntity.h"
#include "Shape/Editor/Solid/PrimitiveEntityData.h"
#include "Shape/Editor/Solid/SolidEntity.h"
#include "Shape/Editor/Solid/SolidEntityData.h"
#include "Shape/Editor/Solid/SolidEntityFactory.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.SolidEntityFactory", SolidEntityFactory, world::IEntityFactory)

const TypeInfoSet SolidEntityFactory::getEntityTypes() const
{
	return makeTypeInfoSet< PrimitiveEntityData, SolidEntityData >();
}

const TypeInfoSet SolidEntityFactory::getEntityEventTypes() const
{
	return TypeInfoSet();
}

const TypeInfoSet SolidEntityFactory::getEntityComponentTypes() const
{
	return TypeInfoSet();
}

Ref< world::Entity > SolidEntityFactory::createEntity(const world::IEntityBuilder* builder, const world::EntityData& entityData) const
{
	if (auto primitiveEntityData = dynamic_type_cast< const PrimitiveEntityData* >(&entityData))
	{
		return primitiveEntityData->createEntity();
	}
	else if (auto solidEntityData = dynamic_type_cast< const SolidEntityData* >(&entityData))
	{
		Ref< SolidEntity > solidEntity = new SolidEntity(solidEntityData->getTransform());
		for (auto childEntityData : solidEntityData->getEntityData())
		{
			Ref< PrimitiveEntity > primitiveEntity = builder->create< PrimitiveEntity >(childEntityData);
			if (primitiveEntity)
				solidEntity->addEntity(primitiveEntity);
		}
		return solidEntity;
	}
	else
		return nullptr;
}

Ref< world::IEntityEvent > SolidEntityFactory::createEntityEvent(const world::IEntityBuilder* builder, const world::IEntityEventData& entityEventData) const
{
	return nullptr;
}

Ref< world::IEntityComponent > SolidEntityFactory::createEntityComponent(const world::IEntityBuilder* builder, const world::IEntityComponentData& entityComponentData) const
{
	return nullptr;
}

	}
}
