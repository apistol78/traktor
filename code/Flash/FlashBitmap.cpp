#include <cstring>
#include "Core/Log/Log.h"
#include "Core/Math/Log2.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Drawing/Filters/ScaleFilter.h"
#include "Flash/FlashBitmap.h"
#include "Flash/Action/Avm1/Classes/AsObject.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashBitmap", FlashBitmap, ActionObject)

FlashBitmap::FlashBitmap()
:	ActionObject(AsObject::getInstance())
,	m_originalWidth(0)
,	m_originalHeight(0)
,	m_bitsWidth(0)
,	m_bitsHeight(0)
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
	
#if TARGET_OS_IPHONE
	// Ensure bitmap is a power-of-2.
	uint32_t log2Width = previousLog2(clone->getWidth());
	uint32_t log2Height = previousLog2(clone->getHeight());
	
	// Clamp size; we cannot afford too big bitmaps.
	if (log2Width > 64)
		log2Width = 64;
	if (log2Height > 64)
		log2Height = 64;
	
	if (log2Width != clone->getWidth() || log2Height != clone->getHeight())
	{
		log::warning << L"Flash bitmap resized to power-of-2 " << log2Width << L"*" << log2Height << Endl;
		
		drawing::ScaleFilter scaleFilter(
			log2Width,
			log2Height,
			drawing::ScaleFilter::MnAverage,
			drawing::ScaleFilter::MgLinear
		);
		
		clone = clone->applyFilter(&scaleFilter);
	}
#endif

	m_originalWidth = image->getWidth();
	m_originalHeight = image->getHeight();
	m_bitsWidth = clone->getWidth();
	m_bitsHeight = clone->getHeight();
	m_bits.reset(new SwfColor [m_bitsWidth * m_bitsHeight]);

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
	m_originalWidth = width;
	m_originalHeight = height;
#if TARGET_OS_IPHONE
	m_bitsWidth = previousLog2(width);
	m_bitsHeight = previousLog2(height);
#else
	m_bitsWidth = width;
	m_bitsHeight = height;
#endif
	m_bits.reset(new SwfColor [m_bitsWidth * m_bitsHeight]);
	return true;
}

uint16_t FlashBitmap::getOriginalWidth() const
{
	return m_originalWidth;
}

uint16_t FlashBitmap::getOriginalHeight() const
{
	return m_originalHeight;
}

uint16_t FlashBitmap::getBitsWidth() const
{
	return m_bitsWidth;
}

uint16_t FlashBitmap::getBitsHeight() const
{
	return m_bitsHeight;
}

const SwfColor* FlashBitmap::getBits() const
{
	return m_bits.c_ptr();
}

void FlashBitmap::setPixel(uint16_t x, uint16_t y, const SwfColor& color)
{
	uint32_t xx = uint32_t(x * m_bitsWidth) / m_originalWidth;
	uint32_t yy = uint32_t(y * m_bitsHeight) / m_originalHeight;
	
	if (xx < m_bitsWidth && yy < m_bitsHeight)
		m_bits[xx + yy * m_bitsWidth] = color;
}

	}
}
