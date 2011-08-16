#include "Drawing/PixelFormat.h"
#include "Drawing/Palette.h"
#include "Core/Math/MathUtils.h"

namespace traktor
{
	namespace drawing
	{
		namespace
		{

inline float clamp(float v)
{
	return min(max(v, 0.0f), 1.0f);
}

#if defined(T_LITTLE_ENDIAN)

uint32_t unpack_1(const void* T_RESTRICT p)
{
	const uint8_t* T_RESTRICT c = static_cast< const uint8_t* T_RESTRICT >(p);
	return c[0];
}

uint32_t unpack_2(const void* T_RESTRICT p)
{
	const uint8_t* T_RESTRICT c = static_cast< const uint8_t* T_RESTRICT >(p);
	return (c[1] << 8) | c[0];
}

uint32_t unpack_3(const void* T_RESTRICT p)
{
	const uint8_t* T_RESTRICT c = static_cast< const uint8_t* T_RESTRICT >(p);
	return (c[2] << 16) | (c[1] << 8) | c[0];
}

uint32_t unpack_4(const void* T_RESTRICT p)
{
	const uint8_t* T_RESTRICT c = static_cast< const uint8_t* T_RESTRICT >(p);
	return (c[3] << 24) | (c[2] << 16) | (c[1] << 8) | c[0];
}

void pack_1(void* T_RESTRICT p, uint32_t v)
{
	uint8_t* T_RESTRICT d = static_cast< uint8_t* T_RESTRICT >(p);
	d[0] = v & 255;
}

void pack_2(void* T_RESTRICT p, uint32_t v)
{
	uint8_t* T_RESTRICT d = static_cast< uint8_t* T_RESTRICT >(p);
	d[0] = v & 255;
	d[1] = (v >> 8) & 255;
}

void pack_3(void* T_RESTRICT p, uint32_t v)
{
	uint8_t* T_RESTRICT d = static_cast< uint8_t* T_RESTRICT >(p);
	d[0] = v & 255;
	d[1] = (v >> 8) & 255;
	d[2] = (v >> 16) & 255;
}

void pack_4(void* T_RESTRICT p, uint32_t v)
{
	uint8_t* T_RESTRICT d = static_cast< uint8_t* T_RESTRICT >(p);
	d[0] = v & 255;
	d[1] = (v >> 8) & 255;
	d[2] = (v >> 16) & 255;
	d[3] = (v >> 24) & 255;
}

#elif defined(T_BIG_ENDIAN)

uint32_t unpack_1(const void* T_RESTRICT p)
{
	const uint8_t* T_RESTRICT c = static_cast< const uint8_t* T_RESTRICT >(p);
	return c[0];
}

uint32_t unpack_2(const void* T_RESTRICT p)
{
	const uint8_t* T_RESTRICT c = static_cast< const uint8_t* T_RESTRICT >(p);
	return (c[0] << 8) | c[1];
}

uint32_t unpack_3(const void* T_RESTRICT p)
{
	const uint8_t* T_RESTRICT c = static_cast< const uint8_t* T_RESTRICT >(p);
	return (c[0] << 16) | (c[1] << 8) | c[2];
}

uint32_t unpack_4(const void* T_RESTRICT p)
{
	const uint8_t* T_RESTRICT c = static_cast< const uint8_t* T_RESTRICT >(p);
	return (c[0] << 24) | (c[1] << 16) | (c[2] << 8) | c[3];
}

void pack_1(void* T_RESTRICT p, uint32_t v)
{
	uint8_t* T_RESTRICT d = static_cast< uint8_t* T_RESTRICT >(p);
	d[0] = v & 255;
}

void pack_2(void* T_RESTRICT p, uint32_t v)
{
	uint8_t* T_RESTRICT d = static_cast< uint8_t* T_RESTRICT >(p);
	d[0] = (v >> 8) & 255;
	d[1] = v & 255;
}

void pack_3(void* T_RESTRICT p, uint32_t v)
{
	uint8_t* T_RESTRICT d = static_cast< uint8_t* T_RESTRICT >(p);
	d[0] = (v >> 16) & 255;
	d[1] = (v >> 8) & 255;
	d[2] = v & 255;
}

void pack_4(void* T_RESTRICT p, uint32_t v)
{
	uint8_t* T_RESTRICT d = static_cast< uint8_t* T_RESTRICT >(p);
	d[0] = (v >> 24) & 255;
	d[1] = (v >> 16) & 255;
	d[2] = (v >> 8) & 255;
	d[3] = v & 255;
}

#endif

		}

PixelFormat::PixelFormat()
:	m_palettized(false)
,	m_floatPoint(false)
,	m_colorBits(32)
,	m_byteSize(4)
,	m_redBits(8)
,	m_redShift(16)
,	m_greenBits(8)
,	m_greenShift(8)
,	m_blueBits(8)
,	m_blueShift(0)
,	m_alphaBits(8)
,	m_alphaShift(24)
,	m_unpack(0)
,	m_pack(0)
{
	// Get pointer to unpack operation.
	switch (m_byteSize)
	{
	case 1:
		m_unpack = &unpack_1;
		break;
	case 2:
		m_unpack = &unpack_2;
		break;
	case 3:
		m_unpack = &unpack_3;
		break;
	case 4:
		m_unpack = &unpack_4;
		break;
	}

	// Get pointer to pack operation.
	switch (m_byteSize)
	{
	case 1:
		m_pack = &pack_1;
		break;
	case 2:
		m_pack = &pack_2;
		break;
	case 3:
		m_pack = &pack_3;
		break;
	case 4:
		m_pack = &pack_4;
		break;
	}
}

PixelFormat::PixelFormat(
	int colorBits,
	uint32_t redBits,
	uint32_t redShift,
	uint32_t greenBits,
	uint32_t greenShift,
	uint32_t blueBits,
	uint32_t blueShift,
	uint32_t alphaBits,
	uint32_t alphaShift,
	bool palettized,
	bool floatPoint
)
:	m_palettized(palettized)
,	m_floatPoint(floatPoint)
,	m_colorBits(colorBits)
,	m_byteSize((colorBits + 7) / 8)
,	m_redBits(redBits)
,	m_redShift(redShift)
,	m_greenBits(greenBits)
,	m_greenShift(greenShift)
,	m_blueBits(blueBits)
,	m_blueShift(blueShift)
,	m_alphaBits(alphaBits)
,	m_alphaShift(alphaShift)
,	m_unpack(0)
,	m_pack(0)
{
	// Get pointer to unpack operation.
	switch (m_byteSize)
	{
	case 1:
		m_unpack = &unpack_1;
		break;
	case 2:
		m_unpack = &unpack_2;
		break;
	case 3:
		m_unpack = &unpack_3;
		break;
	case 4:
		m_unpack = &unpack_4;
		break;
	}

	// Get pointer to pack operation.
	switch (m_byteSize)
	{
	case 1:
		m_pack = &pack_1;
		break;
	case 2:
		m_pack = &pack_2;
		break;
	case 3:
		m_pack = &pack_3;
		break;
	case 4:
		m_pack = &pack_4;
		break;
	}
}
		
PixelFormat::PixelFormat(
	int colorBits,
	uint32_t redMask,
	uint32_t greenMask,
	uint32_t blueMask,
	uint32_t alphaMask,
	bool palettized,
	bool floatPoint
)
:	m_palettized(palettized)
,	m_floatPoint(floatPoint)
,	m_colorBits(colorBits)
,	m_byteSize((colorBits + 7) / 8)
,	m_redBits(0)
,	m_redShift(0)
,	m_greenBits(0)
,	m_greenShift(0)
,	m_blueBits(0)
,	m_blueShift(0)
,	m_alphaBits(0)
,	m_alphaShift(0)
,	m_unpack(0)
,	m_pack(0)
{
	// Calculate shift and bits from masks.
	for (int i = 0; i < colorBits; ++i)
	{
		uint32_t bit = 1 << i;
		if ((redMask & bit) == bit)
		{
			if (m_redBits++ == 0)
				m_redShift = i;
		}
		if ((greenMask & bit) == bit)
		{
			if (m_greenBits++ == 0)
				m_greenShift = i;
		}
		if ((blueMask & bit) == bit)
		{
			if (m_blueBits++ == 0)
				m_blueShift = i;
		}
		if ((alphaMask & bit) == bit)
		{
			if (m_alphaBits++ == 0)
				m_alphaShift = i;
		}
	}

	// Get pointer to unpack operation.
	switch (m_byteSize)
	{
	case 1:
		m_unpack = &unpack_1;
		break;
	case 2:
		m_unpack = &unpack_2;
		break;
	case 3:
		m_unpack = &unpack_3;
		break;
	case 4:
		m_unpack = &unpack_4;
		break;
	}

	// Get pointer to pack operation.
	switch (m_byteSize)
	{
	case 1:
		m_pack = &pack_1;
		break;
	case 2:
		m_pack = &pack_2;
		break;
	case 3:
		m_pack = &pack_3;
		break;
	case 4:
		m_pack = &pack_4;
		break;
	}
}

void PixelFormat::convert(
	const Palette* srcPalette,
	const void* T_RESTRICT srcPixels,
	const PixelFormat& dstFormat,
	const Palette* dstPalette,
	void* T_RESTRICT dstPixels,
	int pixelCount
) const
{
	const uint8_t* T_RESTRICT src = static_cast< const uint8_t* T_RESTRICT >(srcPixels);
	uint8_t* T_RESTRICT dst = static_cast< uint8_t* T_RESTRICT >(dstPixels);
	uint32_t tmp;
	uint32_t i;
	float clr[4];

	// Quick path; if source and destination are <=32 bit formats.
	if (
		!isPalettized() && !isFloatPoint() && getColorBits() <= 32 &&
		!dstFormat.isPalettized() && !dstFormat.isFloatPoint() && dstFormat.getColorBits() <= 32
	)
	{
		uint32_t srb = 8 - getRedBits();
		uint32_t sgb = 8 - getGreenBits();
		uint32_t sbb = 8 - getBlueBits();
		uint32_t sab = 8 - getAlphaBits();

		uint32_t drb = 8 - dstFormat.getRedBits();
		uint32_t dgb = 8 - dstFormat.getGreenBits();
		uint32_t dbb = 8 - dstFormat.getBlueBits();
		uint32_t dab = 8 - dstFormat.getAlphaBits();

		for (int ii = 0; ii < pixelCount; ++ii)
		{
			uint32_t s = (*m_unpack)(src);

			uint32_t r = s >> getRedShift();
			uint32_t g = s >> getGreenShift();
			uint32_t b = s >> getBlueShift();
			uint32_t a = s >> getAlphaShift();

			r = (r << srb) & 255;
			g = (g << sgb) & 255;
			b = (b << sbb) & 255;
			a = (a << sab) & 255;

			r = r >> drb;
			g = g >> dgb;
			b = b >> dbb;
			a = a >> dab;

			(*dstFormat.m_pack)(
				dst,
				(r << dstFormat.getRedShift()) |
				(g << dstFormat.getGreenShift()) |
				(b << dstFormat.getBlueShift()) |
				(a << dstFormat.getAlphaShift())
			);

			src += getByteSize();
			dst += dstFormat.getByteSize();
		}
	}
	else
	{
		// Convert pixels.
		for (int ii = 0; ii < pixelCount; ++ii)
		{
			// src => rgba
			if (isPalettized())
			{
				uint32_t s = (*m_unpack)(src);
				srcPalette->get(s).storeUnaligned(clr);
			}
			else if (isFloatPoint())
			{
				clr[0] = *(const float *)&src[getRedShift()   >> 3];
				clr[1] = *(const float *)&src[getGreenShift() >> 3];
				clr[2] = *(const float *)&src[getBlueShift()  >> 3];
				clr[3] = *(const float *)&src[getAlphaShift() >> 3];
			}
			else if (getColorBits() <= 32)
			{
				uint32_t s = (*m_unpack)(src);

				uint32_t rmx = ((1 << getRedBits()  ) - 1);
				uint32_t gmx = ((1 << getGreenBits()) - 1);
				uint32_t bmx = ((1 << getBlueBits() ) - 1);
				uint32_t amx = ((1 << getAlphaBits()) - 1);

				uint32_t r = (s >> getRedShift()) & rmx;
				uint32_t g = (s >> getGreenShift()) & gmx;
				uint32_t b = (s >> getBlueShift()) & bmx;
				uint32_t a = (s >> getAlphaShift()) & amx;

				clr[0] = rmx ? (float(r) / rmx) : 0.0f;
				clr[1] = gmx ? (float(g) / gmx) : 0.0f;
				clr[2] = bmx ? (float(b) / bmx) : 0.0f;
				clr[3] = amx ? (float(a) / amx) : 0.0f;
			}
			else	// getColorBits() > 32
			{
				for (tmp = i = 0; i < uint32_t(getRedBits()); ++i)
				{
					uint32_t o = i + getRedShift();
					if ((src[o >> 3] & (1 << (o & 7))) != 0)
						tmp |= 1 << (i + 8 - getRedBits());
				}
				clr[0] = tmp / 255.0f;

				for (tmp = i = 0; i < uint32_t(getGreenBits()); ++i)
				{
					uint32_t o = i + getGreenShift();
					if ((src[o >> 3] & (1 << (o & 7))) != 0)
						tmp |= 1 << (i + 8 - getGreenBits());
				}
				clr[1] = tmp / 255.0f;

				for (tmp = i = 0; i < uint32_t(getBlueBits()); ++i)
				{
					uint32_t o = i + getBlueShift();
					if ((src[o >> 3] & (1 << (o & 7))) != 0)
						tmp |= 1 << (i + 8 - getBlueBits());
				}
				clr[2] = tmp / 255.0f;

				for (tmp = i = 0; i < uint32_t(getAlphaBits()); ++i)
				{
					uint32_t o = i + getAlphaShift();
					if ((src[o >> 3] & (1 << (o & 7))) != 0)
						tmp |= 1 << (i + 8 - getAlphaBits());
				}
				clr[3] = tmp / 255.0f;
			}

			// rgba => dst
			if (dstFormat.isPalettized())
			{
				(*dstFormat.m_pack)(
					dst,
					dstPalette->find(Color4f::loadUnaligned(clr))
				);
			}
			else if (dstFormat.isFloatPoint())
			{
				*(float *)&dst[dstFormat.getRedShift()   >> 3] = clr[0];
				*(float *)&dst[dstFormat.getGreenShift() >> 3] = clr[1];
				*(float *)&dst[dstFormat.getBlueShift()  >> 3] = clr[2];
				*(float *)&dst[dstFormat.getAlphaShift() >> 3] = clr[3];
			}
			else if (dstFormat.getColorBits() <= 32)
			{
				uint32_t rmx = ((1 << dstFormat.getRedBits()  ) - 1);
				uint32_t gmx = ((1 << dstFormat.getGreenBits()) - 1);
				uint32_t bmx = ((1 << dstFormat.getBlueBits() ) - 1);
				uint32_t amx = ((1 << dstFormat.getAlphaBits()) - 1);

				uint32_t r = uint32_t(clamp(clr[0]) * rmx);
				uint32_t g = uint32_t(clamp(clr[1]) * gmx);
				uint32_t b = uint32_t(clamp(clr[2]) * bmx);
				uint32_t a = uint32_t(clamp(clr[3]) * amx);

				(*dstFormat.m_pack)(
					dst,
					(r << dstFormat.getRedShift()) |
					(g << dstFormat.getGreenShift()) |
					(b << dstFormat.getBlueShift()) |
					(a << dstFormat.getAlphaShift())
				);
			}
			else	// dstFormat.getColorBits() > 32
			{
				tmp = static_cast< uint8_t >(clamp(clr[0]) * 255);
				tmp >>= (8 - dstFormat.getRedBits());
				for (i = 0; i < uint32_t(dstFormat.getRedBits()); ++i)
				{
					uint32_t o = i + dstFormat.getRedShift();
					if ((tmp & (1 << i)) != 0)
						dst[o >> 3] |= 1 << (o & 7);
					else
						dst[o >> 3] &= ~(1 << (o & 7));
				}
			
				tmp = static_cast< uint8_t >(clamp(clr[1]) * 255);
				tmp >>= (8 - dstFormat.getGreenBits());
				for (i = 0; i < uint32_t(dstFormat.getGreenBits()); ++i)
				{
					uint32_t o = i + dstFormat.getGreenShift();
					if ((tmp & (1 << i)) != 0)
						dst[o >> 3] |= 1 << (o & 7);
					else
						dst[o >> 3] &= ~(1 << (o & 7));
				}
				
				tmp = static_cast< uint8_t >(clamp(clr[2]) * 255);
				tmp >>= (8 - dstFormat.getBlueBits());
				for (i = 0; i < uint32_t(dstFormat.getBlueBits()); ++i)
				{
					uint32_t o = i + dstFormat.getBlueShift();
					if ((tmp & (1 << i)) != 0)
						dst[o >> 3] |= 1 << (o & 7);
					else
						dst[o >> 3] &= ~(1 << (o & 7));
				}
				
				tmp = static_cast< uint8_t >(clamp(clr[3]) * 255);
				tmp >>= (8 - dstFormat.getAlphaBits());
				for (i = 0; i < uint32_t(dstFormat.getAlphaBits()); ++i)
				{
					uint32_t o = i + dstFormat.getAlphaShift();
					if ((tmp & (1 << i)) != 0)
						dst[o >> 3] |= 1 << (o & 7);
					else
						dst[o >> 3] &= ~(1 << (o & 7));
				}
			}

			// Next pixel
			src += getByteSize();
			dst += dstFormat.getByteSize();
		}
	}
}

bool PixelFormat::operator == (const PixelFormat& pf) const
{
	if (m_palettized != pf.m_palettized)
		return false;
	if (m_floatPoint != pf.m_floatPoint)
		return false;
	if (m_colorBits != pf.m_colorBits)
		return false;
	if (m_byteSize != pf.m_byteSize)
		return false;
	if (m_redBits != pf.m_redBits)
		return false;
	if (m_redShift != pf.m_redShift)
		return false;
	if (m_greenBits != pf.m_greenBits)
		return false;
	if (m_greenShift != pf.m_greenShift)
		return false;
	if (m_blueBits != pf.m_blueBits)
		return false;
	if (m_blueShift != pf.m_blueShift)
		return false;
	if (m_alphaBits != pf.m_alphaBits)
		return false;
	if (m_alphaShift != pf.m_alphaShift)
		return false;

	return true;
}

bool PixelFormat::operator != (const PixelFormat& pf) const
{
	return !(*this == pf);
}

const PixelFormat PixelFormat::ms_pfP4(4, 0, 0, 0, 0, true, false);
const PixelFormat PixelFormat::ms_pfP8(8, 0, 0, 0, 0, true, false);
const PixelFormat PixelFormat::ms_pfA8(8, 0, 0, 0, 0xff, false, false);
const PixelFormat PixelFormat::ms_pfR5G5B5(15, 0x7c00, 0x03e0, 0x001f, 0, false, false);
const PixelFormat PixelFormat::ms_pfR5G6B5(16, 0xf800, 0x07e0, 0x001f, 0, false, false);
const PixelFormat PixelFormat::ms_pfR8G8B8(24, 0xff0000, 0x00ff00, 0x0000ff, 0, false, false);
const PixelFormat PixelFormat::ms_pfB8G8R8(24, 0x0000ff, 0x00ff00, 0xff0000, 0, false, false);
const PixelFormat PixelFormat::ms_pfA1R5G5B5(16, 0x7c00, 0x03e0, 0x001f, 0x8000, false, false);
const PixelFormat PixelFormat::ms_pfX8R8G8B8(32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0, false, false);
const PixelFormat PixelFormat::ms_pfX8B8G8R8(32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0, false, false);
const PixelFormat PixelFormat::ms_pfR8G8B8X8(32, 0xff000000, 0x00ff0000, 0x0000ff00, 0, false, false);
const PixelFormat PixelFormat::ms_pfB8G8R8X8(32, 0x0000ff00, 0x00ff0000, 0xff000000, 0, false, false);
const PixelFormat PixelFormat::ms_pfA8R8G8B8(32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000, false, false);
const PixelFormat PixelFormat::ms_pfA8B8G8R8(32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000, false, false);
const PixelFormat PixelFormat::ms_pfR8G8B8A8(32, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff, false, false);
const PixelFormat PixelFormat::ms_pfB8G8R8A8(32, 0x0000ff00, 0x00ff0000, 0xff000000, 0x000000ff, false, false);
const PixelFormat PixelFormat::ms_pfRGBAF32(128, 32, 0, 32, 32, 32, 64, 32, 96, false, true);

	}
}
