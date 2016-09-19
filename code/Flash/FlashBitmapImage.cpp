#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Flash/FlashBitmapImage.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.FlashBitmapImage", 0, FlashBitmapImage, FlashBitmap)

FlashBitmapImage::FlashBitmapImage()
:	FlashBitmap()
{
}

FlashBitmapImage::FlashBitmapImage(const drawing::Image* image)
:	FlashBitmap()
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

const void* FlashBitmapImage::getBits() const
{
	return m_image->getData();
}

void FlashBitmapImage::serialize(ISerializer& s)
{
	FlashBitmap::serialize(s);

	if (s.getDirection() == ISerializer::SdRead)
#if defined(T_LITTLE_ENDIAN)
		m_image = new drawing::Image(drawing::PixelFormat::getA8B8G8R8(), m_width, m_height);
#else
		m_image = new drawing::Image(drawing::PixelFormat::getR8G8B8A8(), m_width, m_height);
#endif

	if (m_image)
	{
		void* bits = m_image->getData();
		uint32_t size = m_width * m_height * 4;
		s >> Member< void* >(L"bits", bits, size);
	}
}

	}
}
