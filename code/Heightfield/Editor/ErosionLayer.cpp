/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Heightfield/Editor/ErosionLayer.h"

#include "Core/Containers/SmallMap.h"
#include "Core/IRefCount.h"
#include "Core/Log/Log.h"
#include "Core/Math/MathUtils.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/System/OS.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadPool.h"
#include "Heightfield/Editor/ErosionFilter.h"
#include "Heightfield/Editor/TileStorage.h"
#include "Heightfield/Heightfield.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstring>

namespace traktor::hf
{
namespace
{

const int32_t c_maxReadThreads = 16;	//!< Bounds memory of tiles read at once.

//! Eroded heightfield of a layer, with its own lock so layers below can be eroded while it's read.
struct Eroded : public RefCountImpl< IRefCount >
{
	Semaphore lock;
	uint64_t stamp = 0;
	Ref< const Heightfield > below;
	Ref< const Heightfield > eroded;
};

Semaphore s_erodedLock;
SmallMap< std::wstring, Ref< Eroded > > s_eroded;

//! Catmull-Rom spline between p1 and p2.
float catmullRom(float p0, float p1, float p2, float p3, float t)
{
	return p1 + 0.5f * t * (p2 - p0 + t * (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3 + t * (3.0f * (p1 - p2) + p3 - p0)));
}

//! Bicubic Catmull-Rom sample of difference in unit heights, a - b, of two heightfields of same size, clamped at edges.
float sampleCubicDifference(const Heightfield* a, const Heightfield* b, float gridX, float gridZ)
{
	const int32_t size = a->getSize();
	const int32_t ix = (int32_t)std::floor(gridX);
	const int32_t iz = (int32_t)std::floor(gridZ);

	float rows[4];
	for (int32_t j = 0; j < 4; ++j)
	{
		const int32_t z = clamp(iz + j - 1, 0, size - 1);
		float p[4];
		for (int32_t i = 0; i < 4; ++i)
		{
			const int32_t x = clamp(ix + i - 1, 0, size - 1);
			p[i] = a->getGridHeightNearestUnsafe(x, z) - b->getGridHeightNearestUnsafe(x, z);
		}
		rows[j] = catmullRom(p[0], p[1], p[2], p[3], gridX - ix);
	}
	return catmullRom(rows[0], rows[1], rows[2], rows[3], gridZ - iz);
}

}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.hf.ErosionLayer", 1, ErosionLayer, HeightfieldAssetLayer)

bool ErosionLayer::apply(const TileStorage* storage, int32_t layer, const Vector4& worldExtent, int32_t tileX, int32_t tileZ, Heightfield* inoutTile) const
{
	Ref< const Heightfield > below;
	Ref< const Heightfield > eroded;
	if (!readEroded(storage, layer, worldExtent, below, eroded))
		return false;

	const int32_t size = inoutTile->getSize();
	AlignedVector< uint16_t > mask;
	if (!storage->readStoredMask(layer, tileX, tileZ, size, mask))
	{
		mask.resize(0);
		mask.resize((size_t)size * size, 65535);
	}

	// Change of heights by erosion is upsampled onto tile where mask cover.
	for (int32_t z = 0; z < size; ++z)
	{
		for (int32_t x = 0; x < size; ++x)
		{
			const uint16_t m = mask[x + (size_t)z * size];
			if (m == 0)
				continue;

			float worldX, worldZ, gridX, gridZ;
			inoutTile->gridToWorld(x, z, worldX, worldZ);
			below->worldToGrid(worldX, worldZ, gridX, gridZ);
			inoutTile->setGridHeight(x, z, inoutTile->getGridHeightNearestUnsafe(x, z) + sampleCubicDifference(eroded, below, gridX, gridZ) * (m / 65535.0f));
		}
	}

	return true;
}

Ref< Heightfield > ErosionLayer::read(const TileStorage* storage, int32_t layer, const Vector4& worldExtent, int32_t tileX, int32_t tileZ, AlignedVector< uint16_t >* outMask) const
{
	// Erosion depends on layers below, thus layer alone is an empty tile.
	Ref< Heightfield > tile = storage->createTile(worldExtent, tileX, tileZ);
	if (!tile)
		return nullptr;

	const int32_t size = tile->getSize();
	if (outMask && !storage->readStoredMask(layer, tileX, tileZ, size, *outMask))
	{
		outMask->resize(0);
		outMask->resize((size_t)size * size, 65535);
	}

	return tile;
}

bool ErosionLayer::isGlobal() const
{
	return true;
}

bool ErosionLayer::haveMask() const
{
	return true;
}

void ErosionLayer::serialize(ISerializer& s)
{
	HeightfieldAssetLayer::serialize(s);
	s >> Member< int32_t >(L"iterations", m_parameters.iterations, AttributeRange(0));
	s >> Member< float >(L"resolution", m_resolution, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));

