#include "Shape/Editor/EntityFactory.h"
#include "Shape/Editor/Spline/ControlPointComponent.h"
#include "Shape/Editor/Spline/ControlPointComponentData.h"
#include "Shape/Editor/Spline/SplineComponent.h"
#include "Shape/Editor/Spline/SplineComponentData.h"
#include "Shape/Editor/Spline/SplineLayerComponent.h"
#include "Shape/Editor/Spline/SplineLayerComponentData.h"

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
	return TypeInfoSet();
}

const TypeInfoSet EntityFactory::getEntityEventTypes() const
{
	return TypeInfoSet();
}

const TypeInfoSet EntityFactory::getEntityComponentTypes() const
{
	return makeTypeInfoSet<
		ControlPointComponentData,
		SplineComponentData,
		SplineLayerComponentData
	>();
}

Ref< world::Entity > EntityFactory::createEntity(const world::IEntityBuilder* builder, const world::EntityData& entityData) const
{
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
	else if (auto splineData = dynamic_type_cast< const SplineComponentData* >(&entityComponentData))
		return splineData->createComponent();
	else if (auto splineLayerData = dynamic_type_cast< const SplineLayerComponentData* >(&entityComponentData))
		return splineLayerData->createComponent(builder, m_resourceManager, m_renderSystem);
	else
		return nullptr;
}

	}
}
