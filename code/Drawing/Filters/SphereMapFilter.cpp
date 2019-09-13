#include "Drawing/Image.h"
#include "Drawing/Filters/SphereMapFilter.h"

namespace traktor
{
	namespace drawing
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.SphereMapFilter", SphereMapFilter, IImageFilter)

void SphereMapFilter::apply(Image* image) const
{
	int32_t width = image->getWidth();
	int32_t height = image->getHeight();

	Ref< Image > sphereImage = image->clone();

	for (int32_t y = 0; y < height; ++y)
	{
		for (int32_t x = 0; x < height; ++x)
		{
			float t = 2.0f * (float(y) / float(height - 1.0f) - 0.5f);
			float s = 2.0f * (float(x) / float(height - 1.0f) - 0.5f);

			if (s * s + t * t > 1.0f)
				continue;

			int32_t offset = 0;
			int32_t slicex = 0;
			int32_t slicey = 0;
			float vsign = 0;

			Vector4 direction(s, t, sqrtf(1.0f - s * s - t * t), 0.0f);
			Vector4 reflection(
				direction.x() * direction.z() * 2.0f,
				direction.y() * direction.z() * 2.0f,
				direction.z() * direction.z() * 2.0f - 1.0f
			);

			switch (majorAxis3(reflection))
			{
			case 0:
				vsign = -sign(float(reflection.x()));
				offset = reflection.x() < 0.0f ? 0 : height;
				reflection = reflection * (Scalar(1.0f) / -reflection.x());
				slicex = int32_t((reflection.z() * -0.5f + 0.5f) * (height - 1));
				slicey = int32_t((reflection.y() * 0.5f * vsign + 0.5f) * (height - 1));
				break;

			case 1:
				vsign = sign(float(reflection.y()));
				offset = reflection.y() < 0.0f ? height * 2 : height * 3;
				reflection = reflection * (Scalar(1.0f) / -reflection.y());
				slicex = int32_t((reflection.x() * 0.5f * vsign + 0.5f) * (height - 1));
				slicey = int32_t((reflection.z() * 0.5f + 0.5f) * (height - 1));
				break;

			case 2:
				vsign = -sign(float(reflection.z()));
				offset = reflection.z() > 0.0f ? height * 4 : height * 5;
				reflection = reflection * (Scalar(1.0f) / -reflection.z());
				slicex = int32_t((reflection.x() * 0.5f + 0.5f) * (height - 1));
				slicey = int32_t((reflection.y() * 0.5f * vsign + 0.5f) * (height - 1));
				break;
			}

			Color4f color;
			if (image->getPixel(slicex + offset, slicey, color))
				sphereImage->setPixel(x, y, color);
		}
	}

	image->swap(sphereImage);
}

	}
}
