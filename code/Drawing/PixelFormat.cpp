/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#if defined(_MSC_VER) && !defined(_XBOX)
#	define USE_XMM_INTRINSICS
#	include <emmintrin.h>
#elif defined(__APPLE__)
#	include <TargetConditionals.h>
#	if TARGET_CPU_X86 && TARGET_OS_MAC && !TARGET_OS_IPHONE
#		define USE_XMM_INTRINSICS
#		include <emmintrin.h>
#	endif
#endif

#include "Core/Math/Half.h"
#include "Core/Math/MathUtils.h"
#include "Drawing/PixelFormat.h"
#include "Drawing/Palette.h"

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

float unpack_fp(const void* T_RESTRICT p, uint32_t nbits)
{
	if (nbits == 16)
		return halfToFloat(*(const half_t*)p);
	else if (nbits == 32)
		return *(const float*)p;
	else
		return 0.0f;
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

void pack_fp(void* T_RESTRICT p, float v, uint32_t nbits)
{
	if (nbits == 16)
		*(half_t* T_RESTRICT)p = floatToHalf(v);
	else if (nbits == 32)
		*(float* T_RESTRICT)p = v;
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

float unpack_fp(const void* T_RESTRICT p, uint32_t nbits)
{
	if (nbits == 16)
		return halfToFloat(*(const half_t*)p);
	else if (nbits == 32)
		return *(const float*)p;
	else
		return 0.0f;
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

void pack_fp(void* T_RESTRICT p, float v, uint32_t nbits)
{
	if (nbits == 16)
		*(half_t* T_RESTRICT)p = floatToHalf(v);
	else if (nbits == 32)
		*(float* T_RESTRICT)p = v;
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

	bool isSourcePacked32 = !isPalettized() && !isFloatPoint() && getColorBits() <= 32;
	bool isDestinationPacked32 = !dstFormat.isPalettized() && !dstFormat.isFloatPoint() && dstFormat.getColorBits() <= 32;

	// Quick path 1; if source and destination are <=32 bit formats.
	if (isSourcePacked32 && isDestinationPacked32)
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

	// Quick-path 2; if source format <=32 and destination format is FP
	else if (isSourcePacked32 && dstFormat.isFloatPoint())
	{
		uint32_t rmx = ((1 << getRedBits()  ) - 1);
		uint32_t gmx = ((1 << getGreenBits()) - 1);
		uint32_t bmx = ((1 << getBlueBits() ) - 1);
		uint32_t amx = ((1 << getAlphaBits()) - 1);

		float ir = rmx ? (1.0f / rmx) : 0.0f;
		float ig = rmx ? (1.0f / gmx) : 0.0f;
		float ib = rmx ? (1.0f / bmx) : 0.0f;
		float ia = rmx ? (1.0f / amx) : 0.0f;

		for (int ii = 0; ii < pixelCount; ++ii)
		{
			uint32_t s = (*m_unpack)(src);

			uint32_t r = (s >> getRedShift()) & rmx;
			uint32_t g = (s >> getGreenShift()) & gmx;
			uint32_t b = (s >> getBlueShift()) & bmx;
			uint32_t a = (s >> getAlphaShift()) & amx;

			pack_fp(&dst[dstFormat.getRedShift() >> 3], r * ir, dstFormat.getRedBits());
			pack_fp(&dst[dstFormat.getGreenShift() >> 3], g * ig, dstFormat.getGreenBits());
			pack_fp(&dst[dstFormat.getBlueShift() >> 3], b * ib, dstFormat.getBlueBits());
			pack_fp(&dst[dstFormat.getAlphaShift() >> 3], a * ia, dstFormat.getAlphaBits());

			src += getByteSize();
			dst += dstFormat.getByteSize();
		}
	}

	// Quick-path 3; if source format is FP and destination format is <=32
	else if (isFloatPoint() && isDestinationPacked32)
	{
		uint32_t rmx = ((1 << dstFormat.getRedBits()  ) - 1);
		uint32_t gmx = ((1 << dstFormat.getGreenBits()) - 1);
		uint32_t bmx = ((1 << dstFormat.getBlueBits() ) - 1);
		uint32_t amx = ((1 << dstFormat.getAlphaBits()) - 1);

		for (int ii = 0; ii < pixelCount; ++ii)
		{
			float rf = unpack_fp(&src[getRedShift() >> 3], getRedBits());
			float gf = unpack_fp(&src[getGreenShift() >> 3], getGreenBits());
			float bf = unpack_fp(&src[getBlueShift() >> 3], getBlueBits());
			float af = unpack_fp(&src[getAlphaShift() >> 3], getAlphaBits());

			uint32_t r = uint32_t(clamp(rf) * rmx);
			uint32_t g = uint32_t(clamp(gf) * gmx);
			uint32_t b = uint32_t(clamp(bf) * bmx);
			uint32_t a = uint32_t(clamp(af) * amx);

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

	// Universal path.
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
				clr[0] = unpack_fp(&src[getRedShift() >> 3], getRedBits());
				clr[1] = unpack_fp(&src[getGreenShift() >> 3], getGreenBits());
				clr[2] = unpack_fp(&src[getBlueShift() >> 3], getBlueBits());
				clr[3] = unpack_fp(&src[getAlphaShift() >> 3], getAlphaBits());
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
				pack_fp(&dst[dstFormat.getRedShift() >> 3], clr[0], dstFormat.getRedBits());
				pack_fp(&dst[dstFormat.getGreenShift() >> 3], clr[1], dstFormat.getGreenBits());
				pack_fp(&dst[dstFormat.getBlueShift() >> 3], clr[2], dstFormat.getBlueBits());
				pack_fp(&dst[dstFormat.getAlphaShift() >> 3], clr[3], dstFormat.getAlphaBits());
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

void PixelFormat::convertTo4f(
	const Palette* srcPalette,
	const void* T_RESTRICT srcPixels,
	Color4f* T_RESTRICT dstPixels,
	int srcPixelPitch,
	int pixelCount
) const
{
	const uint8_t* T_RESTRICT src = static_cast< const uint8_t* T_RESTRICT >(srcPixels);
	Color4f* T_RESTRICT dst = dstPixels;

	if (!isPalettized() && !isFloatPoint() && getColorBits() <= 32)
	{
		uint32_t rmx = (1 << getRedBits()  ) - 1;
		uint32_t gmx = (1 << getGreenBits()) - 1;
		uint32_t bmx = (1 << getBlueBits() ) - 1;
		uint32_t amx = (1 << getAlphaBits()) - 1;

		const float T_MATH_ALIGN16 finv[] =
		{
			rmx ? (1.0f / rmx) : 0.0f,
			gmx ? (1.0f / gmx) : 0.0f,
			bmx ? (1.0f / bmx) : 0.0f,
			amx ? (1.0f / amx) : 0.0f
		};

		const Color4f inv(Vector4::loadAligned(finv));
		int ii = 0;

#if defined(USE_XMM_INTRINSICS)
		__m128i mx = _mm_set_epi32(amx, bmx, gmx, rmx);

		// Do four horizontal pixels at a time thus cannot pitch.
		if (srcPixelPitch <= 1)
		{
			for (; ii < pixelCount - 4; ii += 4)
			{
				uint32_t s0 = (*m_unpack)(src + 0);
				uint32_t s1 = (*m_unpack)(src + 1 * getByteSize());
				uint32_t s2 = (*m_unpack)(src + 2 * getByteSize());
				uint32_t s3 = (*m_unpack)(src + 3 * getByteSize());

				__m128i t0_0 = _mm_set_epi32(s0 >> getAlphaShift(), s0 >> getBlueShift(), s0 >> getGreenShift(), s0 >> getRedShift());
				__m128i t0_1 = _mm_set_epi32(s1 >> getAlphaShift(), s1 >> getBlueShift(), s1 >> getGreenShift(), s1 >> getRedShift());
				__m128i t0_2 = _mm_set_epi32(s2 >> getAlphaShift(), s2 >> getBlueShift(), s2 >> getGreenShift(), s2 >> getRedShift());
				__m128i t0_3 = _mm_set_epi32(s3 >> getAlphaShift(), s3 >> getBlueShift(), s3 >> getGreenShift(), s3 >> getRedShift());

				__m128i t1_0 = _mm_and_si128(t0_0, mx);
				__m128i t1_1 = _mm_and_si128(t0_1, mx);
				__m128i t1_2 = _mm_and_si128(t0_2, mx);
				__m128i t1_3 = _mm_and_si128(t0_3, mx);

				__m128 fp_0 = _mm_cvtepi32_ps(t1_0);
				__m128 fp_1 = _mm_cvtepi32_ps(t1_1);
				__m128 fp_2 = _mm_cvtepi32_ps(t1_2);
				__m128 fp_3 = _mm_cvtepi32_ps(t1_3);

				*dst++ = Color4f(Vector4(fp_0)) * inv;
				*dst++ = Color4f(Vector4(fp_1)) * inv;
				*dst++ = Color4f(Vector4(fp_2)) * inv;
				*dst++ = Color4f(Vector4(fp_3)) * inv;

				src += getByteSize() * 4;
			}
		}
#endif

		for (; ii < pixelCount; ++ii)
		{
			uint32_t s = (*m_unpack)(src);

#if defined(USE_XMM_INTRINSICS)
			__m128i t0 = _mm_set_epi32(s >> getAlphaShift(), s >> getBlueShift(), s >> getGreenShift(), s >> getRedShift());
			__m128i t1 = _mm_and_si128(t0, mx);
			__m128 fp = _mm_cvtepi32_ps(t1);
			*dst = Color4f(Vector4(fp)) * inv;
#else
			uint32_t r = (s >> getRedShift()) & rmx;
			uint32_t g = (s >> getGreenShift()) & gmx;
			uint32_t b = (s >> getBlueShift()) & bmx;
			uint32_t a = (s >> getAlphaShift()) & amx;
			*dst = Color4f(
				float(r),
				float(g),
				float(b),
				float(a)
			) * inv;
#endif
			src += srcPixelPitch * getByteSize();
			dst++;
		}
	}
	else
	{
		float T_MATH_ALIGN16 clr[4];
		uint32_t tmp;
		uint32_t i;

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
				clr[0] = unpack_fp(&src[getRedShift() >> 3], getRedBits());
				clr[1] = unpack_fp(&src[getGreenShift() >> 3], getGreenBits());
				clr[2] = unpack_fp(&src[getBlueShift() >> 3], getBlueBits());
				clr[3] = unpack_fp(&src[getAlphaShift() >> 3], getAlphaBits());
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

			*dst = Color4f::loadAligned(clr);

			src += srcPixelPitch * getByteSize();
			dst++;
		}
	}
}

void PixelFormat::convertFrom4f(
	const Color4f* T_RESTRICT srcPixels,
	const Palette* dstPalette,
	void* T_RESTRICT dstPixels,
	int dstPixelPitch,
	int pixelCount
) const
{
	const Color4f* T_RESTRICT src = srcPixels;
	uint8_t* T_RESTRICT dst = static_cast< uint8_t* T_RESTRICT >(dstPixels);
	float T_MATH_ALIGN16 clr[4];

	if (!isPalettized() && !isFloatPoint() && getColorBits() <= 32)
	{
		uint32_t rmx = ((1 << getRedBits()  ) - 1);
		uint32_t gmx = ((1 << getGreenBits()) - 1);
		uint32_t bmx = ((1 << getBlueBits() ) - 1);
		uint32_t amx = ((1 << getAlphaBits()) - 1);

		for (int ii = 0; ii < pixelCount; ++ii)
		{
			src->storeAligned(clr);

			uint32_t r = uint32_t(clamp(clr[0]) * rmx);
			uint32_t g = uint32_t(clamp(clr[1]) * gmx);
			uint32_t b = uint32_t(clamp(clr[2]) * bmx);
			uint32_t a = uint32_t(clamp(clr[3]) * amx);

			T_ASSERT (m_pack);
			(*m_pack)(
				dst,
				(r << getRedShift()) |
				(g << getGreenShift()) |
				(b << getBlueShift()) |
				(a << getAlphaShift())
			);

			src++;
			dst += dstPixelPitch * getByteSize();
		}
	}
	else
	{
		for (int ii = 0; ii < pixelCount; ++ii)
		{
			src->storeAligned(clr);

			// rgba => dst
			if (isPalettized())
			{
				T_ASSERT (m_pack);
				(*m_pack)(
					dst,
					dstPalette->find(*src)
				);
			}
			else if (isFloatPoint())
			{
				pack_fp(&dst[getRedShift() >> 3], clr[0], getRedBits());
				pack_fp(&dst[getGreenShift() >> 3], clr[1], getGreenBits());
				pack_fp(&dst[getBlueShift() >> 3], clr[2], getBlueBits());
				pack_fp(&dst[getAlphaShift() >> 3], clr[3], getAlphaBits());
			}
			else if (getColorBits() <= 32)
			{
				uint32_t rmx = ((1 << getRedBits()  ) - 1);
				uint32_t gmx = ((1 << getGreenBits()) - 1);
				uint32_t bmx = ((1 << getBlueBits() ) - 1);
				uint32_t amx = ((1 << getAlphaBits()) - 1);

				uint32_t r = uint32_t(clamp(clr[0]) * rmx);
				uint32_t g = uint32_t(clamp(clr[1]) * gmx);
				uint32_t b = uint32_t(clamp(clr[2]) * bmx);
				uint32_t a = uint32_t(clamp(clr[3]) * amx);

				T_ASSERT (m_pack);
				(*m_pack)(
					dst,
					(r << getRedShift()) |
					(g << getGreenShift()) |
					(b << getBlueShift()) |
					(a << getAlphaShift())
				);
			}
			else	// getColorBits() > 32
			{
				uint32_t tmp;
				uint32_t i;

				tmp = static_cast< uint8_t >(clamp(clr[0]) * 255);
				tmp >>= (8 - getRedBits());
				for (i = 0; i < uint32_t(getRedBits()); ++i)
				{
					uint32_t o = i + getRedShift();
					if ((tmp & (1 << i)) != 0)
						dst[o >> 3] |= 1 << (o & 7);
					else
						dst[o >> 3] &= ~(1 << (o & 7));
				}

				tmp = static_cast< uint8_t >(clamp(clr[1]) * 255);
				tmp >>= (8 - getGreenBits());
				for (i = 0; i < uint32_t(getGreenBits()); ++i)
				{
					uint32_t o = i + getGreenShift();
					if ((tmp & (1 << i)) != 0)
						dst[o >> 3] |= 1 << (o & 7);
					else
						dst[o >> 3] &= ~(1 << (o & 7));
				}

				tmp = static_cast< uint8_t >(clamp(clr[2]) * 255);
				tmp >>= (8 - getBlueBits());
				for (i = 0; i < uint32_t(getBlueBits()); ++i)
				{
					uint32_t o = i + getBlueShift();
					if ((tmp & (1 << i)) != 0)
						dst[o >> 3] |= 1 << (o & 7);
					else
						dst[o >> 3] &= ~(1 << (o & 7));
				}

				tmp = static_cast< uint8_t >(clamp(clr[3]) * 255);
				tmp >>= (8 - getAlphaBits());
				for (i = 0; i < uint32_t(getAlphaBits()); ++i)
				{
					uint32_t o = i + getAlphaShift();
					if ((tmp & (1 << i)) != 0)
						dst[o >> 3] |= 1 << (o & 7);
					else
						dst[o >> 3] &= ~(1 << (o & 7));
				}
			}

			src++;
			dst += dstPixelPitch * getByteSize();
		}
	}
}

PixelFormat PixelFormat::endianSwapped() const
{
	uint32_t nbits = m_byteSize * 8;
	return PixelFormat(
		m_colorBits,
		m_redBits,
		m_redBits > 0 ? nbits - m_redShift - m_redBits : 0,
		m_greenBits,
		m_greenBits > 0 ? nbits - m_greenShift - m_greenBits : 0,
		m_blueBits,
		m_blueBits > 0 ? nbits - m_blueShift - m_blueBits : 0,
		m_alphaBits,
		m_alphaBits > 0 ? nbits - m_alphaShift - m_alphaBits : 0,
		m_palettized,
		m_floatPoint
	);
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
const PixelFormat PixelFormat::ms_pfA8(8, 0x00, 0x00, 0x00, 0xff, false, false);
const PixelFormat PixelFormat::ms_pfR8(8, 0xff, 0x00, 0x00, 0x00, false, false);
const PixelFormat PixelFormat::ms_pfR16(16, 0xffff, 0x0000, 0x0000, 0x0000, false, false);
const PixelFormat PixelFormat::ms_pfR5G5B5(15, 0x7c00, 0x03e0, 0x001f, 0x0000, false, false);
const PixelFormat PixelFormat::ms_pfR5G6B5(16, 0xf800, 0x07e0, 0x001f, 0x0000, false, false);
const PixelFormat PixelFormat::ms_pfR5G5B5A1(16, 0xf800, 0x07d0, 0x003e, 0x0001, false, false);
const PixelFormat PixelFormat::ms_pfR4G4B4A4(16, 0xf000, 0x0f00, 0x00f0, 0x000f, false, false);
const PixelFormat PixelFormat::ms_pfR8G8B8(24, 0xff0000, 0x00ff00, 0x0000ff, 0x000000, false, false);
const PixelFormat PixelFormat::ms_pfB8G8R8(24, 0x0000ff, 0x00ff00, 0xff0000, 0x000000, false, false);
const PixelFormat PixelFormat::ms_pfA1R5G5B5(16, 0x7c00, 0x03e0, 0x001f, 0x8000, false, false);
const PixelFormat PixelFormat::ms_pfX8R8G8B8(32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000, false, false);
const PixelFormat PixelFormat::ms_pfX8B8G8R8(32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0x00000000, false, false);
const PixelFormat PixelFormat::ms_pfR8G8B8X8(32, 0xff000000, 0x00ff0000, 0x0000ff00, 0x00000000, false, false);
const PixelFormat PixelFormat::ms_pfB8G8R8X8(32, 0x0000ff00, 0x00ff0000, 0xff000000, 0x00000000, false, false);
const PixelFormat PixelFormat::ms_pfA8R8G8B8(32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000, false, false);
const PixelFormat PixelFormat::ms_pfA8B8G8R8(32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000, false, false);
const PixelFormat PixelFormat::ms_pfR8G8B8A8(32, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff, false, false);
const PixelFormat PixelFormat::ms_pfB8G8R8A8(32, 0x0000ff00, 0x00ff0000, 0xff000000, 0x000000ff, false, false);
const PixelFormat PixelFormat::ms_pfR16F(16, 16, 0, 0, 0, 0, 0, 0, 0, false, true);
const PixelFormat PixelFormat::ms_pfR32F(32, 32, 0, 0, 0, 0, 0, 0, 0, false, true);
const PixelFormat PixelFormat::ms_pfARGBF16(64, 16, 16, 16, 32, 16, 48, 16, 0, false, true);
const PixelFormat PixelFormat::ms_pfARGBF32(128, 32, 32, 32, 64, 32, 96, 32, 0, false, true);
const PixelFormat PixelFormat::ms_pfRGBAF16(64, 16, 0, 16, 16, 16, 32, 16, 48, false, true);
const PixelFormat PixelFormat::ms_pfRGBAF32(128, 32, 0, 32, 32, 32, 64, 32, 96, false, true);
const PixelFormat PixelFormat::ms_pfABGRF16(64, 16, 48, 16, 32, 16, 16, 16, 0, false, true);
const PixelFormat PixelFormat::ms_pfABGRF32(128, 32, 96, 32, 64, 32, 32, 32, 0, false, true);

	}
}
