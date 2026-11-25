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
#include "Core/Math/Vector4.h"
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/RefArray.h"

#include <string>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_HEIGHTFIELD_EDITOR_EXPORT)
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
class HeightfieldAssetLayer;

/*! Read and write data of a heightfield asset's layers.
 * \ingroup Heightfield
 *
 * Each channel of a layer's tile, heights, cuts, splats, colors, each attribute
 * and mask, is stored as a separate compressed instance data stream,
 * "L<id>_<channel>" or, when tiled, "L<id>_<channel>_<x>_<z>".
 * Merged reads start from base layer, then let each layer above modify
 * output of layer below.
 */
class T_DLLCLASS TileStorage : public Object
{
	T_RTTI_CLASS;

public:
	explicit TileStorage(db::Instance* instance, const HeightfieldAsset* asset);

	explicit TileStorage(const db::Instance* instance, const HeightfieldAsset* asset);

	/*! \param tileCount Number of tiles along each side, instead of asset's, 0 for a single untiled channel. */
	explicit TileStorage(db::Instance* instance, const HeightfieldAsset* asset, int32_t tileCount);

	//! Size of overview; heightfield is downsampled by largest power of two which keep samples within overview sample distance, or until within maximum overview size.
	static int32_t calculateOverviewSize(int32_t size, float worldSize);

	//! Release decoded tiles kept by region reads and invalidate global stamps.
	static void flushCache();

	/*! Number of tiles along each side; 0 if data is stored as a single, untiled, channel. */
	int32_t getTileCount() const { return m_tileCount; }

	//! Key of layer, or of merged layers if -1, unique among all storages; for caches shared by storages.
	std::wstring getCacheKey(int32_t layer) const;

	//! Stamp which change whenever layer or any layer below change, any tile of any layer is written when a layer is global, or caches are flushed.
	uint64_t getGlobalStamp(int32_t layer) const;

	//! Number of samples along each side of the whole heightfield; 0 if unable to read.
	int32_t readSize() const;

	//! Optional channels, Heightfield::Layers, stored by any tile of any layer.
	uint32_t readChannels() const;

	/*! Read merged heightfield.
	 * \param channels Optional channels, Heightfield::Layers, to read if stored; tiles without a channel get the channel's default.
	 */
	Ref< Heightfield > readHeightfield(const Vector4& worldExtent, uint32_t channels = 0) const;

	/*! Read merged tile, including one row and column of its +X/+Z neighbours and border samples on all sides, positioned in world. */
	Ref< Heightfield > readHeightfieldTile(const Vector4& worldExtent, int32_t tileX, int32_t tileZ, int32_t border, uint32_t channels = 0) const;

	//! Read whole merged heightfield box filtered into size * size samples.
	Ref< Heightfield > readHeightfieldOverview(const Vector4& worldExtent, int32_t size, uint32_t channels = 0) const;

	//! Read whole merged heightfield box filtered into overview of calculated size.
	Ref< Heightfield > readHeightfieldOverview(const Vector4& worldExtent) const;

	//! Stamp which change whenever any tile of any layer in range, inclusive tile coordinates, is written; or any tile at all if a layer is global.
	uint32_t readStamp(int32_t tileX0, int32_t tileZ0, int32_t tileX1, int32_t tileZ1) const;

	/*! Read single tile, with all channels, of layers evaluated bottom first.
	 * \param layerCount Number of layers, from bottom, which are evaluated; -1 for all.
	 */
	Ref< Heightfield > readMergedTile(const Vector4& worldExtent, int32_t tileX, int32_t tileZ, int32_t layerCount = -1) const;

	/*! Read single tile of layer alone, with all channels, as read by layer.
	 * \param outMask Optional mask of tile, one byte per sample.
	 */
	Ref< Heightfield > readLayerTile(int32_t layer, const Vector4& worldExtent, int32_t tileX, int32_t tileZ, AlignedVector< uint8_t >* outMask = nullptr) const;

	/*! Write tiles of layer in range, inclusive tile coordinates, row major; only what layer store is written, channels are encoded in parallel.
	 * \param masks Optional masks of tiles; base layer's mask isn't stored.
	 */
	bool writeLayerTiles(int32_t layer, int32_t tileX0, int32_t tileZ0, int32_t tileX1, int32_t tileZ1, const RefArray< const Heightfield >& tiles, const AlignedVector< AlignedVector< uint8_t > >* masks = nullptr) const;

	//! Read whole layer, and optionally its mask, assembled from tiles.
	Ref< Heightfield > readLayerHeightfield(int32_t layer, const Vector4& worldExtent, AlignedVector< uint8_t >* outMask = nullptr) const;

	//! Write whole layer, and optionally its mask, split into tiles.
	bool writeLayerHeightfield(int32_t layer, const Heightfield* heightfield, const AlignedVector< uint8_t >* mask = nullptr) const;

	//! Tile without data, of same size as base layer's tiles, positioned in world; heights at world zero, nothing cut.
	Ref< Heightfield > createTile(const Vector4& worldExtent, int32_t tileX, int32_t tileZ) const;

	//! Read single tile of layer as stored, positioned in world; base layer falls back on heightfield format, as stored before layers; null if missing.
	Ref< Heightfield > readStoredTile(int32_t layer, const Vector4& worldExtent, int32_t tileX, int32_t tileZ) const;

	//! Read mask of tile of layer; false if not stored.
	bool readStoredMask(int32_t layer, int32_t tileX, int32_t tileZ, int32_t size, AlignedVector< uint8_t >& outMask) const;

private:
	Ref< db::Instance > m_instance;
	RefArray< const HeightfieldAssetLayer > m_layers;
	AlignedVector< uint32_t > m_layerHashes;	//!< Hash of each layer and all layers below.
	int32_t m_tileCount;
	uint32_t m_globalStamp = 0;	//!< Stamp of every tile; only if a layer is global.

	std::wstring getChannelName(int32_t layer, const std::wstring& channel, int32_t tileX, int32_t tileZ) const;

	//! Read size and optional channels of single tile of layer.
	bool readHeader(int32_t layer, int32_t tileX, int32_t tileZ, int32_t& outSize, uint32_t& outChannels) const;

	//! Read merged tile, or tile of layer, through cache shared by all storages; cached tiles are immutable.
	Ref< const Heightfield > readCachedTile(int32_t layer, const Vector4& worldExtent, int32_t tileX, int32_t tileZ) const;

	//! Read region of merged heightfield, or of layer, of width * width samples starting at x0, z0, box filtered by step; samples outside heightfield are clamped to edge.
	Ref< Heightfield > readHeightfieldRegion(int32_t layer, const Vector4& worldExtent, int32_t tileSize, int32_t x0, int32_t z0, int32_t width, int32_t step, uint32_t channels) const;
};

}
