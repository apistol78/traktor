/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Color4f.h"
#include "Core/Math/Vector4.h"
#include "Core/Object.h"
#include "Core/Ref.h"

#include <string>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TERRAIN_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::db
{

class Instance;

}

namespace traktor::hf
{

class Heightfield;
class HeightfieldAsset;
class TileStorage;

}

namespace traktor::terrain
{

/*! Source heightfield of terrain being edited, each tile is loaded when first accessed.
 * \ingroup Terrain
 *
 * Coordinates are grid coordinates of entire terrain.
 * Heightfield is read with layers up to, and including, edit layer merged;
 * edits are written into edit layer, which take over each edited sample through its mask.
 * Reads with merged set read all layers merged, as displayed.
 */
class T_DLLCLASS TerrainEditData : public Object
{
	T_RTTI_CLASS;

public:
	/*! \param editLayerId Identifier of edit layer which is edited.
	 * \param cachePath Directory where outputs of heightfield layers are cached.
	 */
	bool create(const db::Instance* heightfieldInstance, const hf::HeightfieldAsset* heightfieldAsset, int32_t editLayerId, const std::wstring& cachePath);

	int32_t getSize() const { return m_size; }

	int32_t getTileCount() const { return m_tileCount; }

	int32_t getTileSize() const { return m_tileSize; }

	const Vector4& getWorldExtent() const { return m_worldExtent; }

	float getGridHeightNearest(int32_t gridX, int32_t gridZ, bool merged = false);

	float getGridHeightBilinear(float gridX, float gridZ, bool merged = false);

	float getWorldHeight(float worldX, float worldZ, bool merged = false);

	void setGridHeight(int32_t gridX, int32_t gridZ, float unitY);

	bool getGridCut(int32_t gridX, int32_t gridZ, bool merged = false);

	void setGridCut(int32_t gridX, int32_t gridZ, bool cut);

	uint8_t getGridAttributeDensity(int32_t gridX, int32_t gridZ, uint8_t attribute, bool merged = false);

	void setGridAttribute(int32_t gridX, int32_t gridZ, uint8_t attribute, uint8_t density);

	Vector4 normalAt(float gridX, float gridZ, bool merged = false);

	float worldToUnit(float worldY) const;

	void worldToGrid(float worldX, float worldZ, float& outGridX, float& outGridZ) const;

	bool getSplat(int32_t x, int32_t z, Color4f& outSplat, bool merged = false);

	bool setSplat(int32_t x, int32_t z, const Color4f& splat);

	bool getColor(int32_t x, int32_t z, Color4f& outColor, bool merged = false);

	bool setColor(int32_t x, int32_t z, const Color4f& color);

	//! Write modified tiles, and masks, of edit layer into heightfield instance; layers above edit layer are evaluated again onto written tiles.
	bool writeHeightfield();

private:
	struct Tile
	{
		Ref< hf::Heightfield > merged;	//!< All layers merged; same as level if edit layer is top.
		Ref< hf::Heightfield > level;	//!< Layers up to, and including, edit layer merged; same as edit if edit layer is base.
		Ref< hf::Heightfield > edit;	//!< Tile of edit layer.
		AlignedVector< uint8_t > mask;	//!< Mask of edit layer.
		bool modified = false;
	};

	Ref< const db::Instance > m_heightfieldInstance;
	Ref< hf::TileStorage > m_storage;
	Vector4 m_worldExtent = Vector4::zero();
	int32_t m_size = 0;
	int32_t m_tileCount = 0;
	int32_t m_tileSize = 0;
	int32_t m_editLayer = 0;
	int32_t m_layerCount = 0;
	AlignedVector< Tile > m_tiles;

	//! Tile of grid coordinate; coordinate is converted into tile's coordinate.
	Tile* getTile(int32_t& inoutX, int32_t& inoutZ);

	//! Tile of grid coordinate prepared for edit of sample; coordinate is converted into tile's coordinate.
	Tile* editTile(int32_t& inoutX, int32_t& inoutZ);
};

}
