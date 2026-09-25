/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Color4f.h"
#include "Core/Serialization/ISerializable.h"
#include "Render/Editor/Texture/TrimSheetRegion.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class TrimSheetSlab;

/*! Rectangle in trim sheet pixels.
 * \ingroup Render
 */
struct TrimSheetRect
{
	int32_t x = 0;
	int32_t y = 0;
	int32_t width = 0;
	int32_t height = 0;

	bool empty() const { return width <= 0 || height <= 0; }

	bool inside(int32_t px, int32_t py) const { return px >= x && py >= y && px < x + width && py < y + height; }
};

/*! Trim sheet setup.
 * \ingroup Render
 *
 * Describe layout of trim sheet regions and which source images
 * are placed in each region, for all layers. Each layer is built
 * into a runtime texture through a TrimSheetTextureAsset.
 *
 * Slabs are laid out in order; each slab is taken from the sheet
 * area remaining after the previous slabs.
 */
class T_DLLCLASS TrimSheetSetupAsset : public ISerializable
{
	T_RTTI_CLASS;

public:
	/*! Convention of green channel in source normal maps. */
	enum class NormalConvention
	{
		OpenGL,	//!< Green points up in image, "Y+".
		DirectX	//!< Green points down in image, "Y-".
	};

	/*! Layout of a region, in sheet pixels. */
	struct RegionLayout
	{
		int32_t slab = -1;
		int32_t region = -1;
		TrimSheetRect rect;
	};

	TrimSheetSetupAsset();

	void setWidth(int32_t width) { m_width = width; }

	int32_t getWidth() const { return m_width; }

	void setHeight(int32_t height) { m_height = height; }

	int32_t getHeight() const { return m_height; }

	void setNormalConvention(NormalConvention normalConvention) { m_normalConvention = normalConvention; }

	NormalConvention getNormalConvention() const { return m_normalConvention; }

	/*! Color of sheet where no image is placed. */
	void setBackground(TrimSheetLayer layer, const Color4f& background) { m_backgrounds[(int32_t)layer] = background; }

	const Color4f& getBackground(TrimSheetLayer layer) const { return m_backgrounds[(int32_t)layer]; }

	void insertSlab(int32_t index, TrimSheetSlab* slab);

	void removeSlab(int32_t index);

	RefArray< TrimSheetSlab >& getSlabs() { return m_slabs; }

	const RefArray< TrimSheetSlab >& getSlabs() const { return m_slabs; }

	/*! Get region, null if indices out of range. */
	TrimSheetRegion* getRegion(int32_t slab, int32_t region) const;

	/*! Calculate rectangles of all slabs and regions.
	 *
	 * \param outSlabs Rectangle of each slab, same order as slabs.
	 * \param outRegions Layout of each region, in slab order.
	 */
	void calculateLayout(AlignedVector< TrimSheetRect >& outSlabs, AlignedVector< RegionLayout >& outRegions) const;

	virtual void serialize(ISerializer& s) override final;

private:
	int32_t m_width = 2048;
	int32_t m_height = 2048;
	NormalConvention m_normalConvention = NormalConvention::OpenGL;
	Color4f m_backgrounds[TrimSheetLayerCount];
	RefArray< TrimSheetSlab > m_slabs;
};

}
