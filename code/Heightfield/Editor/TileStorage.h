/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Vector4.h"
#include "Core/Object.h"
#include "Core/Ref.h"

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

/*! Read and write data of a source instance, either as a single "<name>" channel or as compressed "<name>_<x>_<z>" tiles.
 * \ingroup Heightfield
 */
class T_DLLCLASS TileStorage : public Object
{
	T_RTTI_CLASS;

public:
	/*! \param tileCount Number of tiles along each side, 0 for a single untiled channel. */
	explicit TileStorage(db::Instance* instance, int32_t tileCount);

	/*! \param tileCount Number of tiles along each side, 0 for a single untiled channel. */
	explicit TileStorage(const db::Instance* instance, int32_t tileCount);

	//! Size of overview; heightfield is downsampled by largest power of two which keep samples within overview sample distance.
	static int32_t calculateOverviewSize(int32_t size, float worldSize);

	//! Release decoded tiles kept by region reads.
	static void flushCache();

	//! Number of samples along each side of the whole heightfield; 0 if unable to read.
	int32_t readSize() const;

	//! Optional layers, Heightfield::Layers, stored by any tile.
	uint32_t readLayers() const;

	/*! \param layers Optional layers, Heightfield::Layers, to read if stored; tiles without a layer get the layer's default. */
	Ref< Heightfield > readHeightfield(const Vector4& worldExtent, uint32_t layers = 0) const;

	/*! Read tile, including one row and column of its +X/+Z neighbours and border samples on all sides, positioned in world. */
	Ref< Heightfield > readHeightfieldTile(const Vector4& worldExtent, int32_t tileX, int32_t tileZ, int32_t border, uint32_t layers = 0) const;

	//! Read whole heightfield box filtered into size * size samples.
	Ref< Heightfield > readHeightfieldOverview(const Vector4& worldExtent, int32_t size, uint32_t layers = 0) const;

	//! Read whole heightfield box filtered into overview of calculated size.
	Ref< Heightfield > readHeightfieldOverview(const Vector4& worldExtent) const;

	/*! Write heightfield; only tiles intersecting region, { x0, z0, x1, z1 } inclusive grid coordinates, if specified. */
	bool writeHeightfield(const Heightfield* heightfield, const int32_t* region = nullptr) const;

	//! Stamp which change whenever any tile in range, inclusive tile coordinates, is written.
	uint32_t readStamp(const std::wstring& name, int32_t tileX0, int32_t tileZ0, int32_t tileX1, int32_t tileZ1) const;

	//! Read single tile, with all of its layers, exactly as stored; null if missing.
	Ref< Heightfield > readStoredTile(const Vector4& worldExtent, int32_t tileX, int32_t tileZ) const;

	//! Write single tile, with all of its layers, exactly as stored.
	bool writeStoredTile(int32_t tileX, int32_t tileZ, const Heightfield* tile) const;

private:
	Ref< db::Instance > m_instance;
	int32_t m_tileCount;

	std::wstring getChannelName(const std::wstring& name, int32_t tileX, int32_t tileZ) const;

	//! Read size and optional layers of single tile.
	bool readHeader(int32_t tileX, int32_t tileZ, int32_t& outSize, uint32_t& outLayers) const;

	//! Read stored tile through cache shared by all storages; cached tiles are immutable.
	Ref< const Heightfield > readCachedTile(const Vector4& worldExtent, int32_t tileX, int32_t tileZ) const;

	//! Read region, of width * width samples starting at x0, z0, box filtered by step; samples outside heightfield are clamped to edge.
	Ref< Heightfield > readHeightfieldRegion(const Vector4& worldExtent, int32_t tileSize, int32_t x0, int32_t z0, int32_t width, int32_t step, uint32_t layers) const;
};

}
