/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Heightfield/Editor/EditLayer.h"

#include "Core/Log/Log.h"
#include "Core/Math/MathUtils.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Thread/Job.h"
#include "Core/Thread/JobManager.h"
#include "Heightfield/Editor/TileStorage.h"
#include "Heightfield/Heightfield.h"

#include <algorithm>
#include <atomic>

namespace traktor::hf
{
namespace
{

//! Blend values of layer onto merged by mask.
void blendValues(uint8_t* merged, const uint8_t* layer, const uint16_t* mask, int32_t samples, int32_t components)
{
	for (int32_t i = 0; i < samples; ++i)
	{
		const uint32_t m = mask[i];
		if (m == 0)
			continue;

		for (int32_t c = 0; c < components; ++c)
		{
			const size_t j = (size_t)i * components + c;
			merged[j] = (uint8_t)((merged[j] * (65535 - m) + layer[j] * m + 32767) / 65535);
		}
	}
}

}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.hf.EditLayer", 1, EditLayer, HeightfieldAssetLayer)

bool EditLayer::apply(const TileStorage* storage, int32_t layer, const Vector4& worldExtent, int32_t tileX, int32_t tileZ, Heightfield* inoutTile) const
{
	const int32_t size = inoutTile->getSize();
	const int32_t samples = size * size;

	// Tile without mask doesn't contribute.
	AlignedVector< uint16_t > mask;
	if (!storage->readStoredMask(layer, tileX, tileZ, size, mask))
		return true;

	Ref< const Heightfield > tile = storage->readStoredTile(layer, worldExtent, tileX, tileZ);
	if (!tile || tile->getSize() != size)
	{
		log::error << L"Unable to read heightfield; tile " << tileX << L", " << tileZ << L" of layer " << layer << L" missing or size mismatch." << Endl;
		return false;
	}

	// Optional channels and attributes which layer doesn't have are kept.
	height_t* heights = inoutTile->getHeights();
	const height_t* layerHeights = tile->getHeights();
	for (int32_t i = 0; i < samples; ++i)
	{
		const uint64_t m = mask[i];
		if (m == 0)
			continue;

		// Delta is added onto layer below by mask, otherwise heights are blended by mask.
		if (m_delta)
			heights[i] = (height_t)clamp< int64_t >(((int64_t)heights[i] * 65535 + ((int64_t)layerHeights[i] - ZeroDelta) * (int64_t)m + 32767) / 65535, 0, 65535);
		else
			heights[i] = (height_t)((heights[i] * (65535 - m) + layerHeights[i] * m + 32767) / 65535);

		if (m >= 32768)
			inoutTile->setGridCut(i % size, i / size, tile->getGridCut(i % size, i / size));
	}

	for (uint8_t a = 0; a < Heightfield::MaxAttributes; ++a)
		if (const uint8_t* attributes = tile->getAttributes(a))
			blendValues(inoutTile->createAttributes(a), attributes, mask.c_ptr(), samples, 1);

	if (const uint8_t* splats = tile->getSplats())
		blendValues(inoutTile->createSplats(), splats, mask.c_ptr(), samples, 4);

	if (const uint8_t* colors = tile->getColors())
		blendValues(inoutTile->createColors(), colors, mask.c_ptr(), samples, 4);

	return true;
}

Ref< Heightfield > EditLayer::read(const TileStorage* storage, int32_t layer, const Vector4& worldExtent, int32_t tileX, int32_t tileZ, AlignedVector< uint16_t >* outMask) const
{
	// Tiles above base without data are empty.
	Ref< Heightfield > tile = storage->readStoredTile(layer, worldExtent, tileX, tileZ);
	if (!tile && layer > 0)
		tile = storage->createTile(worldExtent, tileX, tileZ);
	if (!tile)
	{
		log::error << L"Unable to read heightfield; no tile " << tileX << L", " << tileZ << L" of layer " << layer << L" in instance." << Endl;
		return nullptr;
	}

	// Base layer cover everything, layers above nothing unless masked.
	if (outMask)
	{
		const int32_t size = tile->getSize();
		if (layer == 0 || !storage->readStoredMask(layer, tileX, tileZ, size, *outMask))
		{
			outMask->resize(0);
			outMask->resize((size_t)size * size, (layer == 0) ? 65535 : 0);
		}
	}

	return tile;
}

bool EditLayer::haveData() const
{
	return true;
}

bool EditLayer::haveMask() const
{
	return true;
}

void EditLayer::serialize(ISerializer& s)
{
	HeightfieldAssetLayer::serialize(s);

	if (s.getVersion< EditLayer >() >= 1)
		s >> Member< bool >(L"delta", m_delta);
}

bool EditLayer::convertHeights(const TileStorage* storage, int32_t layer, const Vector4& worldExtent, bool delta) const
{
	// Delta of base layer is relative to empty tile, thus same as its heights.
	if (layer <= 0)
		return true;

	const int32_t count = std::max(storage->getTileCount(), 1);
	for (int32_t tz = 0; tz < count; ++tz)
	{
		// Stored tiles of a row are converted in parallel, then written.
		RefArray< Heightfield > tiles(count);
		AlignedVector< AlignedVector< uint16_t > > masks(count);
		std::atomic< bool > failed = false;
		AlignedVector< Job::task_t > tasks;
		for (int32_t tx = 0; tx < count; ++tx)
			tasks.push_back([&, tx]() {
				Ref< Heightfield > tile = storage->readStoredTile(layer, worldExtent, tx, tz);
				if (!tile)
					return;

				Ref< const Heightfield > below = storage->readMergedTile(worldExtent, tx, tz, layer);
				if (!below || below->getSize() != tile->getSize())
				{
					failed = true;
					return;
				}

				const int32_t samples = tile->getSize() * tile->getSize();
				height_t* heights = tile->getHeights();
				const height_t* belowHeights = below->getHeights();
				for (int32_t i = 0; i < samples; ++i)
					heights[i] = (height_t)clamp(delta ? heights[i] - belowHeights[i] + ZeroDelta : belowHeights[i] + heights[i] - ZeroDelta, 0, 65535);

				storage->readStoredMask(layer, tx, tz, tile->getSize(), masks[tx]);
				tiles[tx] = tile;
			});
		JobManager::getInstance().fork(tasks.c_ptr(), tasks.size());

		if (failed)
		{
			log::error << L"Unable to convert heights; layers below tiles of row " << tz << L" missing or size mismatch." << Endl;
			return false;
		}

		for (int32_t tx = 0; tx < count; ++tx)
		{
			if (!tiles[tx])
				continue;

			RefArray< const Heightfield > written;
			written.push_back(tiles[tx]);
			const AlignedVector< AlignedVector< uint16_t > > writtenMasks(1, masks[tx]);
			if (!storage->writeLayerTiles(layer, tx, tz, tx, tz, written, masks[tx].empty() ? nullptr : &writtenMasks))
				return false;
		}
	}

	return true;
}

}
