/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Math/Const.h"
#include "Drawing/Image.h"
#include "Drawing/Filters/DilateFilter.h"

namespace traktor
{
	namespace drawing
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.DilateFilter", DilateFilter, IImageFilter)

DilateFilter::DilateFilter(int32_t iterations)
:	m_iterations(iterations)
{
}

void DilateFilter::apply(Image* image) const
{
	Color4f tmp;
	for (int32_t i = 0; i < m_iterations; ++i)
	{
		Ref< Image > final = image->clone(false);

		for (int32_t y = 0; y < image->getHeight(); ++y)
		{
			for (int32_t x = 0; x < image->getWidth(); ++x)
			{
				image->getPixelUnsafe(x, y, tmp);
				if (tmp.getAlpha() > FUZZY_EPSILON)
				{
					final->setPixelUnsafe(x, y, tmp);
					continue;
				}

				Color4f acc(0.0f, 0.0f, 0.0f, 0.0f);
				int32_t cnt = 0;

				for (int32_t iy = -1; iy <= 1; ++iy)
				{
					for (int32_t ix = -1; ix <= 1; ++ix)
					{
						if (ix == 0 || iy == 0)
							continue;

						if (image->getPixel(x + ix, y + iy, tmp))
						{
							if (tmp.getAlpha() > FUZZY_EPSILON)
							{
								acc += tmp;
								cnt++;
							}
						}
					}
				}

				if (cnt > 0)
				{
					acc /= Scalar(float(cnt));
					acc.setAlpha(Scalar(1.0f));
					final->setPixelUnsafe(x, y, acc);
				}
				else
					final->setPixelUnsafe(x, y, tmp);
			}
		}

		image->swap(final);
	}
}

	}
}
