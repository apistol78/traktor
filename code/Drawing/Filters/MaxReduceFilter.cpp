/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Drawing/Filters/MaxReduceFilter.h"

#include "Drawing/Image.h"

namespace traktor::drawing
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.MaxReduceFilter", MaxReduceFilter, IImageFilter)

void MaxReduceFilter::apply(Image* image) const
{
	const int32_t sourceWidth = image->getWidth();
	const int32_t sourceHeight = image->getHeight();

	Ref< drawing::Image > final = new drawing::Image(image->getPixelFormat(), m_width, m_height, image->getPalette());

	AlignedVector< Color4f > source(sourceWidth, Color4f(0.0f, 0.0f, 0.0f, 0.0f));
	AlignedVector< Color4f > destination(m_width, Color4f(0.0f, 0.0f, 0.0f, 0.0f));

	const float sx = float(sourceWidth) / m_width;
	const float sy = float(sourceHeight) / m_height;

	for (int32_t y = 0; y < m_height; ++y)
	{
		// Source rows covered by this destination row; at least one even when magnifying.
		const int32_t y1 = std::min(int32_t(y * sy), sourceHeight - 1);
		const int32_t y2 = std::min(std::max(int32_t(y * sy + sy), y1 + 1), sourceHeight);

		for (int32_t yy = y1; yy < y2; ++yy)
		{
			image->getSpanUnsafe(yy, &source[0]);

			for (int32_t x = 0; x < m_width; ++x)
			{
				const int32_t x1 = std::min(int32_t(x * sx), sourceWidth - 1);
				const int32_t x2 = std::min(std::max(int32_t(x * sx + sx), x1 + 1), sourceWidth);

				Color4f c = source[x1];
				for (int32_t xx = x1 + 1; xx < x2; ++xx)
					c = traktor::max(c, source[xx]);

				destination[x] = (yy > y1) ? traktor::max(destination[x], c) : c;
			}
		}

		final->setSpanUnsafe(y, &destination[0]);
	}

	image->swap(final);
}

}
