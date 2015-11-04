#include "Terrain/EntityFactory.h"
#include "Terrain/OceanComponent.h"
#include "Terrain/OceanComponentData.h"
#include "Terrain/RiverComponent.h"
#include "Terrain/RiverComponentData.h"
#include "Terrain/TerrainComponent.h"
#include "Terrain/TerrainComponentData.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.EntityFactory", EntityFactory, world::IEntityFactory)

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
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< OceanComponentData >());
	typeSet.insert(&type_of< RiverComponentData >());
	typeSet.insert(&type_of< TerrainComponentData >());
	return typeSet;
}

Ref< world::Entity > EntityFactory::createEntity(const world::IEntityBuilder* builder, const world::EntityData& entityData) const
{
	return 0;
}

Ref< world::IEntityEvent > EntityFactory::createEntityEvent(const world::IEntityBuilder* builder, const world::IEntityEventData& entityEventData) const
{
	return 0;
}

Ref< world::IEntityComponent > EntityFactory::createEntityComponent(const world::IEntityBuilder* builder, world::Entity* owner, const world::IEntityComponentData& entityComponentData) const
{
	if (const OceanComponentData* oceanComponentData = dynamic_type_cast< const OceanComponentData* >(&entityComponentData))
	{
		Ref< OceanComponent > oceanComponent = new OceanComponent(owner);
		if (oceanComponent->create(m_resourceManager, m_renderSystem, *oceanComponentData))
			return oceanComponent;
	}
	else if (const RiverComponentData* riverComponentData = dynamic_type_cast< const RiverComponentData* >(&entityComponentData))
	{
		Ref< RiverComponent > riverComponent = new RiverComponent();
		if (riverComponent->create(m_resourceManager, m_renderSystem, *riverComponentData))
			return riverComponent;
	}
	else if (const TerrainComponentData* terrainComponentData = dynamic_type_cast< const TerrainComponentData* >(&entityComponentData))
	{
		Ref< TerrainComponent > terrainComponent = new TerrainComponent(m_resourceManager, m_renderSystem);
		if (terrainComponent->create(*terrainComponentData))
			return terrainComponent;
	}
	return 0;
}

	}
}
