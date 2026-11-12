/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Color4f.h"
#include "Core/Math/Vector4.h"

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

namespace traktor::drawing
{

class Image;

}

namespace traktor::hf
{

class Heightfield;
class HeightfieldAsset;
class TileStorage;

}

namespace traktor::terrain
{

/*! Source data of terrain being edited, each tile is loaded when first accessed.
 * \ingroup Terrain
 *
 * Coordinates are grid coordinates of entire terrain; splat and color are kept in heightfield's resolution.
 */
class T_DLLCLASS TerrainEditData : public Object
{
	T_RTTI_CLASS;

public:
	bool create(const db::Instance* heightfieldInstance, const db::Instance* terrainInstance, const hf::HeightfieldAsset* heightfieldAsset);

	int32_t getSize() const { return m_size; }

	int32_t getTileCount() const { return m_tileCount; }

	int32_t getTileSize() const { return m_tileSize; }

	const Vector4& getWorldExtent() const { return m_worldExtent; }

	float getGridHeightNearest(int32_t gridX, int32_t gridZ);

	float getGridHeightBilinear(float gridX, float gridZ);

	float getWorldHeight(float worldX, float worldZ);

	void setGridHeight(int32_t gridX, int32_t gridZ, float unitY);

	bool getGridCut(int32_t gridX, int32_t gridZ);

	void setGridCut(int32_t gridX, int32_t gridZ, bool cut);

	uint8_t getGridAttributeDensity(int32_t gridX, int32_t gridZ, uint8_t attribute);

	void setGridAttribute(int32_t gridX, int32_t gridZ, uint8_t attribute, uint8_t density);

	Vector4 normalAt(float gridX, float gridZ);

	float worldToUnit(float worldY) const;

	void worldToGrid(float worldX, float worldZ, float& outGridX, float& outGridZ) const;

	bool getSplat(int32_t x, int32_t z, Color4f& outSplat);

	bool setSplat(int32_t x, int32_t z, const Color4f& splat);

	bool getColor(int32_t x, int32_t z, Color4f& outColor);

	bool setColor(int32_t x, int32_t z, const Color4f& color);

	//! Write modified heightfield tiles into instance.
	bool writeHeightfield(db::Instance* instance);

	//! Write modified splat and color tiles into instance.
	bool writeImages(db::Instance* instance);

private:
	enum
	{
		Splat = 0,
		Color = 1
	};

	struct Tile
	{
		Ref< hf::Heightfield > heightfield;
		Ref< drawing::Image > images[2];
		bool heightfieldModified = false;
		bool imageModified[2] = { false, false };
	};

	Ref< const db::Instance > m_heightfieldInstance;
	Ref< const db::Instance > m_terrainInstance;
	Ref< hf::TileStorage > m_storage;
	Vector4 m_worldExtent = Vector4::zero();
	int32_t m_size = 0;
	int32_t m_tileCount = 0;
	int32_t m_tileSize = 0;
	bool m_writeAllImage[2] = { false, false };	//!< Stored image is missing or of another size, thus every tile need to be written.
	AlignedVector< Tile > m_tiles;

	//! Heightfield tile of grid coordinate; coordinate is converted into tile's coordinate.
	hf::Heightfield* getHeightfieldTile(int32_t& inoutX, int32_t& inoutZ);

	//! Image tile of pixel coordinate; coordinate is converted into tile's coordinate.
	drawing::Image* getImageTile(int32_t image, int32_t& inoutX, int32_t& inoutZ);
};

}
