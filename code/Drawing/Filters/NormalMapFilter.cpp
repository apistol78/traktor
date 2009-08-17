#include "Drawing/Filters/NormalMapFilter.h"
#include "Drawing/Image.h"
#include "Core/Heap/GcNew.h"
#include "Core/Math/Vector4.h"

namespace traktor
{
	namespace drawing
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.NormalMapFilter", NormalMapFilter, ImageFilter)

NormalMapFilter::NormalMapFilter(float scale)
{
}

Image* NormalMapFilter::apply(const Image* image)
{
	Ref< Image > final = gc_new< Image >(image->getPixelFormat(), image->getWidth(), image->getHeight(), image->getPalette());
	Color in[3];

	for (int32_t y = 0; y < image->getHeight(); ++y)
	{
		for (int32_t x = 0; x < image->getWidth(); ++x)
		{
			image->getPixel(x, y, in[0]);
			image->getPixel(x + 1, y, in[1]);
			image->getPixel(x, y + 1, in[2]);

			float c[3];
			for (int i = 0; i < 3; ++i)
				c[i] = (in[i].getRed() + in[i].getGreen() + in[i].getBlue()) / 3.0f;

			Vector4 normal =
				Vector4(
					c[1] - c[0],
					c[2] - c[0],
					1.0f
				).normalized();

			normal = normal * Scalar(0.5f) + Scalar(0.5f);

			final->setPixel(x, y, Color(
				normal.x(),
				normal.y(),
				normal.z(),
				c[0]
			));
		}
	}

	return final;
}
	
	}
}
