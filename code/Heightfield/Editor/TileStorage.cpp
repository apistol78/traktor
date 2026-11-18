/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Heightfield/Editor/TileStorage.h"

#include "Compress/Zip/DeflateStreamZip.h"
#include "Compress/Zip/InflateStreamZip.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Date/DateTime.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Math/MathUtils.h"
#include "Core/Misc/Murmur3.h"
#include "Core/Misc/String.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Job.h"
#include "Core/Thread/JobManager.h"
#include "Core/Thread/Semaphore.h"
#include "Database/Instance.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/HeightfieldFormat.h"

#include <algorithm>
#include <cstring>

namespace traktor::hf
{
namespace
{

const uint32_t c_zipBufferSize = 65536;
const float c_overviewSampleDistance = 2.0f;			 //!< Maximum distance between overview samples, in world units.
const int32_t c_overviewMinSize = 2048;					 //!< Overview isn't downsampled below this size.
const int32_t c_rowsPerTask = 64;						 //!< Output rows per task; multiple of 8 so packed cut bits of tasks never share a byte.
const int64_t c_maxCachedBytes = 4096ll * 1024 * 1024;	 //!< Bytes of decoded tiles kept by region reads.
const uint32_t c_layers[] = { Heightfield::LySplat, Heightfield::LyColor };

struct CachedTile
{
	Ref< const Heightfield > tile;
	uint64_t lastWriteTime;
	uint64_t lastUsed;
};

Semaphore s_cacheLock;
SmallMap< std::wstring, CachedTile > s_cache;
int64_t s_cacheBytes = 0;
uint64_t s_cacheUseCounter = 0;

uint8_t* createLayer(Heightfield* heightfield, uint32_t layer)
{
	return (layer == Heightfield::LySplat) ? heightfield->createSplats() : heightfield->createColors();
}

const uint8_t* getLayer(const Heightfield* heightfield, uint32_t layer)
{
	return (layer == Heightfield::LySplat) ? heightfield->getSplats() : heightfield->getColors();
}

//! Memory of heights and optional layers of tile.
int64_t tileBytes(const Heightfield* tile)
{
	const int64_t samples = (int64_t)tile->getSize() * tile->getSize();
	return samples * (sizeof(height_t) + (tile->getSplats() ? 4 : 0) + (tile->getColors() ? 4 : 0));
}

void tileRange(const int32_t* region, int32_t tileCount, int32_t tileWidth, int32_t tileHeight, int32_t outRange[4])
{
	if (region)
	{
		outRange[0] = clamp(region[0] / tileWidth, 0, tileCount - 1);
		outRange[1] = clamp(region[1] / tileHeight, 0, tileCount - 1);
		outRange[2] = clamp(region[2] / tileWidth, 0, tileCount - 1);
		outRange[3] = clamp(region[3] / tileHeight, 0, tileCount - 1);
	}
	else
	{
		outRange[0] = outRange[1] = 0;
		outRange[2] = outRange[3] = tileCount - 1;
	}
}

//! Range, [outBegin, outEnd), of region samples which map onto tile; samples outside are clamped onto edge tiles.
void regionRange(int32_t tile, int32_t tileCount, int32_t tileSize, int32_t regionStart, int32_t regionSize, int32_t& outBegin, int32_t& outEnd)
{
	outBegin = (tile > 0) ? clamp(tile * tileSize - regionStart, 0, regionSize) : 0;
	outEnd = (tile < tileCount - 1) ? clamp((tile + 1) * tileSize - regionStart, 0, regionSize) : regionSize;
}

//! End of band of region rows starting at begin; bands end on multiples of c_rowsPerTask output rows.
int32_t bandEnd(int32_t begin, int32_t end, int32_t step)
{
	return std::min((begin / step / c_rowsPerTask + 1) * c_rowsPerTask * step, end);
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.hf.TileStorage", TileStorage, Object)

TileStorage::TileStorage(db::Instance* instance, int32_t tileCount)
	: m_instance(instance)
	, m_tileCount(tileCount)
{
}

TileStorage::TileStorage(const db::Instance* instance, int32_t tileCount)
	: m_instance(const_cast< db::Instance* >(instance))
	, m_tileCount(tileCount)
{
}

int32_t TileStorage::calculateOverviewSize(int32_t size, float worldSize)
{
	int32_t step = 1;
	while ((size % (step * 2)) == 0 && size / (step * 2) >= c_overviewMinSize && step * 2 * worldSize / size <= c_overviewSampleDistance)
		step *= 2;
	return size / step;
}

void TileStorage::flushCache()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(s_cacheLock);
	s_cache.clear();
	s_cacheBytes = 0;
}

int32_t TileStorage::readSize() const
{
	int32_t size;
	uint32_t layers;
	if (!readHeader(0, 0, size, layers))
		return 0;

	return size * std::max(m_tileCount, 1);
}

uint32_t TileStorage::readLayers() const
{
	const int32_t count = std::max(m_tileCount, 1);
	uint32_t layers = 0;
	for (int32_t tz = 0; tz < count && layers != Heightfield::LyAll; ++tz)
	{
		for (int32_t tx = 0; tx < count && layers != Heightfield::LyAll; ++tx)
		{
			int32_t size;
			uint32_t tileLayers;
			if (readHeader(tx, tz, size, tileLayers))
				layers |= tileLayers;
		}
	}
	return layers;
}

Ref< Heightfield > TileStorage::readHeightfield(const Vector4& worldExtent, uint32_t layers) const
{
	const int32_t size = readSize();
	if (size <= 0)
	{
		log::error << L"Unable to read heightfield; no data in instance." << Endl;
		return nullptr;
	}

	return readHeightfieldRegion(worldExtent, size / std::max(m_tileCount, 1), 0, 0, size, 1, layers);
}

Ref< Heightfield > TileStorage::readHeightfieldTile(const Vector4& worldExtent, int32_t tileX, int32_t tileZ, int32_t border, uint32_t layers) const
{
	const int32_t size = readSize();
	if (size <= 0)
	{
		log::error << L"Unable to read heightfield tile; no data in instance." << Endl;
		return nullptr;
	}

	const int32_t tileSize = size / std::max(m_tileCount, 1);
	return readHeightfieldRegion(worldExtent, tileSize, tileX * tileSize - border, tileZ * tileSize - border, tileSize + 1 + 2 * border, 1, layers);
}

Ref< Heightfield > TileStorage::readHeightfieldOverview(const Vector4& worldExtent, int32_t size, uint32_t layers) const
{
	const int32_t fullSize = readSize();
	if (fullSize <= 0 || size <= 0 || (fullSize % size) != 0)
	{
		log::error << L"Unable to read heightfield overview; size " << fullSize << L" not divisible into " << size << L"." << Endl;
		return nullptr;
	}

	return readHeightfieldRegion(worldExtent, fullSize / std::max(m_tileCount, 1), 0, 0, fullSize, fullSize / size, layers);
}

Ref< Heightfield > TileStorage::readHeightfieldOverview(const Vector4& worldExtent) const
{
	const int32_t fullSize = readSize();
	if (fullSize <= 0)
	{
		log::error << L"Unable to read heightfield overview; no data in instance." << Endl;
		return nullptr;
	}

	return readHeightfieldOverview(worldExtent, calculateOverviewSize(fullSize, worldExtent.x()));
}

bool TileStorage::writeHeightfield(const Heightfield* heightfield, const int32_t* region) const
{
	if (m_tileCount <= 0)
		return writeStoredTile(0, 0, heightfield);

	const int32_t size = heightfield->getSize();
	if ((size % m_tileCount) != 0)
	{
		log::error << L"Unable to write heightfield; size " << size << L" not divisible into " << m_tileCount << L" tiles." << Endl;
		return false;
	}

	const int32_t tileSize = size / m_tileCount;

	int32_t range[4];
	tileRange(region, m_tileCount, tileSize, tileSize, range);

	for (int32_t tz = range[1]; tz <= range[3]; ++tz)
	{
		for (int32_t tx = range[0]; tx <= range[2]; ++tx)
		{
			const int32_t x0 = tx * tileSize;
			const int32_t z0 = tz * tileSize;

			Ref< Heightfield > tile = new Heightfield(tileSize, heightfield->getWorldExtent(), Vector4::zero());

			for (int32_t z = 0; z < tileSize; ++z)
			{
				std::memcpy(tile->getHeights() + z * tileSize, heightfield->getHeights() + x0 + (z0 + z) * size, tileSize * sizeof(height_t));
				for (int32_t x = 0; x < tileSize; ++x)
					tile->setGridCut(x, z, heightfield->getGridCut(x0 + x, z0 + z));
			}

			for (uint8_t a = 0; a < Heightfield::MaxAttributes; ++a)
			{
				const uint8_t* src = heightfield->getAttributes(a);
				if (!src)
					continue;

				uint8_t* dst = tile->createAttributes(a);
				for (int32_t z = 0; z < tileSize; ++z)
					std::memcpy(dst + z * tileSize, src + x0 + (z0 + z) * size, tileSize);
			}

			for (const uint32_t layer : c_layers)
			{
				const uint8_t* src = getLayer(heightfield, layer);
				if (!src)
					continue;

				uint8_t* dst = createLayer(tile, layer);
				for (int32_t z = 0; z < tileSize; ++z)
					std::memcpy(dst + (size_t)z * tileSize * 4, src + ((size_t)x0 + (size_t)(z0 + z) * size) * 4, tileSize * 4);
			}

			if (!writeStoredTile(tx, tz, tile))
				return false;
		}
	}

	return true;
}

uint32_t TileStorage::readStamp(const std::wstring& name, int32_t tileX0, int32_t tileZ0, int32_t tileX1, int32_t tileZ1) const
{
	const int32_t count = std::max(m_tileCount, 1);

	Murmur3 hash;
	hash.begin();

	for (int32_t tz = std::max(tileZ0, 0); tz <= std::min(tileZ1, count - 1); ++tz)
	{
		for (int32_t tx = std::max(tileX0, 0); tx <= std::min(tileX1, count - 1); ++tx)
		{
			DateTime lastWriteTime;
			const uint64_t stamp[] = {
				(uint64_t)tx,
				(uint64_t)tz,
				m_instance->getDataLastWriteTime(getChannelName(name, tx, tz), lastWriteTime) ? lastWriteTime.getSecondsSinceEpoch() : 0
			};
			hash.feedBuffer(stamp, sizeof(stamp));
		}
	}

	hash.end();
	return hash.get();
}

Ref< Heightfield > TileStorage::readStoredTile(const Vector4& worldExtent, int32_t tileX, int32_t tileZ) const
{
	const std::wstring name = getChannelName(L"Data", tileX, tileZ);

	Ref< IStream > stream = m_instance->readData(name);
	if (!stream)
	{
		log::error << L"Unable to read heightfield; no \"" << name << L"\" in instance." << Endl;
		return nullptr;
	}

	Ref< Heightfield > tile;
	if (m_tileCount > 0)
	{
		Ref< IStream > inflate = new compress::InflateStreamZip(stream, c_zipBufferSize);
		tile = HeightfieldFormat().read(inflate, worldExtent, Vector4::zero());
		inflate->close();
	}
	else
		tile = HeightfieldFormat().read(stream, worldExtent, Vector4::zero());

	stream->close();

	if (!tile)
		log::error << L"Unable to read heightfield; corrupt \"" << name << L"\"." << Endl;

	return tile;
}

bool TileStorage::writeStoredTile(int32_t tileX, int32_t tileZ, const Heightfield* tile) const
{
	const std::wstring name = getChannelName(L"Data", tileX, tileZ);

	Ref< IStream > stream = m_instance->writeData(name);
	if (!stream)
	{
		log::error << L"Unable to write heightfield; failed to create \"" << name << L"\" in instance." << Endl;
		return false;
	}

	bool result;
	if (m_tileCount > 0)
	{
		Ref< IStream > deflate = new compress::DeflateStreamZip(stream, c_zipBufferSize);
		result = HeightfieldFormat().write(deflate, tile);
		deflate->close();
	}
	else
		result = HeightfieldFormat().write(stream, tile);

	stream->close();

	if (!result)
		log::error << L"Unable to write heightfield; failed to write \"" << name << L"\"." << Endl;

	return result;
}

std::wstring TileStorage::getChannelName(const std::wstring& name, int32_t tileX, int32_t tileZ) const
{
	return (m_tileCount > 0) ? name + L"_" + toString(tileX) + L"_" + toString(tileZ) : name;
}

bool TileStorage::readHeader(int32_t tileX, int32_t tileZ, int32_t& outSize, uint32_t& outLayers) const
{
	Ref< IStream > stream = m_instance->readData(getChannelName(L"Data", tileX, tileZ));
	if (!stream)
		return false;

	Ref< IStream > source = stream;
	if (m_tileCount > 0)
		source = new compress::InflateStreamZip(stream, 4096);

	const bool result = HeightfieldFormat().readHeader(source, outSize, outLayers);

	if (source != stream)
		source->close();
	stream->close();

	return result;
}

Ref< const Heightfield > TileStorage::readCachedTile(const Vector4& worldExtent, int32_t tileX, int32_t tileZ) const
{
	const std::wstring name = getChannelName(L"Data", tileX, tileZ);
	const std::wstring key = m_instance->getGuid().format() + L"/" + name;

	DateTime lastWriteTime;
	m_instance->getDataLastWriteTime(name, lastWriteTime);

	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(s_cacheLock);
		auto it = s_cache.find(key);
		if (it != s_cache.end() && it->second.lastWriteTime == (uint64_t)lastWriteTime)
		{
			it->second.lastUsed = ++s_cacheUseCounter;
			return it->second.tile;
		}
	}

