/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Containers/AlignedVector.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Drawing/Filters/TransformFilter.h"

namespace traktor
{
	namespace drawing
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.TransformFilter", TransformFilter, IImageFilter)

TransformFilter::TransformFilter(const Color4f& Km, const Color4f& Kc)
:	m_Km(Km)
,	m_Kc(Kc)
{
}

void TransformFilter::apply(Image* image) const
{
	Color4f in;

	int32_t width = image->getWidth();
	int32_t height = image->getHeight();

	AlignedVector< Color4f > span(width);

	for (int32_t y = 0; y < height; ++y)
	{
		image->getSpanUnsafe(y, &span[0]);

		Color4f Km = m_Km, Kc = m_Kc;
		int32_t x = 0;
		for (; x < width - 8; x += 8)
		{
			span[x + 0] = span[x + 0] * Km + Kc;
			span[x + 1] = span[x + 1] * Km + Kc;
			span[x + 2] = span[x + 2] * Km + Kc;
			span[x + 3] = span[x + 3] * Km + Kc;
			span[x + 4] = span[x + 4] * Km + Kc;
			span[x + 5] = span[x + 5] * Km + Kc;
			span[x + 6] = span[x + 6] * Km + Kc;
			span[x + 7] = span[x + 7] * Km + Kc;
		}
		for (; x < width; ++x)
			span[x] = span[x] * Km + Kc;

		image->setSpanUnsafe(y, &span[0]);
	}
}
	
	}
}
