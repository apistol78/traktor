/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Const.h"
#include "Drawing/Image.h"
#include "Drawing/Filters/NormalizeFilter.h"

namespace traktor::drawing
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.NormalizeFilter", NormalizeFilter, IImageFilter)

NormalizeFilter::NormalizeFilter(float scale)
:	m_scale(scale)
{
}

void NormalizeFilter::apply(Image* image) const
{
	const Vector4 scale(1.0f + m_scale, 1.0f + m_scale, 1.0f);
	const int32_t width = image->getWidth();
	const int32_t height = image->getHeight();

	AlignedVector< Color4f > row(width, Color4f(0, 0, 0, 0));
	for (int32_t y = 0; y < height; ++y)
	{
		image->getSpanUnsafe(y, row.ptr());
		for (int32_t x = 0; x < width; ++x)
		{
			Vector4 n = (Vector4(row[x]) * 2.0_simd - 1.0_simd) * scale;

			const Scalar ln = n.length2();
			if (ln >= FUZZY_EPSILON * FUZZY_EPSILON)
				n *= reciprocalSquareRoot(ln);
			else
				n.set(0.0f, 0.0f, 1.0f);

			row[x] = Color4f((n * 0.5_simd + 0.5_simd).xyz0() + Vector4(0.0f, 0.0f, 0.0f, row[x].getAlpha()));
		}
		image->setSpanUnsafe(y, row.ptr());
	}
}

}
