#include "Drawing/Filters/GrayscaleFilter.h"
#include "Drawing/Image.h"
#include "Core/Heap/New.h"

namespace traktor
{
	namespace drawing
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.GrayScaleFilter", GrayscaleFilter, ImageFilter)

Image* GrayscaleFilter::apply(const Image* image)
{
	Ref< Image > final = gc_new< Image >(image->getPixelFormat(), image->getWidth(), image->getHeight(), image->getPalette());
	Color in;
	for (uint32_t y = 0; y < image->getHeight(); ++y)
	{
		for (uint32_t x = 0; x < image->getWidth(); ++x)
		{
			image->getPixel(x, y, in);
			float luminance = 0.2126f * in.getRed() + 0.7152f * in.getGreen() + 0.0722f * in.getBlue();
			Color out(luminance, luminance, luminance, luminance);
			final->setPixel(x, y, out);
		}
	}
	return final;
}

	}
}
