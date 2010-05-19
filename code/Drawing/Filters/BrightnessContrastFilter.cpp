#include "Drawing/Filters/BrightnessContrastFilter.h"
#include "Drawing/Image.h"

namespace traktor
{
	namespace drawing
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.BrightnessContrastFilter", BrightnessContrastFilter, IImageFilter)

BrightnessContrastFilter::BrightnessContrastFilter(float brightness, float contrast)
:	m_brightness(brightness)
,	m_contrast(contrast)
{
}

Ref< Image > BrightnessContrastFilter::apply(const Image* image)
{
	Ref< Image > final = new Image(image->getPixelFormat(), image->getWidth(), image->getHeight(), image->getPalette());
	
	Color in;
	Color c(m_contrast, m_contrast, m_contrast, 1.0f);
	Color b(m_brightness, m_brightness, m_brightness, 0.0f);

	for (int32_t y = 0; y < image->getHeight(); ++y)
	{
		for (int32_t x = 0; x < image->getWidth(); ++x)
		{
			image->getPixelUnsafe(x, y, in);
			final->setPixelUnsafe(x, y, in * c + b);
		}
	}

	return final;
}
	
	}
}
