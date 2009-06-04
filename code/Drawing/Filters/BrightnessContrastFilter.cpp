#include "Drawing/Filters/BrightnessContrastFilter.h"
#include "Drawing/Image.h"
#include "Core/Heap/New.h"

namespace traktor
{
	namespace drawing
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.BrightnessContrastFilter", BrightnessContrastFilter, ImageFilter)

BrightnessContrastFilter::BrightnessContrastFilter(float brightness, float contrast)
:	m_brightness(brightness)
,	m_contrast(contrast)
{
}

Image* BrightnessContrastFilter::apply(const Image* image)
{
	Ref< Image > final = gc_new< Image >(image->getPixelFormat(), image->getWidth(), image->getHeight(), image->getPalette());
	Color in;
	for (uint32_t y = 0; y < image->getHeight(); ++y)
	{
		for (uint32_t x = 0; x < image->getWidth(); ++x)
		{
			image->getPixel(x, y, in);
			final->setPixel(x, y, in * m_contrast + Color(m_brightness, m_brightness, m_brightness, m_brightness));
		}
	}
	return final;
}
	
	}
}
