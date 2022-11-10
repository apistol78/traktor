/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cmath>
#include "Drawing/Filters/QuantizeFilter.h"
#include "Drawing/Image.h"

namespace traktor::drawing
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.QuantizeFilter", QuantizeFilter, IImageFilter)

QuantizeFilter::QuantizeFilter(int steps)
:	m_steps(steps)
{
}

void QuantizeFilter::apply(Image* image) const
{
	Color4f in;
	for (int32_t y = 0; y < image->getHeight(); ++y)
	{
		for (int32_t x = 0; x < image->getWidth(); ++x)
		{
			image->getPixelUnsafe(x, y, in);

			in.setRed(Scalar(std::floor(in.getRed() * m_steps + 0.5f) / m_steps));
			in.setGreen(Scalar(std::floor(in.getGreen() * m_steps + 0.5f) / m_steps));
			in.setBlue(Scalar(std::floor(in.getBlue() * m_steps + 0.5f) / m_steps));
			in.setAlpha(Scalar(std::floor(in.getAlpha() * m_steps + 0.5f) / m_steps));

			image->setPixelUnsafe(x, y, in);
		}
	}
}

}
