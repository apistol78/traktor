#include <cstring>
#include "Flash/FlashBitmap.h"
#include "Flash/Action/Classes/AsObject.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashBitmap", FlashBitmap, ActionObject)

FlashBitmap::FlashBitmap()
:	ActionObject(AsObject::getInstance())
,	m_width(0)
,	m_height(0)
,	m_bits(0)
{
}

FlashBitmap::~FlashBitmap()
{
	m_bits.release();
}

bool FlashBitmap::create(drawing::Image* image)
{
	if (!create(image->getWidth(), image->getHeight()))
		return false;

	bool hasAlpha = image->getPixelFormat()->getAlphaBits() > 0;

	// Ensure pixel format match Flash bits.
	Ref< drawing::Image > clone = image->clone();
	if (hasAlpha)
	{
#if defined(T_LITTLE_ENDIAN)
		clone->convert(drawing::PixelFormat::getA8B8G8R8());
#else	// T_BIG_ENDIAN
		clone->convert(drawing::PixelFormat::getR8G8B8A8());
#endif
	}
	else
	{
#if defined(T_LITTLE_ENDIAN)
		clone->convert(drawing::PixelFormat::getX8B8G8R8());
#else	// T_BIG_ENDIAN
		clone->convert(drawing::PixelFormat::getR8G8B8X8());
#endif
	}

	std::memcpy(
		m_bits.ptr(),
		clone->getData(),
		image->getWidth() * image->getHeight() * sizeof(SwfColor)
	);

	if (!hasAlpha)
	{
		for (int32_t i = 0; i < image->getWidth() * image->getHeight(); ++i)
			m_bits[i].alpha = 255;
	}

	return true;
}

bool FlashBitmap::create(uint16_t width, uint16_t height)
{
	m_width = width;
	m_height = height;
	m_bits.reset(new SwfColor [width * height]);
	return true;
}

uint16_t FlashBitmap::getWidth() const
{
	return m_width;
}

uint16_t FlashBitmap::getHeight() const
{
	return m_height;
}

const SwfColor* FlashBitmap::getBits() const
{
	return m_bits.c_ptr();
}

void FlashBitmap::setPixel(uint16_t x, uint16_t y, const SwfColor& color)
{
	if (m_bits.ptr())
		m_bits[x + y * m_width] = color;
}

	}
}
