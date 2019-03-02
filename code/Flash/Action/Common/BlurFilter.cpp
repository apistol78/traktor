#include "Flash/Action/Common/BlurFilter.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.BlurFilter", BlurFilter, BitmapFilter)

BlurFilter::BlurFilter(float blurX, float blurY, int32_t quality)
:	BitmapFilter("flash.filters.BlurFilter")
,	m_blurX(blurX)
,	m_blurY(blurY)
,	m_quality(quality)
{
}

Ref< BitmapFilter > BlurFilter::clone()
{
	return new BlurFilter(m_blurX, m_blurY, m_quality);
}

float BlurFilter::getBlurX()
{
	return m_blurX;
}

void BlurFilter::setBlurX(float blurX)
{
	m_blurX = blurX;
}

float BlurFilter::getBlurY()
{
	return m_blurY;
}

void BlurFilter::setBlurY(float blurY)
{
	m_blurY = blurY;
}

int32_t BlurFilter::getQuality()
{
	return m_quality;
}

void BlurFilter::setQuality(int32_t quality)
{
	m_quality = quality;
}

	}
}