	Ref< const Heightfield > tile = readStoredTile(worldExtent, tileX, tileZ);
	if (!tile)
		return nullptr;

	T_ANONYMOUS_VAR(Acquire< Semaphore >)(s_cacheLock);

	auto it = s_cache.find(key);
	if (it != s_cache.end())
	{
		s_cacheBytes -= tileBytes(it->second.tile);
		s_cache.erase(it);
	}

	// Evict least recently used tiles until new tile fit within budget.
	const int64_t bytes = tileBytes(tile);
	while (!s_cache.empty() && s_cacheBytes + bytes > c_maxCachedBytes)
	{
		auto lru = s_cache.begin();
		for (auto i = s_cache.begin(); i != s_cache.end(); ++i)
			if (i->second.lastUsed < lru->second.lastUsed)
				lru = i;
		s_cacheBytes -= tileBytes(lru->second.tile);
		s_cache.erase(lru);
	}

	s_cache.insert(key, { tile, (uint64_t)lastWriteTime, ++s_cacheUseCounter });
	s_cacheBytes += bytes;
	return tile;
}

Ref< Heightfield > TileStorage::readHeightfieldRegion(const Vector4& worldExtent, int32_t tileSize, int32_t x0, int32_t z0, int32_t width, int32_t step, uint32_t layers) const
{
	const int32_t count = std::max(m_tileCount, 1);
	const int32_t size = tileSize * count;
	const int32_t outSize = width / step;
	const float cellX = worldExtent.x() / size;
	const float cellZ = worldExtent.z() / size;

	const Vector4 extent = (width == size) ? worldExtent : Vector4(cellX * width, worldExtent.y(), cellZ * width, 0.0f);
	Ref< Heightfield > heightfield = new Heightfield(outSize, extent, Vector4::zero());

	// Optional layers are only read if stored; tiles without layer contribute the layer's default.
	const uint32_t optionalLayers = (layers != 0) ? (layers & readLayers()) : 0;
	Ref< Heightfield > defaults = new Heightfield(1, worldExtent, Vector4::zero());

	// Box filtered regions are accumulated before being normalized.
	AlignedVector< uint32_t > heightSums;
	AlignedVector< uint32_t > cutSums;
	AlignedVector< uint32_t > attributeSums[Heightfield::MaxAttributes];
	AlignedVector< uint32_t > layerSums[sizeof_array(c_layers)];
	uint8_t* attributes[Heightfield::MaxAttributes] = { nullptr };
	uint8_t* outputLayers[sizeof_array(c_layers)] = { nullptr };
	if (step > 1)
	{
		heightSums.resize(outSize * outSize, 0);
		cutSums.resize(outSize * outSize, 0);
	}
	for (int32_t i = 0; i < sizeof_array(c_layers); ++i)
	{
		if ((optionalLayers & c_layers[i]) == 0)
			continue;

		outputLayers[i] = createLayer(heightfield, c_layers[i]);
		createLayer(defaults, c_layers[i]);
		if (step > 1)
			layerSums[i].resize((size_t)outSize * outSize * 4, 0);
	}

	const int32_t mnx = clamp(x0, 0, size - 1) / tileSize;
	const int32_t mnz = clamp(z0, 0, size - 1) / tileSize;
	const int32_t mxx = clamp(x0 + width - 1, 0, size - 1) / tileSize;
	const int32_t mxz = clamp(z0 + width - 1, 0, size - 1) / tileSize;

	for (int32_t tz = mnz; tz <= mxz; ++tz)
	{
		// Tiles of a row are decoded in parallel.
		AlignedVector< Ref< const Heightfield > > rowTiles(mxx - mnx + 1);
		AlignedVector< Job::task_t > tasks;
		for (int32_t tx = mnx; tx <= mxx; ++tx)
			tasks.push_back([&, tx]() {
				rowTiles[tx - mnx] = readCachedTile(worldExtent, tx, tz);
			});
		JobManager::getInstance().fork(tasks.c_ptr(), tasks.size());

		for (int32_t tx = mnx; tx <= mxx; ++tx)
		{
			const Heightfield* tile = rowTiles[tx - mnx];
			if (!tile)
				return nullptr;

			if (tile->getSize() != tileSize)
			{
				log::error << L"Unable to read heightfield; tile " << tx << L", " << tz << L" size mismatch." << Endl;
				return nullptr;
			}

			for (uint8_t a = 0; a < Heightfield::MaxAttributes; ++a)
			{
				if (!tile->getAttributes(a))
					continue;

				if (step > 1)
				{
					if (attributeSums[a].empty())
						attributeSums[a].resize(outSize * outSize, 0);
				}
				else if (!attributes[a])
					attributes[a] = heightfield->createAttributes(a);
			}
		}

		int32_t iz0, iz1;
		regionRange(tz, count, tileSize, z0, width, iz0, iz1);

		// Bands of whole output rows are accumulated in parallel.
		tasks.resize(0);
		for (int32_t bz0 = iz0; bz0 < iz1; bz0 = bandEnd(bz0, iz1, step))
		{
			const int32_t bz1 = bandEnd(bz0, iz1, step);
			tasks.push_back([&, bz0, bz1]() {
				for (int32_t tx = mnx; tx <= mxx; ++tx)
				{
					const Heightfield* tile = rowTiles[tx - mnx];
					const height_t* tileHeights = tile->getHeights();

					const uint8_t* tileAttributes[Heightfield::MaxAttributes];
					for (uint8_t a = 0; a < Heightfield::MaxAttributes; ++a)
						tileAttributes[a] = tile->getAttributes(a);

					// Default of layer is a single sample, thus read with zero stride.
					const uint8_t* tileLayers[sizeof_array(c_layers)] = { nullptr };
					size_t tileLayerStrides[sizeof_array(c_layers)] = { 0 };
					for (int32_t i = 0; i < sizeof_array(c_layers); ++i)
					{
						if (!outputLayers[i])
							continue;

						tileLayers[i] = getLayer(tile, c_layers[i]);
						tileLayerStrides[i] = tileLayers[i] ? 4 : 0;
						if (!tileLayers[i])
							tileLayers[i] = getLayer(defaults, c_layers[i]);
					}

					int32_t ix0, ix1;
					regionRange(tx, count, tileSize, x0, width, ix0, ix1);

					for (int32_t iz = bz0; iz < bz1; ++iz)
					{
						const int32_t lz = clamp(z0 + iz, 0, size - 1) - tz * tileSize;
						const int32_t oz = iz / step;

						for (int32_t ix = ix0; ix < ix1; ++ix)
						{
							const int32_t lx = clamp(x0 + ix, 0, size - 1) - tx * tileSize;
							const int32_t ox = ix / step;
							const int32_t src = lx + lz * tileSize;
							const int32_t dst = ox + oz * outSize;

							if (step > 1)
							{
								heightSums[dst] += tileHeights[src];
								cutSums[dst] += tile->getGridCut(lx, lz) ? 1 : 0;
								for (uint8_t a = 0; a < Heightfield::MaxAttributes; ++a)
									if (tileAttributes[a])
										attributeSums[a][dst] += tileAttributes[a][src];
								for (int32_t i = 0; i < sizeof_array(c_layers); ++i)
									if (tileLayers[i])
										for (int32_t c = 0; c < 4; ++c)
											layerSums[i][(size_t)dst * 4 + c] += tileLayers[i][src * tileLayerStrides[i] + c];
							}
							else
							{
								heightfield->getHeights()[dst] = tileHeights[src];
								heightfield->setGridCut(ox, oz, tile->getGridCut(lx, lz));
								for (uint8_t a = 0; a < Heightfield::MaxAttributes; ++a)
									if (tileAttributes[a])
										attributes[a][dst] = tileAttributes[a][src];
								for (int32_t i = 0; i < sizeof_array(c_layers); ++i)
									if (tileLayers[i])
										std::memcpy(outputLayers[i] + (size_t)dst * 4, tileLayers[i] + src * tileLayerStrides[i], 4);
							}
						}
					}
				}
			});
		}
		JobManager::getInstance().fork(tasks.c_ptr(), tasks.size());
	}

	if (step > 1)
	{
		const uint32_t n = step * step;
		for (int32_t i = 0; i < outSize * outSize; ++i)
		{
			heightfield->getHeights()[i] = (height_t)((heightSums[i] + n / 2) / n);
			heightfield->setGridCut(i % outSize, i / outSize, cutSums[i] * 2 >= n);
		}
		for (uint8_t a = 0; a < Heightfield::MaxAttributes; ++a)
		{
			if (attributeSums[a].empty())
				continue;

			uint8_t* attributes = heightfield->createAttributes(a);
			for (int32_t i = 0; i < outSize * outSize; ++i)
				attributes[i] = (uint8_t)((attributeSums[a][i] + n / 2) / n);
		}
		for (int32_t i = 0; i < sizeof_array(c_layers); ++i)
			for (size_t j = 0; j < layerSums[i].size(); ++j)
				outputLayers[i][j] = (uint8_t)((layerSums[i][j] + n / 2) / n);
	}

	heightfield->setWorldCenter(Vector4(
		cellX * (x0 + (width - size) * 0.5f),
		0.0f,
		cellZ * (z0 + (width - size) * 0.5f),
		0.0f));
	return heightfield;
}

}
