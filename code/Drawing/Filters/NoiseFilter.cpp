#include "Core/Math/Random.h"
#include "Drawing/Image.h"
#include "Drawing/Filters/NoiseFilter.h"

namespace traktor
{
	namespace drawing
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.NoiseFilter", NoiseFilter, IImageFilter)

NoiseFilter::NoiseFilter(float strength)
:	m_strength(strength)
{
}

void NoiseFilter::apply(Image* image) const
{
	Color4f in;
	Random r;

	for (int32_t y = 0; y < image->getHeight(); ++y)
	{
		for (int32_t x = 0; x < image->getWidth(); ++x)
		{
			float f = 1.0f + (2.0f * r.nextFloat() - 1.0f) * m_strength;
			image->getPixelUnsafe(x, y, in);
			image->setPixelUnsafe(x, y, in * Scalar(f));
		}
	}
}

	}
}
