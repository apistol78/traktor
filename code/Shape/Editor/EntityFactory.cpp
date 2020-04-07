#include "Shape/Editor/EntityFactory.h"
#include "Shape/Editor/Spline/ControlPointComponent.h"
#include "Shape/Editor/Spline/ControlPointComponentData.h"
#include "Shape/Editor/Spline/SplineEntity.h"
#include "Shape/Editor/Spline/SplineEntityData.h"
#include "Shape/Editor/Spline/SplineLayerComponent.h"
#include "Shape/Editor/Spline/SplineLayerComponentData.h"
#include "World/IEntityBuilder.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.EntityFactory", EntityFactory, world::IEntityFactory)

EntityFactory::EntityFactory(db::Database* database)
:	m_database(database)
{
}

const TypeInfoSet EntityFactory::getEntityTypes() const
{
	return makeTypeInfoSet<
		SplineEntityData
	>();
}

const TypeInfoSet EntityFactory::getEntityEventTypes() const
{
	return TypeInfoSet();
}

const TypeInfoSet EntityFactory::getEntityComponentTypes() const
{
	return makeTypeInfoSet<
		ControlPointComponentData,
		SplineLayerComponentData
	>();
}

Ref< world::Entity > EntityFactory::createEntity(const world::IEntityBuilder* builder, const world::EntityData& entityData) const
{
	if (auto splineEntityData = dynamic_type_cast< const SplineEntityData* >(&entityData))
	{
		Ref< SplineEntity > entity = new SplineEntity();
		entity->setTransform(entityData.getTransform());

		for (auto childEntityData : splineEntityData->getEntityData())
		{
			Ref< world::Entity > childEntity = builder->create< world::Entity >(childEntityData);
			if (childEntity)
				entity->addEntity(childEntity);
		}

		return entity;
	}
	else
		return nullptr;
}

Ref< world::IEntityEvent > EntityFactory::createEntityEvent(const world::IEntityBuilder* builder, const world::IEntityEventData& entityEventData) const
{
	return nullptr;
}

Ref< world::IEntityComponent > EntityFactory::createEntityComponent(const world::IEntityBuilder* builder, const world::IEntityComponentData& entityComponentData) const
{
	if (auto controlPointData = dynamic_type_cast< const ControlPointComponentData* >(&entityComponentData))
		return controlPointData->createComponent();
	else if (auto splineLayerData = dynamic_type_cast< const SplineLayerComponentData* >(&entityComponentData))
		return splineLayerData->createComponent(m_database);
	else
		return nullptr;
}

	}
}
