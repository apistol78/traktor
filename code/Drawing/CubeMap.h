/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Core/Math/Color4f.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DRAWING_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::drawing
{

class Image;
class PixelFormat;

/*! CubeMap wrapper.
 * \ingroup Drawing
 */
class T_DLLCLASS CubeMap : public Object
{
	T_RTTI_CLASS;

public:
	explicit CubeMap(int32_t size, const drawing::PixelFormat& pixelFormat);

	explicit CubeMap(const Ref< drawing::Image > sides[6]);

	/*! Create cubemap from "cross" image. */
	static Ref< CubeMap > createFromCrossImage(const drawing::Image* image);

	/*! Create cubemap from "equirectangular" image. */
	static Ref< CubeMap > createFromEquirectangularImage(const drawing::Image* image);

	/*! Create cubemap from image, trying to be clever about rearrangement. */
	static Ref< CubeMap > createFromImage(const drawing::Image* image);

	/*! Create flatten "cross" image. */
	Ref< drawing::Image > createCrossImage() const;

	/*! Create equirectangular image. */
	Ref< drawing::Image > createEquirectangular() const;

	/*! Get normalized direction vector from coordinates. */
	Vector4 getDirection(int32_t side, int32_t x, int32_t y) const;

	/*! Get coordinates from direction. */
	void getPosition(const Vector4& direction, int32_t& outSide, int32_t& outX, int32_t& outY) const;

	/*! Set pixel value at direction. */
	void set(const Vector4& direction, const Color4f& value);

	/*! Get pixel value from direction. */
	Color4f get(const Vector4& direction) const;

	/*! Replace image of one side. */
	void setSide(int32_t side, drawing::Image* image) { m_side[side] = image; }

	/*! Get image of one side. */
	drawing::Image* getSide(int32_t side) { return m_side[side]; }

	/*! Get image of one side. */
	const drawing::Image* getSide(int32_t side) const { return m_side[side]; }

	/*! Get size of cubemap. */
	int32_t getSize() const { return m_size; }

private:
	Ref< drawing::Image > m_side[6];
	int32_t m_size;
};

}
