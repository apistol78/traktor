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

namespace traktor::drawing
{

class Image;

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
	explicit TileStorage(int32_t tileCount);

	//! Size of overview; heightfield is downsampled by largest power of two which keep samples within overview sample distance.
	static int32_t calculateOverviewSize(int32_t size, float worldSize);

	//! Number of samples along each side of the whole heightfield; 0 if unable to read.
	int32_t readSize(const db::Instance* instance) const;

	Ref< Heightfield > readHeightfield(const db::Instance* instance, const Vector4& worldExtent) const;

	/*! Read tile, including one row and column of its +X/+Z neighbours and border samples on all sides, positioned in world. */
	Ref< Heightfield > readHeightfieldTile(const db::Instance* instance, const Vector4& worldExtent, int32_t tileX, int32_t tileZ, int32_t border) const;

	//! Read whole heightfield box filtered into size * size samples.
	Ref< Heightfield > readHeightfieldOverview(const db::Instance* instance, const Vector4& worldExtent, int32_t size) const;

	//! Read whole heightfield box filtered into overview of calculated size.
	Ref< Heightfield > readHeightfieldOverview(const db::Instance* instance, const Vector4& worldExtent) const;

	/*! Write heightfield; only tiles intersecting region, { x0, z0, x1, z1 } inclusive grid coordinates, if specified. */
	bool writeHeightfield(db::Instance* instance, const Heightfield* heightfield, const int32_t* region = nullptr) const;

	/*! Read image; null if instance has no such image. */
	Ref< drawing::Image > readImage(const db::Instance* instance, const std::wstring& name) const;

	//! Read image tile, including one row and column of its +X/+Z neighbours; null if instance has no such image.
	Ref< drawing::Image > readImageTile(const db::Instance* instance, const std::wstring& name, int32_t tileX, int32_t tileZ) const;

	//! Read whole image box filtered into size * size pixels; null if instance has no such image.
	Ref< drawing::Image > readImageOverview(const db::Instance* instance, const std::wstring& name, int32_t size) const;

	/*! Write image; only tiles intersecting region, { x0, y0, x1, y1 } inclusive pixel coordinates, if specified. */
	bool writeImage(db::Instance* instance, const std::wstring& name, const drawing::Image* image, const int32_t* region = nullptr) const;

	//! Stamp which change whenever any tile in range, inclusive tile coordinates, is written.
	uint32_t readStamp(const db::Instance* instance, const std::wstring& name, int32_t tileX0, int32_t tileZ0, int32_t tileX1, int32_t tileZ1) const;

	//! True if instance contain data, such as an image, of given name.
	bool haveData(const db::Instance* instance, const std::wstring& name) const;

	//! Read single tile exactly as stored; null if missing.
	Ref< Heightfield > readStoredTile(const db::Instance* instance, const Vector4& worldExtent, int32_t tileX, int32_t tileZ) const;

	//! Write single tile exactly as stored.
	bool writeStoredTile(db::Instance* instance, int32_t tileX, int32_t tileZ, const Heightfield* tile) const;

	//! Read single image tile exactly as stored; null if instance has no such image.
	Ref< drawing::Image > readStoredImageTile(const db::Instance* instance, const std::wstring& name, int32_t tileX, int32_t tileZ) const;

	//! Write single image tile exactly as stored.
	bool writeStoredImageTile(db::Instance* instance, const std::wstring& name, int32_t tileX, int32_t tileZ, const drawing::Image* tile) const;

private:
	int32_t m_tileCount;

	std::wstring getChannelName(const std::wstring& name, int32_t tileX, int32_t tileZ) const;

	//! Read region, of width * width samples starting at x0, z0, box filtered by step; samples outside heightfield are clamped to edge.
	Ref< Heightfield > readHeightfieldRegion(const db::Instance* instance, const Vector4& worldExtent, int32_t tileSize, int32_t x0, int32_t z0, int32_t width, int32_t step) const;

	//! Read region, of width * height pixels starting at x0, y0, box filtered by step; pixels outside image are clamped to edge.
	Ref< drawing::Image > readImageRegion(const db::Instance* instance, const std::wstring& name, const drawing::Image* anchor, int32_t x0, int32_t y0, int32_t width, int32_t height, int32_t stepX, int32_t stepY) const;
};

}
