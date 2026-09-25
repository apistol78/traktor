/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Io/Path.h"
#include "Render/Editor/Texture/TrimSheetSetupAsset.h"

#include <map>
#include <set>
#include <string>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::drawing
{

class Image;

}

namespace traktor::render
{

/*! Compose trim sheet layers from source images.
 * \ingroup Render
 *
 * Source images are cached, and reloaded if modified, so keeping the
 * composer alive is beneficial when composing repeatedly. Not thread safe.
 */
class T_DLLCLASS TrimSheetComposer : public Object
{
	T_RTTI_CLASS;

public:
	/*!
	 * \param assetPath Path which relative source image paths are relative to.
	 */
	explicit TrimSheetComposer(const Path& assetPath);

	/*! Compose layer.
	 *
	 * \return RGBA F32 image, size of the sheet; albedo is sRGB, other layers linear.
	 */
	Ref< drawing::Image > compose(const TrimSheetSetupAsset* setup, TrimSheetLayer layer);

	/*! Compose layer and scale result to thumbnail size. */
	Ref< drawing::Image > composeThumbnail(const TrimSheetSetupAsset* setup, TrimSheetLayer layer, int32_t width, int32_t height);

	/*! Recompose a single region of a layer.
	 *
	 * \param sheet Image previously composed from same setup and layer.
	 */
	void composeRegion(const TrimSheetSetupAsset* setup, TrimSheetLayer layer, const TrimSheetSetupAsset::RegionLayout& regionLayout, drawing::Image* sheet);

	/*! Get size of region's image in a layer as placed in sheet, i.e. after scale and rotation.
	 *
	 * \return False if region has no image in layer or if image cannot be loaded.
	 */
	bool getPlacedSize(const TrimSheetRegion* region, TrimSheetLayer layer, int32_t& outWidth, int32_t& outHeight);

	/*! Release all cached source images. */
	void flush();

	/*! Collect all source image files used by a layer, as specified in setup. */
	static void collectFiles(const TrimSheetSetupAsset* setup, TrimSheetLayer layer, std::set< std::wstring >& outFiles);

private:
	struct Source
	{
		uint64_t lastWriteTime = 0;
		uint64_t size = 0;
		bool missing = false;
		Ref< drawing::Image > image;
	};

	Path m_assetPath;
	std::map< std::wstring, Source > m_sources;

	const drawing::Image* getSource(const Path& fileName, float scale, TrimSheetLayer layer);

	void placeRegion(const TrimSheetSetupAsset* setup, TrimSheetLayer layer, const TrimSheetSetupAsset::RegionLayout& regionLayout, drawing::Image* sheet);
};

}
