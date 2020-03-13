#include "Shape/EntityFactory.h"
#include "Shape/Spline/ControlPointComponent.h"
#include "Shape/Spline/ControlPointComponentData.h"
#include "Shape/Spline/LayerComponent.h"
#include "Shape/Spline/LayerComponentData.h"
#include "Shape/Spline/SplineComponent.h"
#include "Shape/Spline/SplineComponentData.h"

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
		LayerComponentData,
		SplineComponentData
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
	else if (auto layerData = dynamic_type_cast< const LayerComponentData* >(&entityComponentData))
		return layerData->createComponent(builder, m_resourceManager, m_renderSystem);
	else if (auto splineData = dynamic_type_cast< const SplineComponentData* >(&entityComponentData))
		return splineData->createComponent();
	else
		return nullptr;
}

	}
}
