#include <cstring>
#include "Core/Log/Log.h"
#include "Core/Math/Log2.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Drawing/Filters/ScaleFilter.h"
#include "Flash/FlashBitmap.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.FlashBitmap", 0, FlashBitmap, ISerializable)

FlashBitmap::FlashBitmap()
:	m_width(0)
,	m_height(0)
,	m_mips(0)
,	m_bits(0)
{
}

FlashBitmap::FlashBitmap(drawing::Image* image)
:	m_width(0)
,	m_height(0)
,	m_mips(0)
,	m_bits(0)
{
	create(image);
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

	if (isLog2(m_width) && isLog2(m_height))
		m_mips = log2(std::max(m_width, m_height)) + 1;
	else
		m_mips = 1;

	uint32_t mipChainSize = 0;
	for (uint32_t i = 0; i < m_mips; ++i)
	{
		uint32_t mipWidth = std::max< uint32_t >(m_width >> i, 1);
		uint32_t mipHeight = std::max< uint32_t >(m_height >> i, 1);
		mipChainSize += mipWidth * mipHeight;
	}

	m_bits.reset(new SwfColor [mipChainSize]);

	SwfColor* bits = m_bits.ptr();
	T_ASSERT (bits);

	for (uint32_t i = 0; i < m_mips; ++i)
	{
		uint32_t mipWidth = std::max< uint32_t >(m_width >> i, 1);
		uint32_t mipHeight = std::max< uint32_t >(m_height >> i, 1);

		if (i > 0)
		{
			drawing::ScaleFilter scaleFilter(mipWidth, mipHeight, drawing::ScaleFilter::MnAverage, drawing::ScaleFilter::MgLinear);
			clone = clone->applyFilter(&scaleFilter);
		}

		std::memcpy(
			bits,
			clone->getData(),
			mipWidth * mipHeight * sizeof(SwfColor)
		);

		if (!hasAlpha)
		{
			for (uint32_t i = 0; i < mipWidth * mipHeight; ++i)
				bits[i].alpha = 255;
		}

		bits += mipWidth * mipHeight;
	}

	return true;
}

void FlashBitmap::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"width", m_width);
	s >> Member< uint32_t >(L"height", m_height);
	s >> Member< uint32_t >(L"mips", m_mips);

	uint32_t mipChainSize = 0;
	for (uint32_t i = 0; i < m_mips; ++i)
	{
		uint32_t mipWidth = std::max< uint32_t >(m_width >> i, 1);
		uint32_t mipHeight = std::max< uint32_t >(m_height >> i, 1);
		mipChainSize += mipWidth * mipHeight;
	}

	if (s.getDirection() == ISerializer::SdRead)
		m_bits.reset(new SwfColor [mipChainSize]);

	void* bits = m_bits.ptr();
	uint32_t size = mipChainSize * sizeof(SwfColor);

	s >> Member< void* >(L"bits", bits, size);
}

	}
}
