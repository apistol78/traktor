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
#include "Core/Io/Reader.h"
#include "Core/Log/Log.h"
#include "Core/Math/Color4f.h"
#include "Core/Math/MathUtils.h"
#include "Core/Misc/Murmur3.h"
#include "Core/Misc/String.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Job.h"
#include "Core/Thread/JobManager.h"
#include "Core/Thread/Semaphore.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
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
const int64_t c_maxCachedSamples = 1024ll * 1024 * 1024; //!< Samples of decoded tiles kept by region reads.

struct CachedTile
{
	Ref< const Heightfield > tile;
	uint64_t lastWriteTime;
	uint64_t lastUsed;
};

Semaphore s_cacheLock;
SmallMap< std::wstring, CachedTile > s_cache;
int64_t s_cacheSamples = 0;
uint64_t s_cacheUseCounter = 0;

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
	s_cacheSamples = 0;
}

int32_t TileStorage::readSize() const
{
	Ref< IStream > stream = m_instance->readData(getChannelName(L"Data", 0, 0));
	if (!stream)
		return 0;

	Ref< IStream > source = stream;
	if (m_tileCount > 0)
		source = new compress::InflateStreamZip(stream, 4096);

	int32_t version = 0;
	int32_t size = 0;

	Reader reader(source);
	reader >> version;
	reader >> size;

	if (source != stream)
		source->close();
	stream->close();

	return size * std::max(m_tileCount, 1);
}

Ref< Heightfield > TileStorage::readHeightfield(const Vector4& worldExtent) const
{
	if (m_tileCount <= 0)
		return readStoredTile(worldExtent, 0, 0);

	const int32_t size = readSize();
	if (size <= 0)
	{
		log::error << L"Unable to read heightfield; no data in instance." << Endl;
		return nullptr;
	}

	return readHeightfieldRegion(worldExtent, size / m_tileCount, 0, 0, size, 1);
}

Ref< Heightfield > TileStorage::readHeightfieldTile(const Vector4& worldExtent, int32_t tileX, int32_t tileZ, int32_t border) const
{
	const int32_t size = readSize();
	if (size <= 0)
	{
		log::error << L"Unable to read heightfield tile; no data in instance." << Endl;
		return nullptr;
	}

	const int32_t tileSize = size / std::max(m_tileCount, 1);
	return readHeightfieldRegion(worldExtent, tileSize, tileX * tileSize - border, tileZ * tileSize - border, tileSize + 1 + 2 * border, 1);
}

Ref< Heightfield > TileStorage::readHeightfieldOverview(const Vector4& worldExtent, int32_t size) const
{
	const int32_t fullSize = readSize();
	if (fullSize <= 0 || size <= 0 || (fullSize % size) != 0)
	{
		log::error << L"Unable to read heightfield overview; size " << fullSize << L" not divisible into " << size << L"." << Endl;
		return nullptr;
	}

	return readHeightfieldRegion(worldExtent, fullSize / std::max(m_tileCount, 1), 0, 0, fullSize, fullSize / size);
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

			if (!writeStoredTile(tx, tz, tile))
				return false;
		}
	}

	return true;
}

Ref< drawing::Image > TileStorage::readImage(const std::wstring& name) const
{
	if (m_tileCount <= 0)
		return readStoredImageTile(name, 0, 0);

	Ref< drawing::Image > anchor = readStoredImageTile(name, 0, 0);
	if (!anchor)
		return nullptr;

	return readImageRegion(name, anchor, 0, 0, anchor->getWidth() * m_tileCount, anchor->getHeight() * m_tileCount, 1, 1);
}

Ref< drawing::Image > TileStorage::readImageTile(const std::wstring& name, int32_t tileX, int32_t tileZ) const
{
	Ref< drawing::Image > anchor = readStoredImageTile(name, tileX, tileZ);
	if (!anchor)
		return nullptr;

	const int32_t tileWidth = anchor->getWidth();
	const int32_t tileHeight = anchor->getHeight();
	return readImageRegion(name, anchor, tileX * tileWidth, tileZ * tileHeight, tileWidth + 1, tileHeight + 1, 1, 1);
}

