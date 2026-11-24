/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Terrain/Editor/TerrainEditData.h"

#include "Core/Log/Log.h"
#include "Core/Math/MathUtils.h"
#include "Database/Instance.h"
#include "Heightfield/Editor/HeightfieldAsset.h"
#include "Heightfield/Editor/TileStorage.h"
#include "Heightfield/Heightfield.h"

namespace traktor::terrain
{
namespace
{

//! Copy every channel of sample from source into target.
void copySample(const hf::Heightfield* source, hf::Heightfield* target, int32_t x, int32_t z)
{
	target->setGridHeight(x, z, source->getGridHeightNearestUnsafe(x, z));
	target->setGridCut(x, z, source->getGridCut(x, z));

	for (uint8_t a = 0; a < hf::Heightfield::MaxAttributes; ++a)
		if (source->getAttributes(a) || target->getAttributes(a))
			target->setGridAttribute(x, z, a, source->getGridAttributeDensity(x, z, a));

	if (source->getSplats() || target->getSplats())
		target->setGridSplat(x, z, source->getGridSplat(x, z));

	if (source->getColors() || target->getColors())
		target->setGridColor(x, z, source->getGridColor(x, z));
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.TerrainEditData", TerrainEditData, Object)

bool TerrainEditData::create(const db::Instance* heightfieldInstance, const hf::HeightfieldAsset* heightfieldAsset)
{
	m_heightfieldInstance = heightfieldInstance;
	m_storage = new hf::TileStorage(m_heightfieldInstance, heightfieldAsset);

	m_worldExtent = heightfieldAsset->getWorldExtent();
	m_tileCount = std::max(heightfieldAsset->getTileCount(), 1);

	m_size = m_storage->readSize();
	if (m_size <= 0)
	{
		log::error << L"Unable to edit terrain; failed to read heightfield size." << Endl;
		return false;
	}

	// Top layer is edited.
	m_editLayer = (int32_t)heightfieldAsset->getLayers().size() - 1;
	m_tileSize = m_size / m_tileCount;
	m_tiles.resize(m_tileCount * m_tileCount);
	return true;
}

float TerrainEditData::getGridHeightNearest(int32_t gridX, int32_t gridZ)
{
	gridX = clamp(gridX, 0, m_size - 1);
	gridZ = clamp(gridZ, 0, m_size - 1);
	const Tile* tile = getTile(gridX, gridZ);
	return tile ? tile->merged->getGridHeightNearestUnsafe(gridX, gridZ) : 0.0f;
}

float TerrainEditData::getGridHeightBilinear(float gridX, float gridZ)
{
	const int32_t igridX = clamp((int32_t)gridX, 0, m_size - 2);
	const int32_t igridZ = clamp((int32_t)gridZ, 0, m_size - 2);

	float hts[] = {
		getGridHeightNearest(igridX, igridZ),
		getGridHeightNearest(igridX + 1, igridZ),
		getGridHeightNearest(igridX, igridZ + 1),
		getGridHeightNearest(igridX + 1, igridZ + 1)
	};

	const float fgridX = gridX - igridX;
	const float fgridZ = gridZ - igridZ;

	// Same triangulation as heightfield.
	if (fgridX - fgridZ < 0.0f)
		hts[1] = hts[0] + (hts[3] - hts[2]);
	else
		hts[2] = hts[3] + (hts[0] - hts[1]);

	const float hl = hts[0] + (hts[2] - hts[0]) * fgridZ;
	const float hr = hts[1] + (hts[3] - hts[1]) * fgridZ;
	return hl + (hr - hl) * fgridX;
}

float TerrainEditData::getWorldHeight(float worldX, float worldZ)
{
	float gridX, gridZ;
	worldToGrid(worldX, worldZ, gridX, gridZ);
	return m_worldExtent.y() * (getGridHeightBilinear(gridX, gridZ) - 0.5f);
}

void TerrainEditData::setGridHeight(int32_t gridX, int32_t gridZ, float unitY)
{
	if (gridX < 0 || gridX >= m_size || gridZ < 0 || gridZ >= m_size)
		return;

	Tile* tile = editTile(gridX, gridZ);
	if (!tile)
		return;

	tile->merged->setGridHeight(gridX, gridZ, unitY);
	tile->edit->setGridHeight(gridX, gridZ, unitY);
}

bool TerrainEditData::getGridCut(int32_t gridX, int32_t gridZ)
{
	gridX = clamp(gridX, 0, m_size - 1);
	gridZ = clamp(gridZ, 0, m_size - 1);
	const Tile* tile = getTile(gridX, gridZ);
	return tile ? tile->merged->getGridCut(gridX, gridZ) : true;
}

void TerrainEditData::setGridCut(int32_t gridX, int32_t gridZ, bool cut)
{
	if (gridX < 0 || gridX >= m_size || gridZ < 0 || gridZ >= m_size)
		return;

	Tile* tile = editTile(gridX, gridZ);
	if (!tile)
		return;

	tile->merged->setGridCut(gridX, gridZ, cut);
	tile->edit->setGridCut(gridX, gridZ, cut);
}

uint8_t TerrainEditData::getGridAttributeDensity(int32_t gridX, int32_t gridZ, uint8_t attribute)
{
	gridX = clamp(gridX, 0, m_size - 1);
	gridZ = clamp(gridZ, 0, m_size - 1);
	const Tile* tile = getTile(gridX, gridZ);
	return tile ? tile->merged->getGridAttributeDensity(gridX, gridZ, attribute) : 0;
}

void TerrainEditData::setGridAttribute(int32_t gridX, int32_t gridZ, uint8_t attribute, uint8_t density)
{
	if (gridX < 0 || gridX >= m_size || gridZ < 0 || gridZ >= m_size)
		return;

	Tile* tile = editTile(gridX, gridZ);
	if (!tile)
		return;

	tile->merged->setGridAttribute(gridX, gridZ, attribute, density);
	tile->edit->setGridAttribute(gridX, gridZ, attribute, density);
}

Vector4 TerrainEditData::normalAt(float gridX, float gridZ)
{
	const float c_distance = 0.5f;
	const float directions[][2] = {
		{ -c_distance, -c_distance },
		{ 0.0f, -c_distance },
		{ c_distance, -c_distance },
		{ c_distance, 0.0f },
		{ c_distance, c_distance },
		{ 0.0f, 0.0f },
		{ -c_distance, c_distance },
		{ -c_distance, 0.0f }
	};

	const float h0 = getGridHeightBilinear(gridX, gridZ);

	float h[sizeof_array(directions)];
	for (uint32_t i = 0; i < sizeof_array(directions); ++i)
		h[i] = getGridHeightBilinear(gridX + directions[i][0], gridZ + directions[i][1]);

	const float sx = m_worldExtent.x() / m_size;
	const float sy = m_worldExtent.y();
	const float sz = m_worldExtent.z() / m_size;

	Vector4 N = Vector4::zero();
	for (uint32_t i = 0; i < sizeof_array(directions); ++i)
	{
		const uint32_t j = (i + 1) % sizeof_array(directions);
		const Vector4 d1(directions[i][0] * sx, (h[i] - h0) * sy, directions[i][1] * sz);
		const Vector4 d2(directions[j][0] * sx, (h[j] - h0) * sy, directions[j][1] * sz);
		N += cross(d2, d1);
	}
	return N.normalized();
}

float TerrainEditData::worldToUnit(float worldY) const
{
	const float wexy = m_worldExtent.y();
	return (worldY + wexy * 0.5f) / wexy;
}

void TerrainEditData::worldToGrid(float worldX, float worldZ, float& outGridX, float& outGridZ) const
{
	outGridX = m_size * (worldX + m_worldExtent.x() * 0.5f) / m_worldExtent.x() - 0.5f;
	outGridZ = m_size * (worldZ + m_worldExtent.z() * 0.5f) / m_worldExtent.z() - 0.5f;
}

bool TerrainEditData::getSplat(int32_t x, int32_t z, Color4f& outSplat)
{
	if (x < 0 || x >= m_size || z < 0 || z >= m_size)
		return false;

	const Tile* tile = getTile(x, z);
	if (!tile)
		return false;

	outSplat = tile->merged->getGridSplat(x, z);
	return true;
}

bool TerrainEditData::setSplat(int32_t x, int32_t z, const Color4f& splat)
{
	if (x < 0 || x >= m_size || z < 0 || z >= m_size)
		return false;

	Tile* tile = editTile(x, z);
	if (!tile)
		return false;

	tile->merged->setGridSplat(x, z, splat);
	tile->edit->setGridSplat(x, z, splat);
	return true;
}

bool TerrainEditData::getColor(int32_t x, int32_t z, Color4f& outColor)
{
	if (x < 0 || x >= m_size || z < 0 || z >= m_size)
		return false;

	const Tile* tile = getTile(x, z);
	if (!tile)
		return false;

	outColor = tile->merged->getGridColor(x, z);
	return true;
}

bool TerrainEditData::setColor(int32_t x, int32_t z, const Color4f& color)
{
	if (x < 0 || x >= m_size || z < 0 || z >= m_size)
		return false;

	Tile* tile = editTile(x, z);
	if (!tile)
		return false;

	tile->merged->setGridColor(x, z, color);
	tile->edit->setGridColor(x, z, color);
	return true;
}

bool TerrainEditData::writeHeightfield()
{
	for (int32_t i = 0; i < (int32_t)m_tiles.size(); ++i)
	{
		Tile& tile = m_tiles[i];
		if (!tile.modified)
			continue;

		RefArray< const hf::Heightfield > tiles;
		tiles.push_back(tile.edit);

		const AlignedVector< AlignedVector< uint8_t > > masks(1, tile.mask);
		if (!m_storage->writeLayerTiles(m_editLayer, i % m_tileCount, i / m_tileCount, i % m_tileCount, i / m_tileCount, tiles, &masks))
			return false;

		tile.modified = false;
	}
	return true;
}

TerrainEditData::Tile* TerrainEditData::getTile(int32_t& inoutX, int32_t& inoutZ)
{
	const int32_t tileX = inoutX / m_tileSize;
	const int32_t tileZ = inoutZ / m_tileSize;
	inoutX -= tileX * m_tileSize;
	inoutZ -= tileZ * m_tileSize;

	Tile& tile = m_tiles[tileX + tileZ * m_tileCount];
	if (!tile.merged)
	{
		Ref< hf::Heightfield > edit = m_storage->readLayerTile(m_editLayer, m_worldExtent, tileX, tileZ, &tile.mask);
		Ref< hf::Heightfield > merged = (m_editLayer > 0) ? m_storage->readMergedTile(m_worldExtent, tileX, tileZ) : edit;
		if (!edit || !merged || edit->getSize() != m_tileSize || merged->getSize() != m_tileSize)
		{
			log::error << L"Unable to edit terrain; tile " << tileX << L", " << tileZ << L" missing or size mismatch." << Endl;
			return nullptr;
		}

		tile.merged = merged;
		tile.edit = edit;
	}
	return &tile;
}

TerrainEditData::Tile* TerrainEditData::editTile(int32_t& inoutX, int32_t& inoutZ)
{
	Tile* tile = getTile(inoutX, inoutZ);
	if (!tile)
		return nullptr;

	// Edit layer take over sample entirely, thus it's first seeded with merged sample.
	uint8_t& weight = tile->mask[inoutX + inoutZ * m_tileSize];
	if (weight != 255)
	{
		copySample(tile->merged, tile->edit, inoutX, inoutZ);
		weight = 255;
	}

	tile->modified = true;
	return tile;
}

}
