/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cmath>
#include "Core/Containers/AlignedVector.h"
#include "Drawing/Image.h"
#include "Drawing/Filters/GammaFilter.h"

namespace traktor
{
	namespace drawing
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.GammaFilter", GammaFilter, IImageFilter)

GammaFilter::GammaFilter(float gamma)
{
	m_gamma[0] =
	m_gamma[1] =
	m_gamma[2] = gamma;
	m_gamma[3] = 1.0f;
}

GammaFilter::GammaFilter(float gammaR, float gammaG, float gammaB, float gammaA)
{
	m_gamma[0] = gammaR;
	m_gamma[1] = gammaG;
	m_gamma[2] = gammaB;
	m_gamma[3] = gammaA;
}

void GammaFilter::apply(Image* image) const
{
	AlignedVector< Color4f > span(image->getWidth());
	for (int32_t y = 0; y < image->getHeight(); ++y)
	{
		image->getSpanUnsafe(y, span.ptr());
		for (int32_t x = 0; x < image->getWidth(); ++x)
		{
			span[x] = Color4f(
				std::pow(span[x].getRed(), m_gamma[0]),
				std::pow(span[x].getGreen(), m_gamma[1]),
				std::pow(span[x].getBlue(), m_gamma[2]),
				std::pow(span[x].getAlpha(), m_gamma[3])
			);
		}
		image->setSpanUnsafe(y, span.c_ptr());
	}
}
	
	}
}
