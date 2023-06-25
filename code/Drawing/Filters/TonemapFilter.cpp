/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cmath>
#include "Drawing/Filters/TonemapFilter.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"

namespace traktor::drawing
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.TonemapFilter", TonemapFilter, IImageFilter)

void TonemapFilter::apply(Image* image) const
{
	Color4f in;

	Scalar intensity = 0.0_simd;
	for (int32_t y = 0; y < image->getHeight(); ++y)
	{
		for (int32_t x = 0; x < image->getWidth(); ++x)
		{
			image->getPixelUnsafe(x, y, in);
			intensity += (in.getRed() + in.getGreen() + in.getBlue()) / 3.0_simd;
		}
	}
	intensity /= Scalar(float(image->getWidth() * image->getHeight()));

	for (int32_t y = 0; y < image->getHeight(); ++y)
	{
		for (int32_t x = 0; x < image->getWidth(); ++x)
		{
			image->getPixelUnsafe(x, y, in);
			image->setPixelUnsafe(x, y, (in / intensity).rgb1());
		}
	}
}

}
