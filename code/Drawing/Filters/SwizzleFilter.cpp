/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cctype>
#include "Drawing/Filters/SwizzleFilter.h"
#include "Drawing/Image.h"

namespace traktor::drawing
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.SwizzleFilter", SwizzleFilter, IImageFilter)

SwizzleFilter::SwizzleFilter(const std::wstring& swizzle)
{
	T_ASSERT(swizzle.length() >= 4);
	m_swizzle[0] = std::toupper(swizzle[0]);
	m_swizzle[1] = std::toupper(swizzle[1]);
	m_swizzle[2] = std::toupper(swizzle[2]);
	m_swizzle[3] = std::toupper(swizzle[3]);
}

void SwizzleFilter::apply(Image* image) const
{
	Color4f in, out;
	for (int32_t y = 0; y < image->getHeight(); ++y)
	{
		for (int32_t x = 0; x < image->getWidth(); ++x)
		{
			image->getPixelUnsafe(x, y, in);
			for (int32_t i = 0; i < 4; ++i)
			{
				switch (m_swizzle[i])
				{
				case L'A':
					out.set(i, in.getAlpha());
					break;
				case L'R':
					out.set(i, in.getRed());
					break;
				case L'G':
					out.set(i, in.getGreen());
					break;
				case L'B':
					out.set(i, in.getBlue());
					break;
				case L'0':
					out.set(i, 0.0_simd);
					break;
				case L'1':
					out.set(i, 1.0_simd);
					break;
				}
			}
			image->setPixelUnsafe(x, y, out);
		}
	}
}

}
