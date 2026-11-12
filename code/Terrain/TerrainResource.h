/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Serialization/ISerializable.h"
#include "Resource/Id.h"

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

namespace traktor::terrain
{

/*! Terrain persistent resource data.
 * \ingroup Terrain
 */
class T_DLLCLASS TerrainResource : public ISerializable
{
	T_RTTI_CLASS;

public:
	//! Resources of a tile, or overview, of a tiled terrain.
	struct Tile
	{
		resource::Id< hf::Heightfield > heightfield;
		resource::Id< render::ITexture > heightMap;
		resource::Id< render::ITexture > normalMap;
		resource::Id< render::ITexture > cutMap;
		resource::Id< render::ITexture > splatMap;
		resource::Id< render::ITexture > colorMap;

		void serialize(ISerializer& s);
	};

	virtual void serialize(ISerializer& s) override final;

	uint32_t getDetailSkip() const { return m_detailSkip; }

	uint32_t getPatchDim() const { return m_patchDim; }

	const resource::Id< render::Shader >& getTerrainShader() const { return m_terrainShader; }

	const resource::Id< render::Shader >& getSurfaceShader() const { return m_surfaceShader; }

	//! Number of tiles along each side.
	int32_t getTileCount() const { return m_tileCount; }

	//! Number of heightfield samples along each side of a tile, excluding seam.
	int32_t getTileSize() const { return m_tileSize; }

	//! Tiles, row major, each including one row and column of its +X/+Z neighbours.
	const AlignedVector< Tile >& getTiles() const { return m_tiles; }

	//! Low resolution overview of entire terrain.
	const Tile& getOverview() const { return m_overview; }

private:
	friend class TerrainPipeline;

	uint32_t m_detailSkip = 0;
	uint32_t m_patchDim = 0;
	resource::Id< render::Shader > m_terrainShader;
	resource::Id< render::Shader > m_surfaceShader;
	int32_t m_tileCount = 0;
	int32_t m_tileSize = 0;
	AlignedVector< Tile > m_tiles;
	Tile m_overview;
};

}