	if (s.getVersion< ErosionLayer >() >= 1)
	{
		s >> Member< float >(L"capacity", m_parameters.capacity, AttributeRange(0.0f));
		s >> Member< float >(L"minSlope", m_parameters.minSlope, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));
		s >> Member< float >(L"evaporation", m_parameters.evaporation, AttributeRange(0.0f, 1.0f));
		s >> Member< float >(L"erosion", m_parameters.erosion, AttributeRange(0.0f, 1.0f));
		s >> Member< float >(L"deposition", m_parameters.deposition, AttributeRange(0.0f, 1.0f));
		s >> Member< float >(L"inertia", m_parameters.inertia, AttributeRange(0.0f, 1.0f));
		s >> Member< float >(L"gravity", m_parameters.gravity, AttributeRange(0.0f));
	}
}

bool ErosionLayer::readEroded(const TileStorage* storage, int32_t layer, const Vector4& worldExtent, Ref< const Heightfield >& outBelow, Ref< const Heightfield >& outEroded) const
{
	const std::wstring key = storage->getCacheKey(layer);
	const uint64_t stamp = storage->getGlobalStamp(layer);

	Ref< Eroded > entry;
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(s_erodedLock);
		Ref< Eroded >& e = s_eroded[key];
		if (!e)
			e = new Eroded();
		entry = e;
	}

	// Layer is eroded once while other readers of layer wait.
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(entry->lock);

	if (entry->eroded && entry->stamp == stamp)
	{
		outBelow = entry->below;
		outEroded = entry->eroded;
		return true;
	}

	const int32_t fullSize = storage->readSize();
	if (fullSize <= 0)
	{
		log::error << L"Unable to erode heightfield; no data in instance." << Endl;
		return false;
	}

	const int32_t count = std::max(storage->getTileCount(), 1);
	const int32_t tileSize = fullSize / count;
	const int32_t size = (m_resolution > 0.0f) ? clamp((int32_t)(worldExtent.x() / m_resolution + 0.5f), 2, fullSize) : fullSize;

	// Nearest sample of entire heightfield for each sample along an axis, and first sample within each tile.
	AlignedVector< int32_t > nearest(size);
	for (int32_t i = 0; i < size; ++i)
		nearest[i] = std::min((int32_t)((i + 0.5) * fullSize / size), fullSize - 1);

	AlignedVector< int32_t > first(count + 1);
	for (int32_t t = 0, i = 0; t <= count; ++t)
	{
		while (i < size && nearest[i] < t * tileSize)
			++i;
		first[t] = i;
	}

	// Tiles are read on pool threads, since this is often run by a job worker which other workers wait on.
	Ref< Heightfield > below = new Heightfield(size, worldExtent, Vector4::zero());
	std::atomic< int32_t > next = 0;
	std::atomic< bool > result = true;
	const ThreadPool::threadPoolFn_t readTiles = [&]() {
		for (int32_t i = next++; i < count * count; i = next++)
		{
			const int32_t tx = i % count;
			const int32_t tz = i / count;
			if (first[tx] == first[tx + 1] || first[tz] == first[tz + 1])
				continue;

			Ref< const Heightfield > tile = storage->readMergedTile(worldExtent, tx, tz, layer);
			if (!tile)
			{
				result = false;
				continue;
			}

			for (int32_t z = first[tz]; z < first[tz + 1]; ++z)
				for (int32_t x = first[tx]; x < first[tx + 1]; ++x)
					below->setGridHeight(x, z, tile->getGridHeightNearestUnsafe(nearest[x] - tx * tileSize, nearest[z] - tz * tileSize));
		}
	};

	Thread* threads[c_maxReadThreads] = { nullptr };
	const int32_t spawnCount = std::min(clamp((int32_t)OS::getInstance().getCPUCoreCount(), 1, c_maxReadThreads), count * count) - 1;
	for (int32_t i = 0; i < spawnCount; ++i)
		ThreadPool::getInstance().spawn(readTiles, threads[i]);
	readTiles();
	for (int32_t i = 0; i < spawnCount; ++i)
		ThreadPool::getInstance().join(threads[i]);

	if (!result)
		return false;

	Ref< Heightfield > eroded = new Heightfield(size, worldExtent, Vector4::zero());
	std::memcpy(eroded->getHeights(), below->getHeights(), (size_t)size * size * sizeof(height_t));
	ErosionFilter::Parameters parameters = m_parameters;
	parameters.iterations = m_parameters.iterations * 1000;
	ErosionFilter(parameters).apply(eroded);

	entry->stamp = stamp;
	entry->below = below;
	entry->eroded = eroded;
	outBelow = below;
	outEroded = eroded;
	return true;
}

}
