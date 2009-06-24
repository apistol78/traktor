#include <cmath>
#include "Drawing/Filters/QuantizeFilter.h"
#include "Drawing/Image.h"
#include "Core/Heap/New.h"

namespace traktor
{
	namespace drawing
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.QuantizeFilter", QuantizeFilter, ImageFilter)

QuantizeFilter::QuantizeFilter(int steps)
:	m_steps(steps)
{
}

Image* QuantizeFilter::apply(const Image* image)
{
	Ref< Image > final = gc_new< Image >(image->getPixelFormat(), image->getWidth(), image->getHeight(), image->getPalette());
	
	Color in;
	for (int32_t y = 0; y < image->getHeight(); ++y)
	{
		for (int32_t x = 0; x < image->getWidth(); ++x)
		{
			image->getPixel(x, y, in);

			in.setRed(std::floorf(in.getRed() * m_steps + 0.5f) / m_steps);
			in.setGreen(std::floorf(in.getGreen() * m_steps + 0.5f) / m_steps);
			in.setBlue(std::floorf(in.getBlue() * m_steps + 0.5f) / m_steps);
			in.setAlpha(std::floorf(in.getAlpha() * m_steps + 0.5f) / m_steps);

			final->setPixel(x, y, in);
		}
	}

	return final;
}

	}
}
