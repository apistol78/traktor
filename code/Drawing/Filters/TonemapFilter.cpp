#include <cmath>
#include "Drawing/Filters/TonemapFilter.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Core/Heap/New.h"

namespace traktor
{
	namespace drawing
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.TonemapFilter", TonemapFilter, ImageFilter)

Image* TonemapFilter::apply(const Image* image)
{
	Color in;

	float intensity = 0.0f;
	for (int32_t y = 0; y < image->getHeight(); ++y)
	{
		for (int32_t x = 0; x < image->getWidth(); ++x)
		{
			image->getPixel(x, y, in);
			intensity += in.getRed() + in.getGreen() + in.getBlue();
		}
	}
	intensity /= float(image->getWidth() * image->getHeight());

	Ref< Image > final = gc_new< Image >(PixelFormat::getR8G8B8(), image->getWidth(), image->getHeight(), image->getPalette());

	for (int32_t y = 0; y < image->getHeight(); ++y)
	{
		for (int32_t x = 0; x < image->getWidth(); ++x)
		{
			image->getPixel(x, y, in);
			final->setPixel(x, y, in / intensity);
		}
	}

	return final;
}
	
	}
}
