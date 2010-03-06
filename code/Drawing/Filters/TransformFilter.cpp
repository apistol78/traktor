#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Drawing/Filters/TransformFilter.h"

namespace traktor
{
	namespace drawing
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.TransformFilter", TransformFilter, IImageFilter)

TransformFilter::TransformFilter(const Color& Km, const Color& Kc)
:	m_Km(Km)
,	m_Kc(Kc)
{
}

Ref< Image > TransformFilter::apply(const Image* image)
{
	Color in;

	Ref< Image > final = image->clone(false);
	for (int32_t y = 0; y < image->getHeight(); ++y)
	{
		for (int32_t x = 0; x < image->getWidth(); ++x)
		{
			image->getPixel(x, y, in);
			in * m_Km + m_Kc;
			final->setPixel(x, y, in);
		}
	}

	return final;
}
	
	}
}
