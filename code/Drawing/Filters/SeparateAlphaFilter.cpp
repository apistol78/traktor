/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Const.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Drawing/Filters/SeparateAlphaFilter.h"

namespace traktor::drawing
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.SeparateAlphaFilter", SeparateAlphaFilter, IImageFilter)

void SeparateAlphaFilter::apply(Image* image) const
{
	const int32_t width = image->getWidth();
	const int32_t height = image->getHeight();

	AlignedVector< Color4f > span(width);
	for (int32_t y = 0; y < height; ++y)
	{
		image->getSpanUnsafe(y, span.ptr());
		for (int32_t x = 0; x < width; ++x)
		{
			const Scalar a = span[x].getAlpha();
			if (a > FUZZY_EPSILON)
			{
				span[x] = span[x] / a;
				span[x].setAlpha(a);
			}
			else
				span[x] = Color4f(0.0f, 0.0f, 0.0f, 0.0f);
		}
		image->setSpanUnsafe(y, span.c_ptr());
	}
}

}
