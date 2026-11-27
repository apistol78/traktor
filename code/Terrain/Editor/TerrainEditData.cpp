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
#include "Heightfield/Editor/EditLayer.h"
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

//! Height stored in edit layer for height of sample; delta edit layer store height relative to layers below.
float storedHeight(const hf::Heightfield* below, int32_t x, int32_t z, float unitY)
{
	return below ? unitY - below->getGridHeightNearestUnsafe(x, z) + hf::EditLayer::ZeroDelta / 65535.0f : unitY;
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.TerrainEditData", TerrainEditData, Object)

bool TerrainEditData::create(const db::Instance* heightfieldInstance, const hf::HeightfieldAsset* heightfieldAsset, int32_t editLayerId, const std::wstring& cachePath)
{
	m_heightfieldInstance = heightfieldInstance;
	m_storage = new hf::TileStorage(m_heightfieldInstance, heightfieldAsset, cachePath);

	m_worldExtent = heightfieldAsset->getWorldExtent();
	m_tileCount = std::max(heightfieldAsset->getTileCount(), 1);

	m_size = m_storage->readSize();
	if (m_size <= 0)
	{
		log::error << L"Unable to edit terrain; failed to read heightfield size." << Endl;
		return false;
	}

	const auto& layers = heightfieldAsset->getLayers();
	m_layerCount = (int32_t)layers.size();
	m_editLayer = -1;
	for (int32_t i = 0; i < m_layerCount; ++i)
	{
		const hf::EditLayer* editLayer = dynamic_type_cast< const hf::EditLayer* >(layers[i]);
		if (layers[i]->getId() == editLayerId && editLayer)
		{
			m_editLayer = i;
			m_delta = editLayer->isDelta() && i > 0;
		}
	}
	if (m_editLayer < 0)
	{
		log::error << L"Unable to edit terrain; no edit layer " << editLayerId << L" in heightfield." << Endl;
		return false;
	}

	m_tileSize = m_size / m_tileCount;
	m_tiles.resize(m_tileCount * m_tileCount);
	return true;
}

float TerrainEditData::getGridHeightNearest(int32_t gridX, int32_t gridZ, bool merged)
{
	gridX = clamp(gridX, 0, m_size - 1);
	gridZ = clamp(gridZ, 0, m_size - 1);
	const Tile* tile = getTile(gridX, gridZ);
	return tile ? (merged ? tile->merged : tile->level)->getGridHeightNearestUnsafe(gridX, gridZ) : 0.0f;
}

float TerrainEditData::getGridHeightBilinear(float gridX, float gridZ, bool merged)
{
	const int32_t igridX = clamp((int32_t)gridX, 0, m_size - 2);
	const int32_t igridZ = clamp((int32_t)gridZ, 0, m_size - 2);

	float hts[] = {
		getGridHeightNearest(igridX, igridZ, merged),
		getGridHeightNearest(igridX + 1, igridZ, merged),
		getGridHeightNearest(igridX, igridZ + 1, merged),
		getGridHeightNearest(igridX + 1, igridZ + 1, merged)
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

float TerrainEditData::getWorldHeight(float worldX, float worldZ, bool merged)
{
	float gridX, gridZ;
	worldToGrid(worldX, worldZ, gridX, gridZ);
	return m_worldExtent.y() * (getGridHeightBilinear(gridX, gridZ, merged) - 0.5f);
}

void TerrainEditData::setGridHeight(int32_t gridX, int32_t gridZ, float unitY)
{
	if (gridX < 0 || gridX >= m_size || gridZ < 0 || gridZ >= m_size)
		return;

	Tile* tile = editTile(gridX, gridZ);
	if (!tile)
		return;

	// Merged follow change of level until layers above edit layer are evaluated again.
	if (tile->merged != tile->level)
		tile->merged->setGridHeight(gridX, gridZ, tile->merged->getGridHeightNearestUnsafe(gridX, gridZ) + unitY - tile->level->getGridHeightNearestUnsafe(gridX, gridZ));

	tile->level->setGridHeight(gridX, gridZ, unitY);
	tile->edit->setGridHeight(gridX, gridZ, storedHeight(tile->below, gridX, gridZ, unitY));
}

bool TerrainEditData::getGridCut(int32_t gridX, int32_t gridZ, bool merged)
{
	gridX = clamp(gridX, 0, m_size - 1);
	gridZ = clamp(gridZ, 0, m_size - 1);
	const Tile* tile = getTile(gridX, gridZ);
	return tile ? (merged ? tile->merged : tile->level)->getGridCut(gridX, gridZ) : true;
}

void TerrainEditData::setGridCut(int32_t gridX, int32_t gridZ, bool cut)
{
	if (gridX < 0 || gridX >= m_size || gridZ < 0 || gridZ >= m_size)
		return;

	Tile* tile = editTile(gridX, gridZ);
	if (!tile)
		return;

	tile->merged->setGridCut(gridX, gridZ, cut);
	tile->level->setGridCut(gridX, gridZ, cut);
	tile->edit->setGridCut(gridX, gridZ, cut);
}

uint8_t TerrainEditData::getGridAttributeDensity(int32_t gridX, int32_t gridZ, uint8_t attribute, bool merged)
{
	gridX = clamp(gridX, 0, m_size - 1);
	gridZ = clamp(gridZ, 0, m_size - 1);
	const Tile* tile = getTile(gridX, gridZ);
	return tile ? (merged ? tile->merged : tile->level)->getGridAttributeDensity(gridX, gridZ, attribute) : 0;
}

void TerrainEditData::setGridAttribute(int32_t gridX, int32_t gridZ, uint8_t attribute, uint8_t density)
{
	if (gridX < 0 || gridX >= m_size || gridZ < 0 || gridZ >= m_size)
		return;

	Tile* tile = editTile(gridX, gridZ);
	if (!tile)
		return;

	tile->merged->setGridAttribute(gridX, gridZ, attribute, density);
	tile->level->setGridAttribute(gridX, gridZ, attribute, density);
	tile->edit->setGridAttribute(gridX, gridZ, attribute, density);
}

Vector4 TerrainEditData::normalAt(float gridX, float gridZ, bool merged)
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

	const float h0 = getGridHeightBilinear(gridX, gridZ, merged);

	float h[sizeof_array(directions)];
	for (uint32_t i = 0; i < sizeof_array(directions); ++i)
		h[i] = getGridHeightBilinear(gridX + directions[i][0], gridZ + directions[i][1], merged);

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

bool TerrainEditData::getSplat(int32_t x, int32_t z, Color4f& outSplat, bool merged)
{
	if (x < 0 || x >= m_size || z < 0 || z >= m_size)
		return false;

	const Tile* tile = getTile(x, z);
	if (!tile)
		return false;

	outSplat = (merged ? tile->merged : tile->level)->getGridSplat(x, z);
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
	tile->level->setGridSplat(x, z, splat);
	tile->edit->setGridSplat(x, z, splat);
	return true;
}

bool TerrainEditData::getColor(int32_t x, int32_t z, Color4f& outColor, bool merged)
{
	if (x < 0 || x >= m_size || z < 0 || z >= m_size)
		return false;

	const Tile* tile = getTile(x, z);
	if (!tile)
		return false;

	outColor = (merged ? tile->merged : tile->level)->getGridColor(x, z);
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
	tile->level->setGridColor(x, z, color);
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

		const AlignedVector< AlignedVector< uint16_t > > masks(1, tile.mask);
		if (!m_storage->writeLayerTiles(m_editLayer, i % m_tileCount, i / m_tileCount, i % m_tileCount, i / m_tileCount, tiles, &masks))
			return false;

		tile.modified = false;

		if (tile.merged != tile.level)
		{
			Ref< hf::Heightfield > merged = m_storage->readMergedTile(m_worldExtent, i % m_tileCount, i / m_tileCount);
			if (!merged)
				return false;
			tile.merged = merged;
		}
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
		Ref< hf::Heightfield > level = (m_editLayer > 0) ? m_storage->readMergedTile(m_worldExtent, tileX, tileZ, m_editLayer + 1) : edit;
		Ref< hf::Heightfield > merged = (m_editLayer < m_layerCount - 1) ? m_storage->readMergedTile(m_worldExtent, tileX, tileZ) : level;
		Ref< hf::Heightfield > below = m_delta ? m_storage->readMergedTile(m_worldExtent, tileX, tileZ, m_editLayer) : nullptr;
		if (!edit || !level || !merged || edit->getSize() != m_tileSize || level->getSize() != m_tileSize || merged->getSize() != m_tileSize || (m_delta && (!below || below->getSize() != m_tileSize)))
		{
			log::error << L"Unable to edit terrain; tile " << tileX << L", " << tileZ << L" missing or size mismatch." << Endl;
			return nullptr;
		}

		tile.merged = merged;
		tile.level = level;
		tile.edit = edit;
		tile.below = below;
	}
	return &tile;
}

TerrainEditData::Tile* TerrainEditData::editTile(int32_t& inoutX, int32_t& inoutZ)
{
	Tile* tile = getTile(inoutX, inoutZ);
	if (!tile)
		return nullptr;

	// Edit layer take over sample entirely, thus it's first seeded with level sample.
	uint16_t& weight = tile->mask[inoutX + inoutZ * m_tileSize];
	if (weight != 65535)
	{
		copySample(tile->level, tile->edit, inoutX, inoutZ);
		tile->edit->setGridHeight(inoutX, inoutZ, storedHeight(tile->below, inoutX, inoutZ, tile->level->getGridHeightNearestUnsafe(inoutX, inoutZ)));
		weight = 65535;
	}

	tile->modified = true;
	return tile;
}

}
