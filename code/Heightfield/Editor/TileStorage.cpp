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
#include "Core/Containers/SmallMap.h"
#include "Core/Date/DateTime.h"
#include "Core/Guid.h"
#include "Core/Io/IStream.h"
#include "Core/Io/Reader.h"
#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"
#include "Core/Math/MathUtils.h"
#include "Core/Misc/Murmur3.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Job.h"
#include "Core/Thread/JobManager.h"
#include "Core/Thread/Semaphore.h"
#include "Database/Instance.h"
#include "Heightfield/Editor/HeightfieldAsset.h"
#include "Heightfield/Editor/HeightfieldAssetLayer.h"
#include "Heightfield/Editor/HeightfieldCache.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/HeightfieldFormat.h"

#include <algorithm>
#include <atomic>
#include <cstring>

namespace traktor::hf
{
namespace
{

const int32_t c_streamVersion = 2;
const uint32_t c_zipBufferSize = 65536;
const float c_overviewSampleDistance = 2.0f;			 //!< Maximum distance between overview samples, in world units.
const int32_t c_overviewMinSize = 2048;					 //!< Overview isn't downsampled below this size.
const int32_t c_overviewMaxSize = 4096;					 //!< Overview is downsampled until within this size, regardless of sample distance.
const int32_t c_rowsPerTask = 64;						 //!< Output rows per task; multiple of 8 so packed cut bits of tasks never share a byte.
const int64_t c_maxCachedBytes = 4096ll * 1024 * 1024;	 //!< Bytes of decoded tiles kept by region reads.
const uint32_t c_channels[] = { Heightfield::LySplat, Heightfield::LyColor };

//! Data streams of a tile; one stream for each attribute follow mask.
enum Stream
{
	StHeights,
	StCuts,
	StSplats,
	StColors,
	StMask,
	StAttributes
};

const wchar_t* c_streamNames[] = { L"Heights", L"Cuts", L"Splats", L"Colors", L"Mask" };

struct CachedTile
{
	Ref< const Heightfield > tile;
	uint64_t stamp;
	uint64_t lastUsed;
};

Semaphore s_cacheLock;
SmallMap< std::wstring, CachedTile > s_cache;
int64_t s_cacheBytes = 0;
uint64_t s_cacheUseCounter = 0;

//! Compressed data stream of instance, closed when destroyed.
struct ReadStream
{
	Ref< IStream > data;
	Ref< IStream > inflate;

	~ReadStream()
	{
		safeClose(inflate);
		safeClose(data);
	}

	//! Open stream and read its header; stamp is zero if stream was stored before stamps were.
	bool open(const db::Instance* instance, const std::wstring& name, int32_t& outSize, uint64_t& outStamp)
	{
		if ((data = instance->readData(name)) == nullptr)
			return false;

		inflate = new compress::InflateStreamZip(data, c_zipBufferSize);

		int32_t version = 0;
		Reader(inflate) >> version;
		Reader(inflate) >> outSize;
		if (version < 1 || version > c_streamVersion || outSize <= 0)
			return false;

		outStamp = 0;
		if (version >= 2)
			Reader(inflate) >> outStamp;

		return true;
	}

