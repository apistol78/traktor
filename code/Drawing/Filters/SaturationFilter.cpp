/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Drawing/Filters/SaturationFilter.h"

#include "Drawing/Image.h"

namespace traktor::drawing
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.SaturationFilter", SaturationFilter, IImageFilter)

SaturationFilter::SaturationFilter(float saturation)
	: m_saturation(saturation)
{
}

void SaturationFilter::apply(Image* image) const
{
	Color4f in;
	for (int32_t y = 0; y < image->getHeight(); ++y)
	{
		for (int32_t x = 0; x < image->getWidth(); ++x)
		{
			image->getPixelUnsafe(x, y, in);

			const float grey = in.getRed() * 0.2126f + in.getGreen() * 0.7152f + in.getBlue() * 0.0722f;
			const Color4f bl = lerp(
				Color4f(grey, grey, grey, in.getAlpha()),
				in,
				Scalar(m_saturation));

			image->setPixelUnsafe(x, y, bl);
		}
	}
}

}
