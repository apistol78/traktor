#include "Flash/Action/Common/BlurFilter.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.BlurFilter", BlurFilter, BitmapFilter)

BlurFilter::BlurFilter(avm_number_t blurX, avm_number_t blurY, int32_t quality)
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

avm_number_t BlurFilter::getBlurX()
{
	return m_blurX;
}

void BlurFilter::setBlurX(avm_number_t blurX)
{
	m_blurX = blurX;
}

avm_number_t BlurFilter::getBlurY()
{
	return m_blurY;
}

void BlurFilter::setBlurY(avm_number_t blurY)
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
