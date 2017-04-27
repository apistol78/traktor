/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Const.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Drawing/Filters/SeparateAlphaFilter.h"

namespace traktor
{
	namespace drawing
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.SeparateAlphaFilter", SeparateAlphaFilter, IImageFilter)

void SeparateAlphaFilter::apply(Image* image) const
{
	Color4f in;

	int32_t width = image->getWidth();
	int32_t height = image->getHeight();

	AlignedVector< Color4f > span(width);
	for (int32_t y = 0; y < height; ++y)
	{
		image->getSpanUnsafe(y, span.ptr());
		for (int32_t x = 0; x < width; ++x)
		{
			Scalar a = span[x].getAlpha();
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
}
