/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Math/Const.h"
#include "Drawing/Image.h"
#include "Drawing/Filters/CropFilter.h"

namespace traktor
{
	namespace drawing
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.CropFilter", CropFilter, IImageFilter)

CropFilter::CropFilter(
	AnchorType anchorX,
	AnchorType anchorY,
	int32_t width,
	int32_t height
)
:	m_anchorX(anchorX)
,	m_anchorY(anchorY)
,	m_width(width)
,	m_height(height)
{
}

void CropFilter::apply(Image* image) const
{
	Ref< Image > final = new Image(
		image->getPixelFormat(),
		m_width,
		m_height
	);

	int32_t sx = 0;
	int32_t sy = 0;

	switch (m_anchorX)
	{
	case AtLeft:
		sx = 0;
		break;

	case AtCenter:
		sx = (image->getWidth() - m_width) / 2;
		break;

	case AtRight:
		sx = image->getWidth() - m_width;
		break;
	}

	switch (m_anchorY)
	{
	case AtUp:
		sy = 0;
		break;

	case AtCenter:
		sy = (image->getHeight() - m_height) / 2;
		break;

	case AtDown:
		sy = image->getHeight() - m_height;
		break;
	}

	// Use top-left pixel as filler.
	Color4f fill;
	image->getPixel(0, 0, fill);

	Color4f tmp;
	for (int32_t y = 0; y < m_height; ++y)
	{
		for (int32_t x = 0; x < m_width; ++x)
		{
			if (image->getPixel(x + sx, y + sy, tmp))
				final->setPixelUnsafe(x, y, tmp);
			else
				final->setPixelUnsafe(x, y, fill);
		}
	}

	image->swap(final);
}

	}
}
