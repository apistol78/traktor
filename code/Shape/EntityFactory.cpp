#include "Shape/EntityFactory.h"
#include "Shape/Spline/ControlPointEntity.h"
#include "Shape/Spline/ControlPointEntityData.h"
#include "Shape/Spline/SplineEntity.h"
#include "Shape/Spline/SplineEntityData.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.EntityFactory", EntityFactory, world::IEntityFactory)

EntityFactory::EntityFactory(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem)
:	m_resourceManager(resourceManager)
,	m_renderSystem(renderSystem)
{
}

const TypeInfoSet EntityFactory::getEntityTypes() const
{
	return makeTypeInfoSet(
		type_of< ControlPointEntityData >(),
		type_of< SplineEntityData >()
	);
}

const TypeInfoSet EntityFactory::getEntityEventTypes() const
{
	return TypeInfoSet();
}

const TypeInfoSet EntityFactory::getEntityComponentTypes() const
{
	return TypeInfoSet();
}

Ref< world::Entity > EntityFactory::createEntity(const world::IEntityBuilder* builder, const world::EntityData& entityData) const
{
	if (const SplineEntityData* splineEntityData = dynamic_type_cast< const SplineEntityData* >(&entityData))
		return splineEntityData->createEntity(builder, m_resourceManager, m_renderSystem);
	else if (const ControlPointEntityData* controlEntityData = dynamic_type_cast< const ControlPointEntityData* >(&entityData))
		return controlEntityData->createEntity(builder);
	else
		return nullptr;
}

Ref< world::IEntityEvent > EntityFactory::createEntityEvent(const world::IEntityBuilder* builder, const world::IEntityEventData& entityEventData) const
{
	return nullptr;
}

Ref< world::IEntityComponent > EntityFactory::createEntityComponent(const world::IEntityBuilder* builder, const world::IEntityComponentData& entityComponentData) const
{
	return nullptr;
}

	}
}
