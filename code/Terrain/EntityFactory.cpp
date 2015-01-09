#include "Terrain/EntityFactory.h"
#include "Terrain/TerrainEntity.h"
#include "Terrain/TerrainEntityData.h"
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
	typeSet.insert(&type_of< TerrainEntityData >());
	return typeSet;
}

const TypeInfoSet EntityFactory::getEntityEventTypes() const
{
	return TypeInfoSet();
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
	else if (const TerrainEntityData* terrainData = dynamic_type_cast< const TerrainEntityData* >(&entityData))
	{
		Ref< TerrainEntity > terrain = new TerrainEntity(m_resourceManager, m_renderSystem);
		if (terrain->create(*terrainData))
			return terrain;
	}

	return 0;
}

Ref< world::IEntityEvent > EntityFactory::createEntityEvent(const world::IEntityBuilder* builder, const world::IEntityEventData& entityEventData) const
{
	return 0;
}

	}
}
