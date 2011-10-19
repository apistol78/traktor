#include <cstring>
#include "Core/Log/Log.h"
#include "Core/Math/Log2.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Drawing/Filters/ScaleFilter.h"
#include "Flash/FlashBitmap.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashBitmap", FlashBitmap, Object)

FlashBitmap::FlashBitmap()
:	m_width(0)
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
	bool hasAlpha = image->getPixelFormat().getAlphaBits() > 0;

	// Ensure pixel format match Flash bits.
	Ref< drawing::Image > clone = image;
	if (hasAlpha)
	{
#if defined(T_LITTLE_ENDIAN)
		if (clone->getPixelFormat() != drawing::PixelFormat::getA8B8G8R8())
		{
			clone = clone->clone();
			clone->convert(drawing::PixelFormat::getA8B8G8R8());
		}
#else	// T_BIG_ENDIAN
		if (clone->getPixelFormat() != drawing::PixelFormat::getR8G8B8A8())
		{
			clone = clone->clone();
			clone->convert(drawing::PixelFormat::getR8G8B8A8());
		}
#endif
	}
	else
	{
#if defined(T_LITTLE_ENDIAN)
		if (clone->getPixelFormat() != drawing::PixelFormat::getX8B8G8R8())
		{
			clone = clone->clone();
			clone->convert(drawing::PixelFormat::getX8B8G8R8());
		}
#else	// T_BIG_ENDIAN
		if (clone->getPixelFormat() != drawing::PixelFormat::getR8G8B8X8())
		{
			clone = clone->clone();
			clone->convert(drawing::PixelFormat::getR8G8B8X8());
		}
#endif
	}

	m_width = image->getWidth();
	m_height = image->getHeight();
	m_bits.reset(new SwfColor [m_width * m_height]);

	std::memcpy(
		m_bits.ptr(),
		clone->getData(),
		clone->getWidth() * clone->getHeight() * sizeof(SwfColor)
	);

	if (!hasAlpha)
	{
		for (int32_t i = 0; i < clone->getWidth() * clone->getHeight(); ++i)
			m_bits[i].alpha = 255;
	}

	return true;
}

bool FlashBitmap::create(uint16_t width, uint16_t height)
{
	m_width = width;
	m_height = height;
	m_bits.reset(new SwfColor [m_width * m_height]);
	return true;
}

	}
}
