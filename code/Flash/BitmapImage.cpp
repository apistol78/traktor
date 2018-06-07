/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Flash/BitmapImage.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.BitmapImage", 0, BitmapImage, Bitmap)

BitmapImage::BitmapImage()
:	Bitmap()
{
}

BitmapImage::BitmapImage(const drawing::Image* image)
:	Bitmap()
{
	m_image = image->clone();
#if defined(T_LITTLE_ENDIAN)
	if (m_image->getPixelFormat() != drawing::PixelFormat::getA8B8G8R8())
		m_image->convert(drawing::PixelFormat::getA8B8G8R8());
#else
	if (m_image->getPixelFormat() != drawing::PixelFormat::getR8G8B8A8())
		m_image->convert(drawing::PixelFormat::getR8G8B8A8());
#endif

	if (image->getPixelFormat().getAlphaBits() <= 0)
		m_image->clearAlpha(1.0f);

	m_width = uint32_t(m_image->getWidth());
	m_height = uint32_t(m_image->getHeight());
}

const void* BitmapImage::getBits() const
{
	return m_image->getData();
}

void BitmapImage::serialize(ISerializer& s)
{
	Bitmap::serialize(s);

	if (s.getDirection() == ISerializer::SdRead)
#if defined(T_LITTLE_ENDIAN)
		m_image = new drawing::Image(drawing::PixelFormat::getA8B8G8R8(), m_width, m_height);
#else
		m_image = new drawing::Image(drawing::PixelFormat::getR8G8B8A8(), m_width, m_height);
#endif

	void* bits = 0;
	uint32_t size = 0;

	if (m_image)
	{
		bits = m_image->getData();
		size = m_width * m_height * 4;
	}

	s >> Member< void* >(L"bits", bits, size);
}

	}
}
