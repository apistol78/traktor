/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Drawing/Image.h"
#include "Render/Editor/Texture/SphereMapFilter.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.SphereMapFilter", SphereMapFilter, drawing::IImageFilter)

void SphereMapFilter::apply(drawing::Image* image) const
{
	int32_t width = image->getWidth();
	int32_t height = image->getHeight();

	Ref< drawing::Image > sphereImage = image->clone();

	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < height; ++x)
		{
			float t = 2.0f * (float(y) / float(height - 1.0f) - 0.5f);
			float s = 2.0f * (float(x) / float(height - 1.0f) - 0.5f);

			if (s * s + t * t > 1.0f)
				continue;

			int offset = 0;
			int slicex = 0;
			int slicey = 0;
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
				slicex = int((reflection.z() * -0.5f + 0.5f) * (height - 1));
				slicey = int((reflection.y() * 0.5f * vsign + 0.5f) * (height - 1));
				break;

			case 1:
				vsign = sign(float(reflection.y()));
				offset = reflection.y() < 0.0f ? height * 2 : height * 3;
				reflection = reflection * (Scalar(1.0f) / -reflection.y());
				slicex = int((reflection.x() * 0.5f * vsign + 0.5f) * (height - 1));
				slicey = int((reflection.z() * 0.5f + 0.5f) * (height - 1));
				break;

			case 2:
				vsign = -sign(float(reflection.z()));
				offset = reflection.z() > 0.0f ? height * 4 : height * 5;
				reflection = reflection * (Scalar(1.0f) / -reflection.z());
				slicex = int((reflection.x() * 0.5f + 0.5f) * (height - 1));
				slicey = int((reflection.y() * 0.5f * vsign + 0.5f) * (height - 1));
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
