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

void GammaFilter::apply(Image* image) const
{
	Color4f in;
	for (int32_t y = 0; y < image->getHeight(); ++y)
	{
		for (int32_t x = 0; x < image->getWidth(); ++x)
		{
			image->getPixelUnsafe(x, y, in);
			Color4f out(
				std::pow(in.getRed(), m_gamma[0]),
				std::pow(in.getGreen(), m_gamma[1]),
				std::pow(in.getBlue(), m_gamma[2]),
				std::pow(in.getAlpha(), m_gamma[3])
			);
			image->setPixelUnsafe(x, y, out);
		}
	}
}
	
	}
}
