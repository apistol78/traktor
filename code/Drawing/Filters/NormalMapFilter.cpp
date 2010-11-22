#include "Drawing/Image.h"
#include "Drawing/Filters/NormalMapFilter.h"

namespace traktor
{
	namespace drawing
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.NormalMapFilter", NormalMapFilter, IImageFilter)

NormalMapFilter::NormalMapFilter(float scale)
{
}

Ref< Image > NormalMapFilter::apply(const Image* image)
{
	Ref< Image > final = new Image(image->getPixelFormat(), image->getWidth(), image->getHeight(), image->getPalette());
	Color4f in0, in1, in2;
	Scalar c[3];

	for (int32_t y = 0; y < image->getHeight(); ++y)
	{
		for (int32_t x = 0; x < image->getWidth(); ++x)
		{
			image->getPixel(x, y, in0);
			image->getPixel(x + 1, y, in1);
			image->getPixel(x, y + 1, in2);

			for (int i = 0; i < 3; ++i)
				c[i] = (in0.getRed() + in1.getGreen() + in2.getBlue()) / Scalar(3.0f);

			Vector4 normal =
				Vector4(
					c[1] - c[0],
					c[2] - c[0],
					1.0f
				).normalized();

			normal = normal * Scalar(0.5f) + Scalar(0.5f);

			final->setPixelUnsafe(x, y, Color4f(normal.xyz0()));
		}
	}

	return final;
}
	
	}
}
