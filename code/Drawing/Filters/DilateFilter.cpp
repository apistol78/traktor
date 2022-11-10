/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Const.h"
#include "Drawing/Image.h"
#include "Drawing/Filters/DilateFilter.h"

namespace traktor::drawing
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

		int32_t dilated = 0;
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
						if (ix == 0 && iy == 0)
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
					dilated++;
				}
				else
					final->setPixelUnsafe(x, y, tmp);
			}
		}

		if (dilated > 0)
			image->swap(final);
		else
		{
			// No pixels was dilated; entire image fully occupied thus no need to
			// iterate further.
			break;
		}
	}
}

}
