/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Drawing/Filters/BrightnessContrastFilter.h"
#include "Drawing/Image.h"

namespace traktor::drawing
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.BrightnessContrastFilter", BrightnessContrastFilter, IImageFilter)

BrightnessContrastFilter::BrightnessContrastFilter(float brightness, float contrast)
:	m_brightness(brightness)
,	m_contrast(contrast)
{
}

void BrightnessContrastFilter::apply(Image* image) const
{
	Color4f in;
	Color4f c(m_contrast, m_contrast, m_contrast, 1.0f);
	Color4f b(m_brightness, m_brightness, m_brightness, 0.0f);

	for (int32_t y = 0; y < image->getHeight(); ++y)
	{
		for (int32_t x = 0; x < image->getWidth(); ++x)
		{
			image->getPixelUnsafe(x, y, in);
			image->setPixelUnsafe(x, y, in * c + b);
		}
	}
}

}