	bool read(void* block, int64_t bytes)
	{
		return Reader(inflate).read(block, bytes) == bytes;
	}
};

std::wstring getStreamName(int32_t stream)
{
	return (stream < StAttributes) ? std::wstring(c_streamNames[stream]) : L"Attribute" + toString(stream - StAttributes);
}

uint64_t hash64(const void* data, size_t bytes)
{
	Murmur3 low(0), high(1);
	for (Murmur3* hash : { &low, &high })
	{
		hash->begin();
		hash->feedBuffer(data, bytes);
		hash->end();
	}
	return ((uint64_t)high.get() << 32) | low.get();
}

//! Unique stamp of newly written data.
uint64_t createStamp()
{
	const Guid guid = Guid::create();
	return hash64((const uint8_t*)guid, 16);
}

uint8_t* createChannel(Heightfield* heightfield, uint32_t channel)
{
	return (channel == Heightfield::LySplat) ? heightfield->createSplats() : heightfield->createColors();
}

const uint8_t* getChannel(const Heightfield* heightfield, uint32_t channel)
{
	return (channel == Heightfield::LySplat) ? heightfield->getSplats() : heightfield->getColors();
}

int32_t getChannelStream(uint32_t channel)
{
	return (channel == Heightfield::LySplat) ? StSplats : StColors;
}

//! Optional channels, Heightfield::Layers, of tile.
uint32_t tileChannels(const Heightfield* tile)
{
	uint32_t channels = 0;
	for (const uint32_t channel : c_channels)
		if (getChannel(tile, channel))
			channels |= channel;
	return channels;
}

//! Attributes, one bit each, of tile which carry any density.
uint32_t tileAttributes(const Heightfield* tile)
{
	uint32_t attributes = 0;
	for (uint8_t a = 0; a < Heightfield::MaxAttributes; ++a)
		if (tile->haveAttribute(a))
			attributes |= 1 << a;
	return attributes;
}

//! Memory of heights and optional channels of tile.
int64_t tileBytes(const Heightfield* tile)
{
	const int64_t samples = (int64_t)tile->getSize() * tile->getSize();
	return samples * (sizeof(height_t) + (tile->getSplats() ? 4 : 0) + (tile->getColors() ? 4 : 0));
}

Vector4 getTileExtent(const Vector4& worldExtent, int32_t count)
{
	return worldExtent * Vector4(1.0f / count, 1.0f, 1.0f / count, 0.0f);
}

Vector4 getTileCenter(const Vector4& worldExtent, int32_t count, int32_t tileX, int32_t tileZ)
{
	return worldExtent * Vector4((tileX + 0.5f) / count - 0.5f, 0.0f, (tileZ + 0.5f) / count - 0.5f, 0.0f);
}

//! Read data of stream which header match size.
bool readStream(const db::Instance* instance, const std::wstring& name, int32_t size, void* data, int64_t bytes)
{
	ReadStream stream;
	int32_t streamSize;
	uint64_t stamp;
	return stream.open(instance, name, streamSize, stamp) && streamSize == size && stream.read(data, bytes);
}

//! Write stream of tile; heights also carry which optional channels and attributes tile has.
bool writeStream(IStream* stream, const Heightfield* tile, const uint8_t* mask, int32_t which, uint64_t stamp)
{
	const int32_t size = tile->getSize();
	const int64_t samples = (int64_t)size * size;

	Ref< IStream > deflate = new compress::DeflateStreamZip(stream, c_zipBufferSize);
	Writer writer(deflate);
	writer << c_streamVersion;
	writer << size;
	writer << stamp;

	int64_t bytes = 0;
	int64_t written = 0;
	switch (which)
	{
	case StHeights:
		writer << tileChannels(tile);
		writer << tileAttributes(tile);
		bytes = samples * sizeof(height_t);
		written = writer.write(tile->getHeights(), bytes);
		break;

	case StCuts:
		bytes = (samples + 7) / 8;
		written = writer.write(tile->getCuts(), bytes);
		break;

	case StSplats:
		bytes = samples * 4;
		written = writer.write(tile->getSplats(), bytes);
		break;

	case StColors:
		bytes = samples * 4;
		written = writer.write(tile->getColors(), bytes);
		break;

	case StMask:
		bytes = samples;
		written = writer.write(mask, bytes);
		break;

	default:
		bytes = samples;
		written = writer.write(tile->getAttributes((uint8_t)(which - StAttributes)), bytes);
		break;
	}

	deflate->close();
	stream->close();
	return written == bytes;
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

TileStorage::TileStorage(db::Instance* instance, const HeightfieldAsset* asset, const Path& cachePath)
	: TileStorage(instance, asset, asset->getTileCount())
{
	m_cachePath = cachePath;
}

TileStorage::TileStorage(const db::Instance* instance, const HeightfieldAsset* asset, const Path& cachePath)
	: TileStorage(const_cast< db::Instance* >(instance), asset, asset->getTileCount())
{
	m_cachePath = cachePath;
}

TileStorage::TileStorage(db::Instance* instance, const HeightfieldAsset* asset, int32_t tileCount)
	: m_instance(instance)
	, m_tileCount(tileCount)
{
	for (const HeightfieldAssetLayer* layer : asset->getLayers())
	{
		m_layerHashes.push_back((m_layers.empty() ? 0 : m_layerHashes.back() * 31) + DeepHash(layer).get());
		m_layers.push_back(layer);
	}

	const float base[] = { asset->getWorldExtent().x(), asset->getWorldExtent().y(), asset->getWorldExtent().z(), (float)m_tileCount };
	m_baseStamp = hash64(base, sizeof(base));

	const int32_t count = std::max(m_tileCount, 1);
	const int32_t tiles = count * count;
	m_ownStamps.resize(m_layers.size() * tiles, 0);
	m_stamps.resize(m_layers.size() * tiles, 0);
	m_globalStamps.resize(m_layers.size(), 0);

	for (int32_t layer = 0; layer < (int32_t)m_layers.size(); ++layer)
		for (int32_t i = 0; i < tiles; ++i)
			m_ownStamps[layer * tiles + i] = readOwnStamp(layer, i % count, i / count);

	calculateStamps(true);
}

int32_t TileStorage::calculateOverviewSize(int32_t size, float worldSize)
{
	int32_t step = 1;
	while ((size % (step * 2)) == 0 && size / (step * 2) >= c_overviewMinSize && (size / step > c_overviewMaxSize || step * 2 * worldSize / size <= c_overviewSampleDistance))
		step *= 2;
	return size / step;
}

void TileStorage::flushCache()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(s_cacheLock);
	s_cache.clear();
	s_cacheBytes = 0;
}

std::wstring TileStorage::getCacheKey(int32_t layer) const
{
	return m_instance->getGuid().format() + L"/" + toString(m_tileCount) + L"/" + toString((layer >= 0) ? m_layers[layer]->getId() : -1);
}

uint64_t TileStorage::getStamp(int32_t layer, int32_t tileX, int32_t tileZ) const
{
	const int32_t count = std::max(m_tileCount, 1);
	return m_stamps[layer * count * count + tileX + tileZ * count];
}

uint64_t TileStorage::getGlobalStamp(int32_t layer) const
{
	return m_globalStamps[layer];
}

int32_t TileStorage::readSize() const
{
	int32_t size;
	uint32_t channels;
	if (!readHeader(0, 0, 0, size, channels))
		return 0;

	return size * std::max(m_tileCount, 1);
}

uint32_t TileStorage::readChannels() const
{
	const int32_t count = std::max(m_tileCount, 1);
	uint32_t channels = 0;
	for (int32_t layer = 0; layer < (int32_t)m_layers.size(); ++layer)
	{
		for (int32_t tz = 0; tz < count && channels != Heightfield::LyAll; ++tz)
		{
			for (int32_t tx = 0; tx < count && channels != Heightfield::LyAll; ++tx)
			{
				int32_t size;
				uint32_t tileChannels;
				if (readHeader(layer, tx, tz, size, tileChannels))
					channels |= tileChannels;
			}
		}
	}
	return channels;
}

Ref< Heightfield > TileStorage::readHeightfield(const Vector4& worldExtent, uint32_t channels) const
{
	const int32_t size = readSize();
	if (size <= 0)
	{
		log::error << L"Unable to read heightfield; no data in instance." << Endl;
		return nullptr;
	}

	return readHeightfieldRegion(-1, worldExtent, size / std::max(m_tileCount, 1), 0, 0, size, 1, channels);
}

Ref< Heightfield > TileStorage::readHeightfieldTile(const Vector4& worldExtent, int32_t tileX, int32_t tileZ, int32_t border, uint32_t channels) const
{
	const int32_t size = readSize();
	if (size <= 0)
	{
		log::error << L"Unable to read heightfield tile; no data in instance." << Endl;
		return nullptr;
	}

	const int32_t tileSize = size / std::max(m_tileCount, 1);
	return readHeightfieldRegion(-1, worldExtent, tileSize, tileX * tileSize - border, tileZ * tileSize - border, tileSize + 1 + 2 * border, 1, channels);
}

Ref< Heightfield > TileStorage::readHeightfieldOverview(const Vector4& worldExtent, int32_t size, uint32_t channels) const
{
	const int32_t fullSize = readSize();
	if (fullSize <= 0 || size <= 0 || (fullSize % size) != 0)
	{
		log::error << L"Unable to read heightfield overview; size " << fullSize << L" not divisible into " << size << L"." << Endl;
		return nullptr;
	}

	return readHeightfieldRegion(-1, worldExtent, fullSize / std::max(m_tileCount, 1), 0, 0, fullSize, fullSize / size, channels);
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

uint32_t TileStorage::readStamp(int32_t tileX0, int32_t tileZ0, int32_t tileX1, int32_t tileZ1) const
{
	const int32_t count = std::max(m_tileCount, 1);
	const int32_t top = (int32_t)m_layers.size() - 1;

	Murmur3 hash;
	hash.begin();
	for (int32_t tz = std::max(tileZ0, 0); tz <= std::min(tileZ1, count - 1); ++tz)
	{
		for (int32_t tx = std::max(tileX0, 0); tx <= std::min(tileX1, count - 1); ++tx)
		{
			const uint64_t stamp = getStamp(top, tx, tz);
			hash.feedBuffer(&stamp, sizeof(stamp));
		}
	}
	hash.end();
	return hash.get();
}

Ref< Heightfield > TileStorage::readMergedTile(const Vector4& worldExtent, int32_t tileX, int32_t tileZ, int32_t layerCount) const
{
	const int32_t layers = (layerCount >= 0) ? std::min(layerCount, (int32_t)m_layers.size()) : (int32_t)m_layers.size();
	if (layers <= 1)
	{
		Ref< Heightfield > merged = readStoredTile(0, worldExtent, tileX, tileZ);
		if (!merged)
			log::error << L"Unable to read heightfield; no tile " << tileX << L", " << tileZ << L" in instance." << Endl;
		return merged;
	}

	// Output of layer is read from cache, or evaluated from output of layer below, which is also cached.
	const int32_t layer = layers - 1;
	const int32_t count = std::max(m_tileCount, 1);
	const std::wstring name = m_instance->getGuid().format() + L"_" + toString(m_layers[layer]->getId()) + L"_" + toString(tileX) + L"_" + toString(tileZ);
	return HeightfieldCache::getInstance().get(m_cachePath, name, getStamp(layer, tileX, tileZ), getTileExtent(worldExtent, count), getTileCenter(worldExtent, count, tileX, tileZ), [&]() -> Ref< Heightfield > {
		Ref< Heightfield > merged = readMergedTile(worldExtent, tileX, tileZ, layer);
		if (!merged || !m_layers[layer]->apply(this, layer, worldExtent, tileX, tileZ, merged))
			return nullptr;
		merged->updateCellBounds();
		return merged;
	});
}

Ref< Heightfield > TileStorage::readLayerTile(int32_t layer, const Vector4& worldExtent, int32_t tileX, int32_t tileZ, AlignedVector< uint8_t >* outMask) const
{
	return m_layers[layer]->read(this, layer, worldExtent, tileX, tileZ, outMask);
}

bool TileStorage::writeLayerTiles(int32_t layer, int32_t tileX0, int32_t tileZ0, int32_t tileX1, int32_t tileZ1, const RefArray< const Heightfield >& tiles, const AlignedVector< AlignedVector< uint8_t > >* masks) const
{
	struct Write
	{
		Ref< IStream > stream;
		int32_t tile;
		int32_t which;
	};

	const int32_t count = std::max(m_tileCount, 1);
	const int32_t width = tileX1 - tileX0 + 1;

	const bool haveData = m_layers[layer]->haveData();
	const bool haveMask = m_layers[layer]->haveMask() && layer > 0;
	if (!haveData && !haveMask)
	{
		log::error << L"Unable to write heightfield; layer " << layer << L" doesn't store any data." << Endl;
		return false;
	}

	// Streams of a tile carry a new stamp; streams are created by calling thread, which owns the transaction, and encoded in parallel.
	AlignedVector< Write > writes;
	AlignedVector< uint64_t > stamps;
	for (int32_t i = 0; i < (int32_t)tiles.size(); ++i)
	{
		const Heightfield* tile = tiles[i];
		const int32_t tileX = tileX0 + i % width;
		const int32_t tileZ = tileZ0 + i / width;

		AlignedVector< int32_t > which;
		if (haveData)
		{
			which = { StHeights, StCuts };
			for (const uint32_t channel : c_channels)
				if (getChannel(tile, channel))
					which.push_back(getChannelStream(channel));
			for (uint8_t a = 0; a < Heightfield::MaxAttributes; ++a)
				if (tile->haveAttribute(a))
					which.push_back(StAttributes + a);
		}

		if (masks && haveMask)
		{
			if ((*masks)[i].size() != (size_t)tile->getSize() * tile->getSize())
			{
				log::error << L"Unable to write heightfield; mask of tile " << tileX << L", " << tileZ << L" size mismatch." << Endl;
				return false;
			}
			which.push_back(StMask);
		}

		for (const int32_t w : which)
		{
			const std::wstring name = getChannelName(layer, getStreamName(w), tileX, tileZ);
			Ref< IStream > stream = m_instance->writeData(name);
			if (!stream)
			{
				log::error << L"Unable to write heightfield; failed to create \"" << name << L"\" in instance." << Endl;
				return false;
			}
			writes.push_back({ stream, i, w });
		}

		stamps.push_back(createStamp());
	}

	std::atomic< bool > result = true;
	AlignedVector< Job::task_t > tasks;
	for (int32_t i = 0; i < (int32_t)writes.size(); ++i)
		tasks.push_back([&, i]() {
			const Write& write = writes[i];
			const uint8_t* mask = (write.which == StMask) ? (*masks)[write.tile].c_ptr() : nullptr;
			if (!writeStream(write.stream, tiles[write.tile], mask, write.which, stamps[write.tile]))
			{
				log::error << L"Unable to write heightfield; failed to write \"" << getStreamName(write.which) << L"\" of tile " << tileX0 + write.tile % width << L", " << tileZ0 + write.tile / width << L"." << Endl;
				result = false;
			}
		});
	JobManager::getInstance().fork(tasks.c_ptr(), tasks.size());

	if (!result)
		return false;

	for (int32_t i = 0; i < (int32_t)tiles.size(); ++i)
	{
		const int32_t tileX = tileX0 + i % width;
		const int32_t tileZ = tileZ0 + i / width;
		m_ownStamps[layer * count * count + tileX + tileZ * count] = readOwnStamp(layer, tileX, tileZ);
	}

	// Global layers aren't evaluated again for each write through storage, thus their outputs are unique to storage.
	if (!m_written)
	{
		const uint64_t unique = createStamp();
		for (uint64_t& globalStamp : m_globalStamps)
		{
			const uint64_t values[] = { globalStamp, unique };
			globalStamp = hash64(values, sizeof(values));
		}
		m_written = true;
	}

	calculateStamps(false);
	return true;
}

Ref< Heightfield > TileStorage::readLayerHeightfield(int32_t layer, const Vector4& worldExtent, AlignedVector< uint8_t >* outMask) const
{
	const int32_t size = readSize();
	if (size <= 0)
	{
		log::error << L"Unable to read heightfield; no data in instance." << Endl;
		return nullptr;
	}

	const int32_t count = std::max(m_tileCount, 1);
	const int32_t tileSize = size / count;

	Ref< Heightfield > heightfield = readHeightfieldRegion(layer, worldExtent, tileSize, 0, 0, size, 1, Heightfield::LyAll);
	if (!heightfield)
		return nullptr;

	if (outMask)
	{
		outMask->resize((size_t)size * size);

		// Masks are read through layer, since layer decide mask of tiles; tiles of a row are read in parallel.
		for (int32_t tz = 0; tz < count; ++tz)
		{
			AlignedVector< AlignedVector< uint8_t > > masks(count);
			AlignedVector< Job::task_t > tasks;
			for (int32_t tx = 0; tx < count; ++tx)
				tasks.push_back([&, tx]() {
					if (!readLayerTile(layer, worldExtent, tx, tz, &masks[tx]))
						masks[tx].clear();
				});
			JobManager::getInstance().fork(tasks.c_ptr(), tasks.size());

			for (int32_t tx = 0; tx < count; ++tx)
			{
				if (masks[tx].size() != (size_t)tileSize * tileSize)
				{
					log::error << L"Unable to read heightfield; mask of tile " << tx << L", " << tz << L" of layer " << layer << L" missing or size mismatch." << Endl;
					return nullptr;
				}

				for (int32_t z = 0; z < tileSize; ++z)
					std::memcpy(outMask->ptr() + (size_t)tx * tileSize + ((size_t)tz * tileSize + z) * size, masks[tx].c_ptr() + (size_t)z * tileSize, tileSize);
			}
		}
	}

	return heightfield;
}

bool TileStorage::writeLayerHeightfield(int32_t layer, const Heightfield* heightfield, const AlignedVector< uint8_t >* mask) const
{
	const int32_t count = std::max(m_tileCount, 1);
	const int32_t size = heightfield->getSize();
	if ((size % count) != 0)
	{
		log::error << L"Unable to write heightfield; size " << size << L" not divisible into " << count << L" tiles." << Endl;
		return false;
	}

	const int32_t tileSize = size / count;

	// Tiles of a row are written together so they're encoded in parallel.
	for (int32_t tz = 0; tz < count; ++tz)
	{
		RefArray< const Heightfield > tiles;
		AlignedVector< AlignedVector< uint8_t > > masks;

		for (int32_t tx = 0; tx < count; ++tx)
		{
			const int32_t x0 = tx * tileSize;
			const int32_t z0 = tz * tileSize;

			Ref< Heightfield > tile = new Heightfield(tileSize, heightfield->getWorldExtent(), Vector4::zero());

			for (int32_t z = 0; z < tileSize; ++z)
			{
				std::memcpy(tile->getHeights() + (size_t)z * tileSize, heightfield->getHeights() + x0 + (size_t)(z0 + z) * size, tileSize * sizeof(height_t));
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
					std::memcpy(dst + (size_t)z * tileSize, src + x0 + (size_t)(z0 + z) * size, tileSize);
			}

			for (const uint32_t channel : c_channels)
			{
				const uint8_t* src = getChannel(heightfield, channel);
				if (!src)
					continue;

				uint8_t* dst = createChannel(tile, channel);
				for (int32_t z = 0; z < tileSize; ++z)
					std::memcpy(dst + (size_t)z * tileSize * 4, src + ((size_t)x0 + (size_t)(z0 + z) * size) * 4, tileSize * 4);
			}

			if (mask)
			{
				AlignedVector< uint8_t >& tileMask = masks.push_back();
				tileMask.resize((size_t)tileSize * tileSize);
				for (int32_t z = 0; z < tileSize; ++z)
					std::memcpy(tileMask.ptr() + (size_t)z * tileSize, mask->c_ptr() + x0 + (size_t)(z0 + z) * size, tileSize);
			}

			tiles.push_back(tile);
		}

		if (!writeLayerTiles(layer, 0, tz, count - 1, tz, tiles, mask ? &masks : nullptr))
			return false;
	}

	return true;
}

Ref< Heightfield > TileStorage::createTile(const Vector4& worldExtent, int32_t tileX, int32_t tileZ) const
{
	const int32_t count = std::max(m_tileCount, 1);
	const int32_t size = readSize() / count;
	if (size <= 0)
	{
		log::error << L"Unable to create heightfield tile; no data in instance." << Endl;
		return nullptr;
	}

	Ref< Heightfield > tile = new Heightfield(size, getTileExtent(worldExtent, count), getTileCenter(worldExtent, count, tileX, tileZ));
	tile->setGridHeight(0, 0, 0.5f);
	std::fill_n(tile->getHeights(), (size_t)size * size, tile->getHeights()[0]);
	std::memset(tile->getCuts(), 0xff, ((size_t)size * size + 7) / 8);
	tile->updateCellBounds();
	return tile;
}

Ref< Heightfield > TileStorage::readStoredTile(int32_t layer, const Vector4& worldExtent, int32_t tileX, int32_t tileZ) const
{
	const int32_t count = std::max(m_tileCount, 1);
	const Vector4 tileExtent = getTileExtent(worldExtent, count);
	const Vector4 tileCenter = getTileCenter(worldExtent, count, tileX, tileZ);

	ReadStream heights;
	int32_t size;
	uint64_t stamp;
	if (!heights.open(m_instance, getChannelName(layer, c_streamNames[StHeights], tileX, tileZ), size, stamp))
	{
		if (layer > 0)
			return nullptr;

		Ref< IStream > stream = m_instance->readData(getChannelName(-1, L"Data", tileX, tileZ));
		if (!stream)
			return nullptr;

		Ref< Heightfield > tile;
		if (m_tileCount > 0)
		{
			Ref< IStream > inflate = new compress::InflateStreamZip(stream, c_zipBufferSize);
			tile = HeightfieldFormat().read(inflate, tileExtent, tileCenter);
			inflate->close();
		}
		else
			tile = HeightfieldFormat().read(stream, tileExtent, tileCenter);

		stream->close();

		if (!tile)
			log::error << L"Unable to read heightfield; corrupt tile " << tileX << L", " << tileZ << L"." << Endl;

		return tile;
	}

	uint32_t channels = 0;
	uint32_t attributes = 0;
	Reader(heights.inflate) >> channels;
	Reader(heights.inflate) >> attributes;

	const int64_t samples = (int64_t)size * size;
	Ref< Heightfield > tile = new Heightfield(size, tileExtent, tileCenter);

	bool result =
		heights.read(tile->getHeights(), samples * sizeof(height_t)) &&
		readStream(m_instance, getChannelName(layer, c_streamNames[StCuts], tileX, tileZ), size, tile->getCuts(), (samples + 7) / 8);

	for (uint8_t a = 0; a < Heightfield::MaxAttributes && result; ++a)
	{
		if ((attributes & (1 << a)) != 0)
			result = readStream(m_instance, getChannelName(layer, getStreamName(StAttributes + a), tileX, tileZ), size, tile->createAttributes(a), samples);
	}

	for (const uint32_t channel : c_channels)
	{
		if (result && (channels & channel) != 0)
			result = readStream(m_instance, getChannelName(layer, getStreamName(getChannelStream(channel)), tileX, tileZ), size, createChannel(tile, channel), samples * 4);
	}

	if (!result)
	{
		log::error << L"Unable to read heightfield; corrupt tile " << tileX << L", " << tileZ << L" of layer " << layer << L"." << Endl;
		return nullptr;
	}

	tile->updateCellBounds();
	return tile;
}

bool TileStorage::readStoredMask(int32_t layer, int32_t tileX, int32_t tileZ, int32_t size, AlignedVector< uint8_t >& outMask) const
{
	outMask.resize((size_t)size * size);
	return readStream(m_instance, getChannelName(layer, c_streamNames[StMask], tileX, tileZ), size, outMask.ptr(), outMask.size());
}

std::wstring TileStorage::getChannelName(int32_t layer, const std::wstring& channel, int32_t tileX, int32_t tileZ) const
{
	// Negative layer is unprefixed, as data was named before layers.
	const std::wstring name = (layer >= 0) ? L"L" + toString(m_layers[layer]->getId()) + L"_" + channel : channel;
	return (m_tileCount > 0) ? name + L"_" + toString(tileX) + L"_" + toString(tileZ) : name;
}

uint64_t TileStorage::readOwnStamp(int32_t layer, int32_t tileX, int32_t tileZ) const
{
	const std::wstring names[] = {
		getChannelName(layer, c_streamNames[StHeights], tileX, tileZ),
		getChannelName(layer, c_streamNames[StMask], tileX, tileZ),
		getChannelName(-1, L"Data", tileX, tileZ)
	};
	const bool stored[] = { m_layers[layer]->haveData(), m_layers[layer]->haveMask() && layer > 0, layer == 0 };

	uint64_t stamps[sizeof_array(names)] = { 0 };
	for (int32_t i = 0; i < sizeof_array(names); ++i)
	{
		if (!stored[i])
			continue;

		ReadStream stream;
		int32_t size;
		uint64_t stamp;
		if (stream.open(m_instance, names[i], size, stamp) && stamp != 0)
		{
			stamps[i] = stamp;
			continue;
		}

		DateTime lastWriteTime;
		if (stream.data && m_instance->getDataLastWriteTime(names[i], lastWriteTime))
			stamps[i] = lastWriteTime.getSecondsSinceEpoch();
	}

	return hash64(stamps, sizeof(stamps));
}

void TileStorage::calculateStamps(bool global) const
{
	const int32_t count = std::max(m_tileCount, 1);
	const int32_t tiles = count * count;

	// Output of global layer depend on entire output of layer below.
	for (int32_t layer = 0; layer < (int32_t)m_layers.size(); ++layer)
	{
		if (global && layer > 0)
		{
			const uint64_t below = hash64(&m_stamps[(layer - 1) * tiles], tiles * sizeof(uint64_t));
			const uint64_t stamps[] = { below, m_layerHashes[layer] };
			m_globalStamps[layer] = hash64(stamps, sizeof(stamps));
		}

		for (int32_t i = 0; i < tiles; ++i)
		{
			const uint64_t stamps[] = {
				(layer > 0) ? m_stamps[(layer - 1) * tiles + i] : m_baseStamp + i,
				m_layerHashes[layer],
				m_ownStamps[layer * tiles + i],
				m_layers[layer]->isGlobal() ? m_globalStamps[layer] : 0
			};
			m_stamps[layer * tiles + i] = hash64(stamps, sizeof(stamps));
		}
	}
}

bool TileStorage::readHeader(int32_t layer, int32_t tileX, int32_t tileZ, int32_t& outSize, uint32_t& outChannels) const
{
	ReadStream heights;
	uint64_t stamp;
	if (heights.open(m_instance, getChannelName(layer, c_streamNames[StHeights], tileX, tileZ), outSize, stamp))
	{
		Reader(heights.inflate) >> outChannels;
		return true;
	}

	if (layer > 0)
		return false;

	Ref< IStream > stream = m_instance->readData(getChannelName(-1, L"Data", tileX, tileZ));
	if (!stream)
		return false;

	Ref< IStream > source = stream;
	if (m_tileCount > 0)
		source = new compress::InflateStreamZip(stream, 4096);

	const bool result = HeightfieldFormat().readHeader(source, outSize, outChannels);

	if (source != stream)
		source->close();
	stream->close();

	return result;
}

Ref< const Heightfield > TileStorage::readCachedTile(int32_t layer, const Vector4& worldExtent, int32_t tileX, int32_t tileZ) const
{
	const std::wstring key = getCacheKey(layer) + L"/" + toString(tileX) + L"_" + toString(tileZ);
	const uint64_t stamp = getStamp((layer >= 0) ? layer : (int32_t)m_layers.size() - 1, tileX, tileZ);

	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(s_cacheLock);
		auto it = s_cache.find(key);
		if (it != s_cache.end() && it->second.stamp == stamp)
		{
			it->second.lastUsed = ++s_cacheUseCounter;
			return it->second.tile;
		}
	}

	Ref< const Heightfield > tile = (layer >= 0) ? readLayerTile(layer, worldExtent, tileX, tileZ) : readMergedTile(worldExtent, tileX, tileZ);
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

	s_cache.insert(key, { tile, stamp, ++s_cacheUseCounter });
	s_cacheBytes += bytes;
	return tile;
}

Ref< Heightfield > TileStorage::readHeightfieldRegion(int32_t layer, const Vector4& worldExtent, int32_t tileSize, int32_t x0, int32_t z0, int32_t width, int32_t step, uint32_t channels) const
{
	const int32_t count = std::max(m_tileCount, 1);
	const int32_t size = tileSize * count;
	const int32_t outSize = width / step;
	const float cellX = worldExtent.x() / size;
	const float cellZ = worldExtent.z() / size;

	const Vector4 extent = (width == size) ? worldExtent : Vector4(cellX * width, worldExtent.y(), cellZ * width, 0.0f);
	Ref< Heightfield > heightfield = new Heightfield(outSize, extent, Vector4::zero());

	// Optional channels are only read if stored; tiles without channel contribute the channel's default.
	const uint32_t optionalChannels = (channels != 0) ? (channels & readChannels()) : 0;
	Ref< Heightfield > defaults = new Heightfield(1, worldExtent, Vector4::zero());

	// Box filtered regions are accumulated before being normalized.
	AlignedVector< uint32_t > heightSums;
	AlignedVector< uint32_t > cutSums;
	AlignedVector< uint32_t > attributeSums[Heightfield::MaxAttributes];
	AlignedVector< uint32_t > channelSums[sizeof_array(c_channels)];
	uint8_t* attributes[Heightfield::MaxAttributes] = { nullptr };
	uint8_t* outputChannels[sizeof_array(c_channels)] = { nullptr };
	if (step > 1)
	{
		heightSums.resize(outSize * outSize, 0);
		cutSums.resize(outSize * outSize, 0);
	}
	for (int32_t i = 0; i < sizeof_array(c_channels); ++i)
	{
		if ((optionalChannels & c_channels[i]) == 0)
			continue;

		outputChannels[i] = createChannel(heightfield, c_channels[i]);
		createChannel(defaults, c_channels[i]);
		if (step > 1)
			channelSums[i].resize((size_t)outSize * outSize * 4, 0);
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
				rowTiles[tx - mnx] = readCachedTile(layer, worldExtent, tx, tz);
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

					// Default of channel is a single sample, thus read with zero stride.
					const uint8_t* tileChannels[sizeof_array(c_channels)] = { nullptr };
					size_t tileChannelStrides[sizeof_array(c_channels)] = { 0 };
					for (int32_t i = 0; i < sizeof_array(c_channels); ++i)
					{
						if (!outputChannels[i])
							continue;

						tileChannels[i] = getChannel(tile, c_channels[i]);
						tileChannelStrides[i] = tileChannels[i] ? 4 : 0;
						if (!tileChannels[i])
							tileChannels[i] = getChannel(defaults, c_channels[i]);
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
								for (int32_t i = 0; i < sizeof_array(c_channels); ++i)
									if (tileChannels[i])
										for (int32_t c = 0; c < 4; ++c)
											channelSums[i][(size_t)dst * 4 + c] += tileChannels[i][src * tileChannelStrides[i] + c];
							}
							else
							{
								heightfield->getHeights()[dst] = tileHeights[src];
								heightfield->setGridCut(ox, oz, tile->getGridCut(lx, lz));
								for (uint8_t a = 0; a < Heightfield::MaxAttributes; ++a)
									if (tileAttributes[a])
										attributes[a][dst] = tileAttributes[a][src];
								for (int32_t i = 0; i < sizeof_array(c_channels); ++i)
									if (tileChannels[i])
										std::memcpy(outputChannels[i] + (size_t)dst * 4, tileChannels[i] + src * tileChannelStrides[i], 4);
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
		for (int32_t i = 0; i < sizeof_array(c_channels); ++i)
			for (size_t j = 0; j < channelSums[i].size(); ++j)
				outputChannels[i][j] = (uint8_t)((channelSums[i][j] + n / 2) / n);
	}

	heightfield->setWorldCenter(Vector4(
		cellX * (x0 + (width - size) * 0.5f),
		0.0f,
		cellZ * (z0 + (width - size) * 0.5f),
		0.0f));
	return heightfield;
}

}
