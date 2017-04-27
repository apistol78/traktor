/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Const.h"
#include "Drawing/Image.h"
#include "Drawing/Filters/BlurFilter.h"

namespace traktor
{
	namespace drawing
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.BlurFilter", BlurFilter, IImageFilter)

BlurFilter::BlurFilter(int32_t x, int32_t y)
:	m_x(x)
,	m_y(y)
{
}

void BlurFilter::apply(Image* image) const
{
	Ref< Image > imm = image->clone(false);

	int32_t w = image->getWidth();
	int32_t h = image->getHeight();

	AlignedVector< Color4f > span(std::max(w + m_x * 2, h + m_y * 2));
	AlignedVector< Color4f > out(std::max(w, h));

	// Horizontal pass.
	{
		const Scalar invX(1.0f / (m_x * 2.0f + 1.0f));

		for (int32_t y = 0; y < h; ++y)
		{
			image->getSpanUnsafe(y, span.ptr() + m_x);

			for (int32_t x = 0; x < m_x; ++x)
			{
				span[x] = span[m_x];
				span[x + w + m_x] = span[w + m_x - 1];
			}

			for (int32_t x = 0; x < w; ++x)
				out[x] = span[x + m_x];

			for (int32_t dx = 0; dx < m_x; ++dx)
			{
				for (int32_t x = 0; x < w; ++x)
					out[x] += span[x + m_x - dx];
				for (int32_t x = 0; x < w; ++x)
					out[x] += span[x + m_x + dx];
			}

			for (int32_t x = 0; x < w; ++x)
				out[x] *= invX;

			imm->setSpanUnsafe(y, out.c_ptr());
		}
	}

	// Vertical pass.
	{
		const Scalar invY(1.0f / (m_y * 2.0f + 1.0f));

		for (int32_t x = 0; x < w; ++x)
		{
			imm->getVerticalSpanUnsafe(x, span.ptr() + m_y);

			for (int32_t y = 0; y < m_y; ++y)
			{
				span[y] = span[m_y];
				span[y + h + m_y] = span[h + m_y - 1];
			}

			for (int32_t y = 0; y < h; ++y)
				out[y] = span[y + m_y];

			for (int32_t dy = 0; dy < m_y; ++dy)
			{
				for (int32_t y = 0; y < h; ++y)
					out[y] += span[y + m_y - dy];
				for (int32_t y = 0; y < h; ++y)
					out[y] += span[y + m_y + dy];
			}

			for (int32_t y = 0; y < h; ++y)
				out[y] *= invY;

			image->setVerticalSpanUnsafe(x, out.c_ptr());
		}
	}
}

	}
}
