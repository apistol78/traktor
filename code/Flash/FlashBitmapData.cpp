#include <cstring>
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Flash/FlashBitmapData.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.FlashBitmapData", 0, FlashBitmapData, FlashBitmap)

FlashBitmapData::FlashBitmapData()
:	FlashBitmap()
,	m_bits(0)
{
}

FlashBitmapData::FlashBitmapData(drawing::Image* image)
:	FlashBitmap()
,	m_bits(0)
{
	create(image);
}

FlashBitmapData::~FlashBitmapData()
{
	m_bits.release();
}

bool FlashBitmapData::create(drawing::Image* image)
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

	SwfColor* bits = m_bits.ptr();
	T_ASSERT (bits);

	std::memcpy(
		bits,
		clone->getData(),
		m_width * m_height * sizeof(SwfColor)
	);

	if (!hasAlpha)
	{
		for (uint32_t i = 0; i < m_width * m_height; ++i)
			bits[i].alpha = 255;
	}

	return true;
}

void FlashBitmapData::serialize(ISerializer& s)
{
	FlashBitmap::serialize(s);

	uint32_t bitsSize = m_width * m_height;

	if (s.getDirection() == ISerializer::SdRead)
		m_bits.reset(new SwfColor [bitsSize]);

	void* bits = m_bits.ptr();
	uint32_t size = bitsSize * sizeof(SwfColor);

	s >> Member< void* >(L"bits", bits, size);
}

	}
}
