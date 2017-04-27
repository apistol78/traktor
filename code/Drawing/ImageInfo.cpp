/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Drawing/ImageInfo.h"

namespace traktor
{
	namespace drawing
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.ImageInfo", ImageInfo, Object)

ImageInfo::ImageInfo()
:	m_gamma(0.0f)
{
}

void ImageInfo::setAuthor(const std::wstring& author)
{
	m_author = author;
}

const std::wstring& ImageInfo::getAuthor() const
{
	return m_author;
}

void ImageInfo::setCopyright(const std::wstring& copyright)
{
	m_copyright = copyright;
}

const std::wstring& ImageInfo::getCopyright() const
{
	return m_copyright;
}

void ImageInfo::setFormat(const std::wstring& format)
{
	m_format = format;
}

const std::wstring& ImageInfo::getFormat() const
{
	return m_format;
}

void ImageInfo::setGamma(float gamma)
{
	m_gamma = gamma;
}

float ImageInfo::getGamma() const
{
	return m_gamma;
}

	}
}
