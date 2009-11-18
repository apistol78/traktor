#include <cmath>
#include "Drawing/Filters/GammaFilter.h"
#include "Drawing/Image.h"

namespace traktor
{
	namespace drawing
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.GammaFilter", GammaFilter, IImageFilter)

template <typename T> T clamp(T value, T min, T max)
{
	if (value < min) return min;
	if (value > max) return max;
	return value;
}

GammaFilter::GammaFilter(float gamma)
{
	m_gamma[0] =
	m_gamma[1] =
	m_gamma[2] = gamma;
	m_gamma[3] = 1.0f;
}

GammaFilter::GammaFilter(float gammaR, float gammaG, float gammaB, float gammaA)
{
	m_gamma[0] = gammaR;
	m_gamma[1] = gammaG;
	m_gamma[2] = gammaB;
	m_gamma[3] = gammaA;
}

Ref< Image > GammaFilter::apply(const Image* image)
{
	Ref< Image > final = new Image(image->getPixelFormat(), image->getWidth(), image->getHeight(), image->getPalette());
	Color in;
	for (int32_t y = 0; y < image->getHeight(); ++y)
	{
		for (int32_t x = 0; x < image->getWidth(); ++x)
		{
			image->getPixel(x, y, in);
			Color out(
				std::pow(in.getRed(), m_gamma[0]),
				std::pow(in.getGreen(), m_gamma[1]),
				std::pow(in.getBlue(), m_gamma[2]),
				std::pow(in.getAlpha(), m_gamma[3])
			);
			final->setPixel(x, y, out);
		}
	}
	return final;
}
	
	}
}
