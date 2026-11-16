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
#include "Drawing/Filters/ScaleFilter.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Heightfield/Editor/HeightfieldAsset.h"
#include "Heightfield/Editor/TileStorage.h"
#include "Heightfield/Heightfield.h"

namespace traktor::terrain
{
namespace
{

const wchar_t* c_imageNames[] = { L"Splat", L"Color" };

const Color4f c_imageDefaults[] = { Color4f(1.0f, 0.0f, 0.0f, 0.0f), Color4f(0.5f, 0.5f, 0.5f, 0.0f) };

//! Pixel format of image in memory; color is kept in high precision since brush blend in small increments.
const drawing::PixelFormat& imageFormat(int32_t image)
{
	static const drawing::PixelFormat c_splatFormat = drawing::PixelFormat::getR8G8B8A8().endianSwapped();
	return (image == 0) ? c_splatFormat : drawing::PixelFormat::getABGRF32();
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.TerrainEditData", TerrainEditData, Object)

bool TerrainEditData::create(const db::Instance* heightfieldInstance, const db::Instance* terrainInstance, const hf::HeightfieldAsset* heightfieldAsset)
{
	m_heightfieldInstance = heightfieldInstance;
	m_terrainInstance = terrainInstance;

	m_heightfieldStorage = new hf::TileStorage(m_heightfieldInstance, heightfieldAsset->getTileCount());
	m_terrainStorage = new hf::TileStorage(m_terrainInstance, heightfieldAsset->getTileCount());

	m_worldExtent = heightfieldAsset->getWorldExtent();
	m_tileCount = std::max(heightfieldAsset->getTileCount(), 1);

	m_size = m_heightfieldStorage->readSize();
	if (m_size <= 0)
	{
		log::error << L"Unable to edit terrain; failed to read heightfield size." << Endl;
		return false;
	}

	m_tileSize = m_size / m_tileCount;
	m_tiles.resize(m_tileCount * m_tileCount);
	return true;
}

float TerrainEditData::getGridHeightNearest(int32_t gridX, int32_t gridZ)
{
	gridX = clamp(gridX, 0, m_size - 1);
	gridZ = clamp(gridZ, 0, m_size - 1);
	hf::Heightfield* tile = getHeightfieldTile(gridX, gridZ);
	return tile ? tile->getGridHeightNearestUnsafe(gridX, gridZ) : 0.0f;
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

	const int32_t tileIndex = (gridX / m_tileSize) + (gridZ / m_tileSize) * m_tileCount;
	hf::Heightfield* tile = getHeightfieldTile(gridX, gridZ);
	if (!tile)
		return;

	tile->setGridHeight(gridX, gridZ, unitY);
	m_tiles[tileIndex].heightfieldModified = true;
}

bool TerrainEditData::getGridCut(int32_t gridX, int32_t gridZ)
{
	gridX = clamp(gridX, 0, m_size - 1);
	gridZ = clamp(gridZ, 0, m_size - 1);
	hf::Heightfield* tile = getHeightfieldTile(gridX, gridZ);
	return tile ? tile->getGridCut(gridX, gridZ) : true;
}

void TerrainEditData::setGridCut(int32_t gridX, int32_t gridZ, bool cut)
{
	if (gridX < 0 || gridX >= m_size || gridZ < 0 || gridZ >= m_size)
		return;

	const int32_t tileIndex = (gridX / m_tileSize) + (gridZ / m_tileSize) * m_tileCount;
	hf::Heightfield* tile = getHeightfieldTile(gridX, gridZ);
	if (!tile)
		return;

	tile->setGridCut(gridX, gridZ, cut);
	m_tiles[tileIndex].heightfieldModified = true;
}

uint8_t TerrainEditData::getGridAttributeDensity(int32_t gridX, int32_t gridZ, uint8_t attribute)
{
	gridX = clamp(gridX, 0, m_size - 1);
	gridZ = clamp(gridZ, 0, m_size - 1);
	hf::Heightfield* tile = getHeightfieldTile(gridX, gridZ);
	return tile ? tile->getGridAttributeDensity(gridX, gridZ, attribute) : 0;
}

void TerrainEditData::setGridAttribute(int32_t gridX, int32_t gridZ, uint8_t attribute, uint8_t density)
{
	if (gridX < 0 || gridX >= m_size || gridZ < 0 || gridZ >= m_size)
		return;

	const int32_t tileIndex = (gridX / m_tileSize) + (gridZ / m_tileSize) * m_tileCount;
	hf::Heightfield* tile = getHeightfieldTile(gridX, gridZ);
	if (!tile)
		return;

	tile->setGridAttribute(gridX, gridZ, attribute, density);
	m_tiles[tileIndex].heightfieldModified = true;
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
	drawing::Image* tile = getImageTile(Splat, x, z);
	return tile ? tile->getPixel(x, z, outSplat) : false;
}

bool TerrainEditData::setSplat(int32_t x, int32_t z, const Color4f& splat)
{
	if (x < 0 || x >= m_size || z < 0 || z >= m_size)
		return false;

	const int32_t tileIndex = (x / m_tileSize) + (z / m_tileSize) * m_tileCount;
	drawing::Image* tile = getImageTile(Splat, x, z);
	if (!tile || !tile->setPixel(x, z, splat))
		return false;

	m_tiles[tileIndex].imageModified[Splat] = true;
	return true;
}

bool TerrainEditData::getColor(int32_t x, int32_t z, Color4f& outColor)
{
	if (x < 0 || x >= m_size || z < 0 || z >= m_size)
		return false;
	drawing::Image* tile = getImageTile(Color, x, z);
	return tile ? tile->getPixel(x, z, outColor) : false;
}

bool TerrainEditData::setColor(int32_t x, int32_t z, const Color4f& color)
{
	if (x < 0 || x >= m_size || z < 0 || z >= m_size)
		return false;

	const int32_t tileIndex = (x / m_tileSize) + (z / m_tileSize) * m_tileCount;
	drawing::Image* tile = getImageTile(Color, x, z);
	if (!tile || !tile->setPixel(x, z, color))
		return false;

	m_tiles[tileIndex].imageModified[Color] = true;
	return true;
}

bool TerrainEditData::writeHeightfield()
{
	for (int32_t i = 0; i < (int32_t)m_tiles.size(); ++i)
	{
		Tile& tile = m_tiles[i];
		if (!tile.heightfieldModified)
			continue;

		if (!m_heightfieldStorage->writeStoredTile(i % m_tileCount, i / m_tileCount, tile.heightfield))
			return false;

		tile.heightfieldModified = false;
	}
	return true;
}

bool TerrainEditData::writeImages()
{
	for (int32_t image = 0; image < 2; ++image)
	{
		bool modified = false;
		for (const auto& tile : m_tiles)
			modified |= tile.imageModified[image];
		if (!modified)
			continue;

		for (int32_t i = 0; i < (int32_t)m_tiles.size(); ++i)
		{
			Tile& tile = m_tiles[i];
			if (!m_writeAllImage[image] && !tile.imageModified[image])
				continue;

			int32_t x = (i % m_tileCount) * m_tileSize;
			int32_t z = (i / m_tileCount) * m_tileSize;
			const drawing::Image* imageTile = getImageTile(image, x, z);
			if (!imageTile)
				return false;

			// Color is stored in low precision.
			Ref< drawing::Image > stored = imageTile->clone();
			if (image == Color)
				stored->convert(drawing::PixelFormat::getA8B8G8R8());

			if (!m_terrainStorage->writeStoredImageTile(c_imageNames[image], i % m_tileCount, i / m_tileCount, stored))
				return false;

			tile.imageModified[image] = false;
		}

		m_writeAllImage[image] = false;
	}
	return true;
}

hf::Heightfield* TerrainEditData::getHeightfieldTile(int32_t& inoutX, int32_t& inoutZ)
{
	const int32_t tileX = inoutX / m_tileSize;
	const int32_t tileZ = inoutZ / m_tileSize;
	inoutX -= tileX * m_tileSize;
	inoutZ -= tileZ * m_tileSize;

	Tile& tile = m_tiles[tileX + tileZ * m_tileCount];
	if (!tile.heightfield)
	{
		tile.heightfield = m_heightfieldStorage->readStoredTile(m_worldExtent, tileX, tileZ);
		if (tile.heightfield && tile.heightfield->getSize() != m_tileSize)
		{
			log::error << L"Unable to edit terrain; tile " << tileX << L", " << tileZ << L" size mismatch." << Endl;
			tile.heightfield = nullptr;
		}
	}
	return tile.heightfield;
}

drawing::Image* TerrainEditData::getImageTile(int32_t image, int32_t& inoutX, int32_t& inoutZ)
{
	const int32_t tileX = inoutX / m_tileSize;
	const int32_t tileZ = inoutZ / m_tileSize;
	inoutX -= tileX * m_tileSize;
	inoutZ -= tileZ * m_tileSize;

	Tile& tile = m_tiles[tileX + tileZ * m_tileCount];
	if (!tile.images[image])
	{
		Ref< drawing::Image > imageTile = m_terrainStorage->readStoredImageTile(c_imageNames[image], tileX, tileZ);
		if (imageTile)
		{
			imageTile->convert(imageFormat(image));
			if (imageTile->getWidth() != m_tileSize || imageTile->getHeight() != m_tileSize)
			{
				drawing::ScaleFilter scaleFilter(m_tileSize, m_tileSize, drawing::ScaleFilter::MnAverage, drawing::ScaleFilter::MgLinear);
				imageTile->apply(&scaleFilter);
				m_writeAllImage[image] = true;
			}
		}
		else
		{
			imageTile = new drawing::Image(imageFormat(image), m_tileSize, m_tileSize);
			imageTile->clear(c_imageDefaults[image]);
			m_writeAllImage[image] = true;
		}
		tile.images[image] = imageTile;
	}
	return tile.images[image];
}

}
