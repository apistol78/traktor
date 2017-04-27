/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cmath>
#include "Drawing/Filters/TonemapFilter.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"

namespace traktor
{
	namespace drawing
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.TonemapFilter", TonemapFilter, IImageFilter)

void TonemapFilter::apply(Image* image) const
{
	Color4f in;

	Scalar intensity(0.0f);
	for (int32_t y = 0; y < image->getHeight(); ++y)
	{
		for (int32_t x = 0; x < image->getWidth(); ++x)
		{
			image->getPixelUnsafe(x, y, in);
			intensity += in.getRed() + in.getGreen() + in.getBlue();
		}
	}
	intensity /= Scalar(float(image->getWidth() * image->getHeight()));

	for (int32_t y = 0; y < image->getHeight(); ++y)
	{
		for (int32_t x = 0; x < image->getWidth(); ++x)
		{
			image->getPixelUnsafe(x, y, in);
			image->setPixelUnsafe(x, y, in / intensity);
		}
	}
}
	
	}
}
