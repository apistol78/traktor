#include "Terrain/EntityFactory.h"
#include "Terrain/TerrainComponent.h"
#include "Terrain/TerrainComponentData.h"
#include "Terrain/OceanEntity.h"
#include "Terrain/OceanEntityData.h"
#include "Terrain/RiverEntity.h"
#include "Terrain/RiverEntityData.h"

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
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< OceanEntityData >());
	typeSet.insert(&type_of< RiverEntityData >());
	return typeSet;
}

const TypeInfoSet EntityFactory::getEntityEventTypes() const
{
	return TypeInfoSet();
}

const TypeInfoSet EntityFactory::getEntityComponentTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< TerrainComponentData >());
	return typeSet;
}

Ref< world::Entity > EntityFactory::createEntity(const world::IEntityBuilder* builder, const world::EntityData& entityData) const
{
	if (const OceanEntityData* oceanData = dynamic_type_cast< const OceanEntityData* >(&entityData))
	{
		Ref< OceanEntity > ocean = new OceanEntity();
		if (ocean->create(m_resourceManager, m_renderSystem, *oceanData))
			return ocean;
	}
	else if (const RiverEntityData* riverData = dynamic_type_cast< const RiverEntityData* >(&entityData))
	{
		Ref< RiverEntity > river = new RiverEntity();
		if (river->create(m_resourceManager, m_renderSystem, *riverData))
			return river;
	}
	return 0;
}

Ref< world::IEntityEvent > EntityFactory::createEntityEvent(const world::IEntityBuilder* builder, const world::IEntityEventData& entityEventData) const
{
	return 0;
}

Ref< world::IEntityComponent > EntityFactory::createEntityComponent(const world::IEntityBuilder* builder, world::Entity* owner, const world::IEntityComponentData& entityComponentData) const
{
	if (const TerrainComponentData* terrainComponentData = dynamic_type_cast< const TerrainComponentData* >(&entityComponentData))
	{
		Ref< TerrainComponent > terrainComponent = new TerrainComponent(m_resourceManager, m_renderSystem);
		if (terrainComponent->create(*terrainComponentData))
			return terrainComponent;
	}
	return 0;
}

	}
}
