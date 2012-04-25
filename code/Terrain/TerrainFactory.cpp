#include "Database/Database.h"
#include "Heightfield/Heightfield.h"
#include "Render/ISimpleTexture.h"
#include "Render/Shader.h"
#include "Resource/IResourceManager.h"
#include "Terrain/Terrain.h"
#include "Terrain/TerrainResource.h"
#include "Terrain/TerrainFactory.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.TerrainFactory", TerrainFactory, resource::IResourceFactory)

TerrainFactory::TerrainFactory(db::Database* db)
:	m_db(db)
{
}

const TypeInfoSet TerrainFactory::getResourceTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Terrain >());
	return typeSet;
}

bool TerrainFactory::isCacheable() const
{
	return true;
}

Ref< Object > TerrainFactory::create(resource::IResourceManager* resourceManager, const TypeInfo& resourceType, const Guid& guid)
{
	Ref< const TerrainResource > terrainResource = m_db->getObjectReadOnly< TerrainResource >(guid);

	Ref< Terrain > terrain = new Terrain();

	if (!resourceManager->bind(terrainResource->getHeightfield(), terrain->m_heightfield))
		return 0;
	if (!resourceManager->bind(terrainResource->getNormalMap(), terrain->m_normalMap))
		return 0;
	if (!resourceManager->bind(terrainResource->getHeightMap(), terrain->m_heightMap))
		return 0;

	const std::vector< resource::Id< render::Shader > >& surfaceLayers = terrainResource->getSurfaceLayers();
	
	terrain->m_surfaceLayers.resize(surfaceLayers.size());
	for (size_t i = 0; i < surfaceLayers.size(); ++i)
	{
		if (!resourceManager->bind(surfaceLayers[i], terrain->m_surfaceLayers[i]))
			return 0;
	}

	return terrain;
}

	}
}
