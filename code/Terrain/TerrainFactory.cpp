/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Database/Instance.h"
#include "Heightfield/Heightfield.h"
#include "Render/ITexture.h"
#include "Render/Shader.h"
#include "Resource/IResourceManager.h"
#include "Terrain/Terrain.h"
#include "Terrain/TerrainResource.h"
#include "Terrain/TerrainFactory.h"

namespace traktor::terrain
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.terrain.TerrainFactory", 0, TerrainFactory, resource::IResourceFactory)

bool TerrainFactory::initialize(const ObjectStore& objectStore)
{
	return true;
}

const TypeInfoSet TerrainFactory::getResourceTypes() const
{
	return makeTypeInfoSet< TerrainResource >();
}

const TypeInfoSet TerrainFactory::getProductTypes(const TypeInfo& resourceType) const
{
	return makeTypeInfoSet< Terrain >();
}

bool TerrainFactory::isCacheable(const TypeInfo& productType) const
{
	return true;
}

Ref< Object > TerrainFactory::create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const
{
	Ref< const TerrainResource > terrainResource = instance->getObject< TerrainResource >();
	if (!terrainResource)
		return nullptr;

	Ref< Terrain > terrain = new Terrain();
	terrain->m_detailSkip = terrainResource->getDetailSkip();
	terrain->m_patchDim = terrainResource->getPatchDim();

	if (!resourceManager->bind(terrainResource->getHeightfield(), terrain->m_heightfield))
		return nullptr;

	if (terrainResource->getColorMap())
	{
		if (!resourceManager->bind(terrainResource->getColorMap(), terrain->m_colorMap))
			return nullptr;
	}

	if (!resourceManager->bind(terrainResource->getNormalMap(), terrain->m_normalMap))
		return nullptr;
	if (!resourceManager->bind(terrainResource->getHeightMap(), terrain->m_heightMap))
		return nullptr;
	if (!resourceManager->bind(terrainResource->getSplatMap(), terrain->m_splatMap))
		return nullptr;

	if (terrainResource->getCutMap())
	{
		if (!resourceManager->bind(terrainResource->getCutMap(), terrain->m_cutMap))
			return nullptr;
	}

	if (!resourceManager->bind(terrainResource->getTerrainShader(), terrain->m_terrainShader))
		return nullptr;
	if (!resourceManager->bind(terrainResource->getSurfaceShader(), terrain->m_surfaceShader))
		return nullptr;

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
