/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Drawing/Image.h"
#include "Drawing/Filters/NormalMapFilter.h"

namespace traktor
{
	namespace drawing
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.NormalMapFilter", NormalMapFilter, IImageFilter)

NormalMapFilter::NormalMapFilter(float scale)
:	m_scale(scale)
{
}

void NormalMapFilter::apply(Image* image) const
{
	Ref< Image > final = image->clone(false);
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
					(c[1] - c[0]) * m_scale,
					(c[2] - c[0]) * m_scale,
					1.0f
				).normalized();

			normal = normal * Scalar(0.5f) + Scalar(0.5f);

			final->setPixelUnsafe(x, y, Color4f(normal.xyz0()));
		}
	}

	image->swap(final);
}
	
	}
}
