#include "Database/Database.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/MaterialMask.h"
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
	typeSet.insert(&type_of< TerrainResource >());
	return typeSet;
}

const TypeInfoSet TerrainFactory::getProductTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Terrain >());
	return typeSet;
}

bool TerrainFactory::isCacheable() const
{
	return true;
}

Ref< Object > TerrainFactory::create(resource::IResourceManager* resourceManager, const TypeInfo& resourceType, const Guid& guid, const Object* current) const
{
	Ref< const TerrainResource > terrainResource = m_db->getObjectReadOnly< TerrainResource >(guid);
	if (!terrainResource)
		return 0;

	Ref< Terrain > terrain = new Terrain();

	terrain->m_detailSkip = terrainResource->getDetailSkip();
	terrain->m_patchDim = terrainResource->getPatchDim();

	if (!resourceManager->bind(terrainResource->getHeightfield(), terrain->m_heightfield))
		return 0;

	if (terrainResource->getMaterialMask())
	{
		if (!resourceManager->bind(terrainResource->getMaterialMask(), terrain->m_materialMask))
			return 0;
	}

	if (terrainResource->getColorMap())
	{
		if (!resourceManager->bind(terrainResource->getColorMap(), terrain->m_colorMap))
			return 0;
	}

	if (!resourceManager->bind(terrainResource->getNormalMap(), terrain->m_normalMap))
		return 0;
	if (!resourceManager->bind(terrainResource->getHeightMap(), terrain->m_heightMap))
		return 0;
	if (!resourceManager->bind(terrainResource->getSplatMap(), terrain->m_splatMap))
		return 0;

	if (terrainResource->getCutMap())
	{
		if (!resourceManager->bind(terrainResource->getCutMap(), terrain->m_cutMap))
			return 0;
	}

	if (!resourceManager->bind(terrainResource->getTerrainCoarseShader(), terrain->m_terrainCoarseShader))
		return 0;
	if (!resourceManager->bind(terrainResource->getTerrainDetailShader(), terrain->m_terrainDetailShader))
		return 0;
	if (!resourceManager->bind(terrainResource->getSurfaceShader(), terrain->m_surfaceShader))
		return 0;

	terrain->m_patches.resize(terrainResource->getPatches().size());
	for (uint32_t i = 0; i < terrainResource->getPatches().size(); ++i)
	{
		terrain->m_patches[i].height[0] = terrainResource->getPatches()[i].height[0];
		terrain->m_patches[i].height[1] = terrainResource->getPatches()[i].height[1];

		terrain->m_patches[i].error[0] = terrainResource->getPatches()[i].error[0];
		terrain->m_patches[i].error[1] = terrainResource->getPatches()[i].error[1];
		terrain->m_patches[i].error[2] = terrainResource->getPatches()[i].error[2];
	}

	return terrain;
}

	}
}
