/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Drawing/Filters/GrayscaleFilter.h"
#include "Drawing/Image.h"

namespace traktor
{
	namespace drawing
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.GrayScaleFilter", GrayscaleFilter, IImageFilter)

void GrayscaleFilter::apply(Image* image) const
{
	Color4f in;
	for (int32_t y = 0; y < image->getHeight(); ++y)
	{
		for (int32_t x = 0; x < image->getWidth(); ++x)
		{
			image->getPixelUnsafe(x, y, in);
			float luminance = 0.2126f * in.getRed() + 0.7152f * in.getGreen() + 0.0722f * in.getBlue();
			Color4f out(luminance, luminance, luminance, in.getAlpha());
			image->setPixelUnsafe(x, y, out);
		}
	}
}

	}
}