Ref< drawing::Image > TileStorage::readImageOverview(const std::wstring& name, int32_t size) const
{
	Ref< drawing::Image > anchor = readStoredImageTile(name, 0, 0);
	if (!anchor)
		return nullptr;

	const int32_t count = std::max(m_tileCount, 1);
	const int32_t width = anchor->getWidth() * count;
	const int32_t height = anchor->getHeight() * count;
	if (size <= 0 || (width % size) != 0 || (height % size) != 0)
	{
		log::error << L"Unable to read image overview; size " << width << L" * " << height << L" not divisible into " << size << L"." << Endl;
		return nullptr;
	}

	return readImageRegion(name, anchor, 0, 0, width, height, width / size, height / size);
}

bool TileStorage::writeImage(const std::wstring& name, const drawing::Image* image, const int32_t* region) const
{
	if (m_tileCount <= 0)
		return writeStoredImageTile(name, 0, 0, image);

	const int32_t width = image->getWidth();
	const int32_t height = image->getHeight();
	if ((width % m_tileCount) != 0 || (height % m_tileCount) != 0)
	{
		log::error << L"Unable to write image; size " << width << L" * " << height << L" not divisible into " << m_tileCount << L" tiles." << Endl;
		return false;
	}

	const int32_t tileWidth = width / m_tileCount;
	const int32_t tileHeight = height / m_tileCount;

	int32_t range[4];
	tileRange(region, m_tileCount, tileWidth, tileHeight, range);

	for (int32_t tz = range[1]; tz <= range[3]; ++tz)
	{
		for (int32_t tx = range[0]; tx <= range[2]; ++tx)
		{
			Ref< drawing::Image > tile = new drawing::Image(image->getPixelFormat(), tileWidth, tileHeight);
			tile->copy(image, tx * tileWidth, tz * tileHeight, tileWidth, tileHeight);

			if (!writeStoredImageTile(name, tx, tz, tile))
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

bool TileStorage::haveData(const std::wstring& name) const
{
	DateTime lastWriteTime;
	return m_instance->getDataLastWriteTime(getChannelName(name, 0, 0), lastWriteTime);
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

Ref< drawing::Image > TileStorage::readStoredImageTile(const std::wstring& name, int32_t tileX, int32_t tileZ) const
{
	const std::wstring channel = getChannelName(name, tileX, tileZ);

	Ref< IStream > stream = m_instance->readData(channel);
	if (!stream)
		return nullptr;

	Ref< drawing::Image > image;
	if (m_tileCount > 0)
	{
		Ref< IStream > inflate = new compress::InflateStreamZip(stream, c_zipBufferSize);
		image = drawing::Image::load(inflate, L"tri");
		inflate->close();
	}
	else
	{
		image = drawing::Image::load(stream, L"tri");
		if (!image)
		{
			stream->seek(IStream::SeekSet, 0);
			image = drawing::Image::load(stream, L"tga");
		}
	}

	stream->close();

	if (!image)
		log::error << L"Unable to read image; corrupt \"" << channel << L"\"." << Endl;

	return image;
}

bool TileStorage::writeStoredImageTile(const std::wstring& name, int32_t tileX, int32_t tileZ, const drawing::Image* tile) const
{
	const std::wstring channel = getChannelName(name, tileX, tileZ);

	Ref< IStream > stream = m_instance->writeData(channel);
	if (!stream)
	{
		log::error << L"Unable to write image; failed to create \"" << channel << L"\" in instance." << Endl;
		return false;
	}

	bool result;
	if (m_tileCount > 0)
	{
		Ref< IStream > deflate = new compress::DeflateStreamZip(stream, c_zipBufferSize);
		result = tile->save(deflate, L"tri");
		deflate->close();
	}
	else
		result = tile->save(stream, L"tri");

	stream->close();

	if (!result)
		log::error << L"Unable to write image; failed to write \"" << channel << L"\"." << Endl;

	return result;
}

std::wstring TileStorage::getChannelName(const std::wstring& name, int32_t tileX, int32_t tileZ) const
{
	return (m_tileCount > 0) ? name + L"_" + toString(tileX) + L"_" + toString(tileZ) : name;
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
		s_cacheSamples -= (int64_t)it->second.tile->getSize() * it->second.tile->getSize();
		s_cache.erase(it);
	}

	// Evict least recently used tiles until new tile fit within budget.
	const int64_t samples = (int64_t)tile->getSize() * tile->getSize();
	while (!s_cache.empty() && s_cacheSamples + samples > c_maxCachedSamples)
	{
		auto lru = s_cache.begin();
		for (auto i = s_cache.begin(); i != s_cache.end(); ++i)
			if (i->second.lastUsed < lru->second.lastUsed)
				lru = i;
		s_cacheSamples -= (int64_t)lru->second.tile->getSize() * lru->second.tile->getSize();
		s_cache.erase(lru);
	}

	s_cache.insert(key, { tile, (uint64_t)lastWriteTime, ++s_cacheUseCounter });
	s_cacheSamples += samples;
	return tile;
}

Ref< Heightfield > TileStorage::readHeightfieldRegion(const Vector4& worldExtent, int32_t tileSize, int32_t x0, int32_t z0, int32_t width, int32_t step) const
{
	const int32_t count = std::max(m_tileCount, 1);
	const int32_t size = tileSize * count;
	const int32_t outSize = width / step;
	const float cellX = worldExtent.x() / size;
	const float cellZ = worldExtent.z() / size;

	const Vector4 extent = (width == size) ? worldExtent : Vector4(cellX * width, worldExtent.y(), cellZ * width, 0.0f);
	Ref< Heightfield > heightfield = new Heightfield(outSize, extent, Vector4::zero());

	// Box filtered regions are accumulated before being normalized.
	AlignedVector< uint32_t > heightSums;
	AlignedVector< uint32_t > cutSums;
	AlignedVector< uint32_t > attributeSums[Heightfield::MaxAttributes];
	uint8_t* attributes[Heightfield::MaxAttributes] = { nullptr };
	if (step > 1)
	{
		heightSums.resize(outSize * outSize, 0);
		cutSums.resize(outSize * outSize, 0);
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
							}
							else
							{
								heightfield->getHeights()[dst] = tileHeights[src];
								heightfield->setGridCut(ox, oz, tile->getGridCut(lx, lz));
								for (uint8_t a = 0; a < Heightfield::MaxAttributes; ++a)
									if (tileAttributes[a])
										attributes[a][dst] = tileAttributes[a][src];
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
	}

	heightfield->setWorldCenter(Vector4(
		cellX * (x0 + (width - size) * 0.5f),
		0.0f,
		cellZ * (z0 + (width - size) * 0.5f),
		0.0f));
	return heightfield;
}

Ref< drawing::Image > TileStorage::readImageRegion(const std::wstring& name, const drawing::Image* anchor, int32_t x0, int32_t y0, int32_t width, int32_t height, int32_t stepX, int32_t stepY) const
{
	const int32_t count = std::max(m_tileCount, 1);
	const int32_t tileWidth = anchor->getWidth();
	const int32_t tileHeight = anchor->getHeight();
	const int32_t imageWidth = tileWidth * count;
	const int32_t imageHeight = tileHeight * count;
	const int32_t outWidth = width / stepX;
	const int32_t outHeight = height / stepY;
	const drawing::PixelFormat& pf = anchor->getPixelFormat();
	const int32_t pixelSize = pf.getByteSize();

	Ref< drawing::Image > image = new drawing::Image(pf, outWidth, outHeight);

	// Box filtered regions are accumulated before being normalized.
	AlignedVector< Color4f > sums;
	if (stepX > 1 || stepY > 1)
		sums.resize(outWidth * outHeight, Color4f(0.0f, 0.0f, 0.0f, 0.0f));

	const int32_t mnx = clamp(x0, 0, imageWidth - 1) / tileWidth;
	const int32_t mny = clamp(y0, 0, imageHeight - 1) / tileHeight;
	const int32_t mxx = clamp(x0 + width - 1, 0, imageWidth - 1) / tileWidth;
	const int32_t mxy = clamp(y0 + height - 1, 0, imageHeight - 1) / tileHeight;

	for (int32_t ty = mny; ty <= mxy; ++ty)
	{
		// Tiles of a row are decoded in parallel.
		AlignedVector< Ref< drawing::Image > > rowTiles(mxx - mnx + 1);
		AlignedVector< Job::task_t > tasks;
		for (int32_t tx = mnx; tx <= mxx; ++tx)
			tasks.push_back([&, tx]() {
				Ref< drawing::Image > tile = readStoredImageTile(name, tx, ty);
				if (tile && tile->getWidth() == tileWidth && tile->getHeight() == tileHeight && !(tile->getPixelFormat() == pf))
					tile->convert(pf);
				rowTiles[tx - mnx] = tile;
			});
		JobManager::getInstance().fork(tasks.c_ptr(), tasks.size());

		for (int32_t tx = mnx; tx <= mxx; ++tx)
		{
			const drawing::Image* tile = rowTiles[tx - mnx];
			if (!tile)
			{
				log::error << L"Unable to read image; no tile " << tx << L", " << ty << L" of \"" << name << L"\"." << Endl;
				return nullptr;
			}

			if (tile->getWidth() != tileWidth || tile->getHeight() != tileHeight)
			{
				log::error << L"Unable to read image; tile " << tx << L", " << ty << L" of \"" << name << L"\" size mismatch." << Endl;
				return nullptr;
			}
		}

		int32_t iy0, iy1;
		regionRange(ty, count, tileHeight, y0, height, iy0, iy1);

		// Bands of whole output rows are accumulated in parallel.
		tasks.resize(0);
		for (int32_t by0 = iy0; by0 < iy1; by0 = bandEnd(by0, iy1, stepY))
		{
			const int32_t by1 = bandEnd(by0, iy1, stepY);
			tasks.push_back([&, by0, by1]() {
				uint8_t* data = static_cast< uint8_t* >(image->getData());
				for (int32_t tx = mnx; tx <= mxx; ++tx)
				{
					const drawing::Image* tile = rowTiles[tx - mnx];
					const uint8_t* tileData = static_cast< const uint8_t* >(tile->getData());

					int32_t ix0, ix1;
					regionRange(tx, count, tileWidth, x0, width, ix0, ix1);

					for (int32_t iy = by0; iy < by1; ++iy)
					{
						const int32_t ly = clamp(y0 + iy, 0, imageHeight - 1) - ty * tileHeight;
						const int32_t oy = iy / stepY;

						for (int32_t ix = ix0; ix < ix1; ++ix)
						{
							const int32_t lx = clamp(x0 + ix, 0, imageWidth - 1) - tx * tileWidth;
							const int32_t ox = ix / stepX;

							if (!sums.empty())
							{
								Color4f color;
								tile->getPixelUnsafe(lx, ly, color);
								sums[ox + oy * outWidth] += color;
							}
							else
								std::memcpy(data + (ox + oy * outWidth) * pixelSize, tileData + (lx + ly * tileWidth) * pixelSize, pixelSize);
						}
					}
				}
			});
		}
		JobManager::getInstance().fork(tasks.c_ptr(), tasks.size());
	}

	if (!sums.empty())
	{
		const Scalar n(1.0f / (stepX * stepY));
		for (int32_t i = 0; i < outWidth * outHeight; ++i)
			image->setPixelUnsafe(i % outWidth, i / outWidth, sums[i] * n);
	}

	return image;
}

}
