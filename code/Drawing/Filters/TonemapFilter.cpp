#include <cmath>
#include "Drawing/Filters/TonemapFilter.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"

namespace traktor
{
	namespace drawing
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.TonemapFilter", TonemapFilter, IImageFilter)

Ref< Image > TonemapFilter::apply(const Image* image)
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

	Ref< Image > final = new Image(PixelFormat::getR8G8B8(), image->getWidth(), image->getHeight(), image->getPalette());

	for (int32_t y = 0; y < image->getHeight(); ++y)
	{
		for (int32_t x = 0; x < image->getWidth(); ++x)
		{
			image->getPixelUnsafe(x, y, in);
			final->setPixelUnsafe(x, y, in / intensity);
		}
	}

	return final;
}
	
	}
}
