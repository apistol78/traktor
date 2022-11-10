/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Drawing/IImageFilter.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DRAWING_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::drawing
{

/*! Scale image filter.
 * \ingroup Drawing
 *
 * Magnify or minify image, either using point sampling or
 * linear filtering.
 */
class T_DLLCLASS ScaleFilter : public IImageFilter
{
	T_RTTI_CLASS;

public:
	// Scale method when image is scaled down.
	enum MinifyType
	{
		MnCenter,	// Center source pixel.
		MnAverage	// Average pixels from source rectangle.
	};

	// Scale method when image is scaled up.
	enum MagnifyType
	{
		MgNearest,	// Nearest source pixel.
		MgLinear	// Linear interpolate source pixels.
	};

	explicit ScaleFilter(
		int32_t width,
		int32_t height,
		MinifyType minify,
		MagnifyType magnify,
		bool keepZeroAlpha = false
	);

protected:
	virtual void apply(Image* image) const override final;

private:
	int32_t m_width;
	int32_t m_height;
	MinifyType m_minify;
	MagnifyType m_magnify;
	bool m_keepZeroAlpha;
};

}
