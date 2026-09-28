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

/*! Compose trim sheet layers from source images, which are cached and reloaded if modified; not thread safe.
 * \ingroup Render
 */
class T_DLLCLASS TrimSheetComposer : public Object
{
	T_RTTI_CLASS;

public:
	/*! Relative source image paths are resolved against assetPath; parallel composes using jobs, must be false within a job. */
	explicit TrimSheetComposer(const Path& assetPath, bool parallel = true);

	/*! Compose layer into an RGBA F32 image of sheet size; albedo is sRGB, other layers linear. */
	Ref< drawing::Image > compose(const TrimSheetSetupAsset* setup, TrimSheetLayer layer);

	/*! Compose layer and scale result to thumbnail size. */
	Ref< drawing::Image > composeThumbnail(const TrimSheetSetupAsset* setup, TrimSheetLayer layer, int32_t width, int32_t height);

	/*! Compose a single region of a layer, including margins; same pixels as region's part of composed layer. */
	Ref< drawing::Image > composeRegion(const TrimSheetSetupAsset* setup, TrimSheetLayer layer, const TrimSheetSetupAsset::RegionLayout& regionLayout);

	/*! Get size of region's image in a layer after scale and rotation; false if there is no image or it cannot be loaded. */
	bool getPlacedSize(const TrimSheetRegion* region, TrimSheetLayer layer, int32_t& outWidth, int32_t& outHeight);

	/*! Release all cached source images. */
	void flush();

	/*! Collect all source image files used by a layer, as specified in setup. */
	static void collectFiles(const TrimSheetSetupAsset* setup, TrimSheetLayer layer, std::set< std::wstring >& outFiles);

	/*! Get key identifying everything in setup which composition of a layer depends on. */
	static std::wstring getLayerKey(const TrimSheetSetupAsset* setup, TrimSheetLayer layer);

private:
	struct Source
	{
		uint64_t lastWriteTime = 0;
		uint64_t size = 0;
		bool missing = false;
		Ref< drawing::Image > image;
	};

	Path m_assetPath;
	bool m_parallel;
	std::map< std::wstring, Source > m_sources;
	std::set< std::wstring > m_invalidSwizzles;

	const drawing::Image* getSource(const Path& fileName, const std::wstring& swizzle, float scale, TrimSheetLayer layer);

	void placeRegion(const TrimSheetSetupAsset* setup, TrimSheetLayer layer, const TrimSheetSetupAsset::RegionLayout& regionLayout, drawing::Image* sheet);
};

}
