/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
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

/*! Reduce an image by taking the maximum of each source block.
 * \ingroup Drawing
 *
 * A mip chain which is used to accelerate ray marching a height field - quad
 * tree displacement mapping - is not an image but a bounding hierarchy; each
 * texel must bound the texels it covers, so the reduction has to be a maximum
 * and not an average. Averaging lets the ray step past thin ridges since the
 * coarse levels then sit below the surface they are supposed to be above.
 */
class T_DLLCLASS MaxReduceFilter : public IImageFilter
{
	T_RTTI_CLASS;

public:
	explicit MaxReduceFilter(int32_t width, int32_t height)
	:	m_width(width)
	,	m_height(height)
	{
	}

protected:
	virtual void apply(Image* image) const override final;

private:
	int32_t m_width;
	int32_t m_height;
};

}
