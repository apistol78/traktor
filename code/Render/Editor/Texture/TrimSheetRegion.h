/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Io/Path.h"
#include "Core/Serialization/ISerializable.h"

#include <string>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

/*! Texture layers of a trim sheet.
 * \ingroup Render
 */
enum class TrimSheetLayer : int32_t
{
	Albedo = 0,
	Specular = 1,
	Roughness = 2,
	Metallic = 3,
	Normal = 4,
	Height = 5
};

constexpr int32_t TrimSheetLayerCount = 6;

/*! Trim sheet region.
 * \ingroup Render
 *
 * A region is a split of a slab, it spans the entire thickness
 * of the slab. It has one source image per layer, all layers'
 * images are placed using the same placement.
 *
 * An image's channels are first swizzled, it's then scaled, rotated
 * clockwise, and finally translated so its top-left corner lands at
 * the offset relative to the top-left corner of the region's content.
 * Tiling repeats the placed image, in sheet axes, to fill the content.
 * Anything outside of the content is clipped.
 *
 * An optional margin is added across the slab's thickness, above and
 * below, or left and right in a vertical slab, of the region's content.
 * It's filled with repeated edge pixels of the content so bilinear
 * filtering at the region's edges doesn't bleed from neighbours. The
 * margin doesn't take space from the content; the slab is made thicker
 * instead. Content of all regions in a slab are aligned, thus the slab
 * is made thicker by the largest margin of its regions.
 */
class T_DLLCLASS TrimSheetRegion : public ISerializable
{
	T_RTTI_CLASS;

public:
	/*! Source image of a layer. */
	struct Image
	{
		Path fileName;

		/*! Source channel of each output channel, same as TextureOutput; "r", "g", "b", "a", "0" or "1" per channel. */
		std::wstring swizzle = L"rgba";

		void serialize(ISerializer& s);
	};

	enum class Rotation
	{
		Deg0,
		Deg90,
		Deg180,
		Deg270
	};

	enum class Tiling
	{
		None,
		Horizontal,
		Vertical,
		Both
	};

	TrimSheetRegion() = default;

	explicit TrimSheetRegion(int32_t size);

	void setName(const std::wstring& name) { m_name = name; }

	const std::wstring& getName() const { return m_name; }

	/*! Length, in pixels, along the slab; 0 means share of remaining length. */
	void setSize(int32_t size) { m_size = size; }

	int32_t getSize() const { return m_size; }

	/*! Margin, in pixels, on both sides across slab's thickness; 0 means no margin. */
	void setMargin(int32_t margin) { m_margin = margin; }

	int32_t getMargin() const { return m_margin; }

	/*! Source image file of layer; empty if region has no image in layer. */
	void setFileName(TrimSheetLayer layer, const Path& fileName) { m_images[(int32_t)layer].fileName = fileName; }

	const Path& getFileName(TrimSheetLayer layer) const { return m_images[(int32_t)layer].fileName; }

	/*! Swizzle of source image's channels in layer. */
	void setSwizzle(TrimSheetLayer layer, const std::wstring& swizzle) { m_images[(int32_t)layer].swizzle = swizzle; }

	const std::wstring& getSwizzle(TrimSheetLayer layer) const { return m_images[(int32_t)layer].swizzle; }

	/*! True if region has an image in any layer. */
	bool hasImage() const;

	/*! Clockwise rotation. */
	void setRotation(Rotation rotation) { m_rotation = rotation; }

	Rotation getRotation() const { return m_rotation; }

	/*! Offset, in sheet pixels, of image's top-left corner relative to region's top-left corner. */
	void setOffset(int32_t offsetX, int32_t offsetY)
	{
		m_offsetX = offsetX;
		m_offsetY = offsetY;
	}

	int32_t getOffsetX() const { return m_offsetX; }

	int32_t getOffsetY() const { return m_offsetY; }

	void setScale(float scale) { m_scale = scale; }

	float getScale() const { return m_scale; }

	void setTiling(Tiling tiling) { m_tiling = tiling; }

	Tiling getTiling() const { return m_tiling; }

	/*! True if placed image is repeated horizontally. */
	bool tileHorizontal() const { return m_tiling == Tiling::Horizontal || m_tiling == Tiling::Both; }

	/*! True if placed image is repeated vertically. */
	bool tileVertical() const { return m_tiling == Tiling::Vertical || m_tiling == Tiling::Both; }

	/*! True if rotation swaps width and height of the image. */
	bool rotationSwapsAxes() const { return m_rotation == Rotation::Deg90 || m_rotation == Rotation::Deg270; }

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_name;
	int32_t m_size = 0;
	int32_t m_margin = 0;
	Image m_images[TrimSheetLayerCount];
	Rotation m_rotation = Rotation::Deg0;
	int32_t m_offsetX = 0;
	int32_t m_offsetY = 0;
	float m_scale = 1.0f;
	Tiling m_tiling = Tiling::None;
};

}
