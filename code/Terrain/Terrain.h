/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Resource/Proxy.h"
#include "Terrain/TerrainResource.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TERRAIN_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::hf
{

class Heightfield;

}

namespace traktor::render
{

class ITexture;
class Shader;

}

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::terrain
{

/*! Terrain runtime description.
 * \ingroup Terrain
 */
class T_DLLCLASS Terrain : public Object
{
	T_RTTI_CLASS;

public:
	//! Resources of a tile, or overview, of terrain.
	struct TileResources
	{
		resource::Proxy< hf::Heightfield > heightfield;
		resource::Proxy< render::ITexture > heightMap;
		resource::Proxy< render::ITexture > normalMap;
		resource::Proxy< render::ITexture > cutMap;
		resource::Proxy< render::ITexture > splatMap;
		resource::Proxy< render::ITexture > colorMap;
	};

	//! Amount of detail to skip.
	uint32_t getDetailSkip() const { return m_detailSkip; }

	//! Size of a single patch.
	uint32_t getPatchDim() const { return m_patchDim; }

	//! Terrain shader resource.
	const resource::Proxy< render::Shader >& getTerrainShader() const { return m_terrainShader; }

	//! Terrain surface shader resource.
	const resource::Proxy< render::Shader >& getSurfaceShader() const { return m_surfaceShader; }

	//! Number of tiles along each side.
	int32_t getTileCount() const { return m_tileCount; }

	//! Number of heightfield samples along each side of a tile, excluding seam.
	int32_t getTileSize() const { return m_tileSize; }

	//! Bind resources of tile, row major index; blocks until loaded.
	bool bindTile(resource::IResourceManager* resourceManager, int32_t tile, TileResources& outResources) const;

	//! Resources of low resolution overview of entire terrain; always loaded.
	const TileResources& getOverview() const { return m_overview; }

private:
	friend class TerrainFactory;

	uint32_t m_detailSkip = 0;
	uint32_t m_patchDim = 0;
	resource::Proxy< render::Shader > m_terrainShader;
	resource::Proxy< render::Shader > m_surfaceShader;
	int32_t m_tileCount = 0;
	int32_t m_tileSize = 0;
	AlignedVector< TerrainResource::Tile > m_tiles;
	TileResources m_overview;

	static bool bindResources(resource::IResourceManager* resourceManager, const TerrainResource::Tile& tile, TileResources& outResources);
};

}
