#include "Drawing/Image.h"
#include "Drawing/Filters/ConvolutionFilter.h"
#include "Drawing/Filters/SharpenFilter.h"

namespace traktor
{
	namespace drawing
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.SharpenFilter", SharpenFilter, IImageFilter)

SharpenFilter::SharpenFilter(int radius, float strength)
:	m_radius(radius)
,	m_strength(strength)
{
}

void SharpenFilter::apply(Image* image) const
{
	Ref< Image > unsharpenMask = image->clone();

	switch (m_radius)
	{
	case 3:
		unsharpenMask->apply(ConvolutionFilter::createGaussianBlur3());
		break;

	case 5:
		unsharpenMask->apply(ConvolutionFilter::createGaussianBlur5());
		break;

	default:
		break;
	}

	Color4f in, inMask;

	for (int32_t y = 0; y < image->getHeight(); ++y)
	{
		for (int32_t x = 0; x < image->getWidth(); ++x)
		{
			image->getPixelUnsafe(x, y, in);
			unsharpenMask->getPixelUnsafe(x, y, inMask);

			in.setRed  (in.getRed()   + in.getRed()   - inMask.getRed());
			in.setGreen(in.getGreen() + in.getGreen() - inMask.getGreen());
			in.setBlue (in.getBlue()  + in.getBlue()  - inMask.getBlue());

			image->setPixelUnsafe(x, y, in);
		}
	}
}

	}
}
