/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Const.h"
#include "Drawing/Image.h"
#include "Drawing/Filters/NormalizeFilter.h"

namespace traktor
{
	namespace drawing
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.NormalizeFilter", NormalizeFilter, IImageFilter)

void NormalizeFilter::apply(Image* image) const
{
	const Scalar c_two(2.0f);
	const Scalar c_one(1.0f);
	const Scalar c_half(0.5f);

	const int32_t width = image->getWidth();
	const int32_t height = image->getHeight();

	AlignedVector< Color4f > row(width, Color4f(0, 0, 0, 0));
	for (int32_t y = 0; y < height; ++y)
	{
		image->getSpanUnsafe(y, row.ptr());
		for (int32_t x = 0; x < width; ++x)
		{
			Vector4 n = Vector4(row[x]) * c_two - c_one;			
			Scalar ln = n.length2();
			if (ln >= FUZZY_EPSILON * FUZZY_EPSILON)
				n *= reciprocalSquareRoot(ln);
			else
				n.set(0.0f, 1.0f, 0.0f);
			row[x] = Color4f((n * c_half + c_half).xyz0() + Vector4(0.0f, 0.0f, 0.0f, row[x].getAlpha()));
		}
		image->setSpanUnsafe(y, row.ptr());
	}
}

	}
}
