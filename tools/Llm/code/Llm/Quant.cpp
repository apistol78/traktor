/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Llm/Quant.h"

#include "Core/Math/Half.h"
#include "Core/Math/Vector4.h"

#include <algorithm>
#include <cstring>

namespace traktor::llm
{
namespace
{

const uint32_t c_superBlockSize = 256;

/*! Read a little endian half from possibly unaligned storage. */
float readHalf(const uint8_t* p)
{
	uint16_t bits;
	std::memcpy(&bits, p, sizeof(bits));
	return halfToFloat((half_t)bits);
}

/*! Q4_0: 32 elements per block, 18 bytes.
 *
 *   half    d            scale
 *   uint8   qs[16]       two nibbles per byte
 *
 * Element i (0..15) is the low nibble of qs[i], element i + 16 the high
 * nibble; both are biased by -8 and scaled by d.
 */
void dequantizeQ4_0(const uint8_t* src, float* dst, uint32_t blocks)
{
	for (uint32_t b = 0; b < blocks; ++b)
	{
		const float d = readHalf(src);
		const uint8_t* qs = src + 2;

		for (uint32_t i = 0; i < 16; ++i)
		{
			dst[i] = d * (float)((int32_t)(qs[i] & 0x0f) - 8);
			dst[i + 16] = d * (float)((int32_t)(qs[i] >> 4) - 8);
		}

		src += 18;
		dst += 32;
	}
}

/*! Q8_0: 32 elements per block, 34 bytes.
 *
 *   half    d            scale
 *   int8    qs[32]
 */
void dequantizeQ8_0(const uint8_t* src, float* dst, uint32_t blocks)
{
	for (uint32_t b = 0; b < blocks; ++b)
	{
		const float d = readHalf(src);
		const int8_t* qs = (const int8_t*)(src + 2);

		for (uint32_t i = 0; i < 32; ++i)
			dst[i] = d * (float)qs[i];

		src += 34;
		dst += 32;
	}
}

/*! Unpack one of the eight 6-bit scale/min pairs of a Q4_K block.
 *
 * The twelve scale bytes hold eight 6-bit scales and eight 6-bit mins.
 * The first four pairs live in the low six bits of bytes 0..7; the last
 * four are split, taking their low nibble from bytes 8..11 and their top
 * two bits from the unused high bits of bytes 0..7.
 */
void unpackScaleMinK4(uint32_t j, const uint8_t* q, uint8_t& outScale, uint8_t& outMin)
{
	if (j < 4)
	{
		outScale = q[j] & 63;
		outMin = q[j + 4] & 63;
	}
	else
	{
		outScale = (q[j + 4] & 0x0f) | ((q[j - 4] >> 6) << 4);
		outMin = (q[j + 4] >> 4) | ((q[j] >> 6) << 4);
	}
}

/*! Q4_K: 256 elements per super block, 144 bytes.
 *
 *   half    d            scale of the quantized scales
 *   half    dmin         scale of the quantized mins
 *   uint8   scales[12]   eight 6-bit scales and eight 6-bit mins
 *   uint8   qs[128]      eight sub blocks of 32 nibbles
 *
 * Each of the eight sub blocks reconstructs as q * (d * scale) - dmin * min.
 */
void dequantizeQ4_K(const uint8_t* src, float* dst, uint32_t blocks)
{
	for (uint32_t b = 0; b < blocks; ++b)
	{
		const float d = readHalf(src);
		const float dmin = readHalf(src + 2);
		const uint8_t* scales = src + 4;
		const uint8_t* qs = src + 16;

		// Two sub blocks share each byte of qs, so step through them in pairs.
		for (uint32_t j = 0; j < 8; j += 2)
		{
			uint8_t sc, m;

			unpackScaleMinK4(j, scales, sc, m);
			const float d1 = d * (float)sc;
			const float m1 = dmin * (float)m;

			unpackScaleMinK4(j + 1, scales, sc, m);
			const float d2 = d * (float)sc;
			const float m2 = dmin * (float)m;

			for (uint32_t i = 0; i < 32; ++i)
			{
				dst[i] = d1 * (float)(qs[i] & 0x0f) - m1;
				dst[i + 32] = d2 * (float)(qs[i] >> 4) - m2;
			}

			qs += 32;
			dst += 64;
		}

		src += 144;
	}
}

/*! Q6_K: 256 elements per super block, 210 bytes.
 *
 *   uint8   ql[128]      low four bits of each quant
 *   uint8   qh[64]       high two bits of each quant
 *   int8    scales[16]   one signed scale per sixteen elements
 *   half    d            super block scale
 *
 * Quants are six bits biased by -32; the super block is processed in two
 * halves of 128 elements, each half consuming 64 ql, 32 qh and 8 scales.
 */
void dequantizeQ6_K(const uint8_t* src, float* dst, uint32_t blocks)
{
	for (uint32_t b = 0; b < blocks; ++b)
	{
		const uint8_t* ql = src;
		const uint8_t* qh = src + 128;
		const int8_t* scales = (const int8_t*)(src + 192);
		const float d = readHalf(src + 208);

		for (uint32_t n = 0; n < c_superBlockSize; n += 128)
		{
			for (uint32_t i = 0; i < 32; ++i)
			{
				const uint32_t is = i / 16;
				const int32_t q1 = (int32_t)((ql[i] & 0x0f) | (((qh[i] >> 0) & 3) << 4)) - 32;
				const int32_t q2 = (int32_t)((ql[i + 32] & 0x0f) | (((qh[i] >> 2) & 3) << 4)) - 32;
				const int32_t q3 = (int32_t)((ql[i] >> 4) | (((qh[i] >> 4) & 3) << 4)) - 32;
				const int32_t q4 = (int32_t)((ql[i + 32] >> 4) | (((qh[i] >> 6) & 3) << 4)) - 32;

				dst[i] = d * (float)scales[is] * (float)q1;
				dst[i + 32] = d * (float)scales[is + 2] * (float)q2;
				dst[i + 64] = d * (float)scales[is + 4] * (float)q3;
				dst[i + 96] = d * (float)scales[is + 6] * (float)q4;
			}

			dst += 128;
			ql += 64;
			qh += 32;
			scales += 8;
		}

		src += 210;
	}
}

void dequantizeF16(const uint8_t* src, float* dst, uint32_t count)
{
	for (uint32_t i = 0; i < count; ++i)
		dst[i] = readHalf(src + i * 2);
}

/*! Decode \a blocks whole blocks; \a dst must hold blocks * blockSize floats. */
void dequantizeBlocks(GgmlType type, const uint8_t* src, float* dst, uint32_t blocks)
{
	switch (type)
	{
	case GgmlType::F32:
		std::memcpy(dst, src, blocks * sizeof(float));
		break;

	case GgmlType::F16:
		dequantizeF16(src, dst, blocks);
		break;

	case GgmlType::Q4_0:
		dequantizeQ4_0(src, dst, blocks);
		break;

	case GgmlType::Q8_0:
		dequantizeQ8_0(src, dst, blocks);
		break;

	case GgmlType::Q4_K:
		dequantizeQ4_K(src, dst, blocks);
		break;

	case GgmlType::Q6_K:
		dequantizeQ6_K(src, dst, blocks);
		break;

	default:
		break;
	}
}

}

void dequantize(GgmlType type, const void* src, float* dst, uint32_t count)
{
	const uint32_t blockSize = getBlockSize(type);
	if (blockSize == 0 || !isSupported(type))
		return;
	dequantizeBlocks(type, (const uint8_t*)src, dst, count / blockSize);
}

float dotRow(GgmlType type, const void* row, const float* v, uint32_t count)
{
	// F32 rows need no staging at all.
	if (type == GgmlType::F32)
		return dotF32((const float*)row, v, count);

	const uint32_t blockSize = getBlockSize(type);
	const uint32_t blockBytes = getBlockBytes(type);
	if (blockSize == 0 || !isSupported(type))
		return 0.0f;

	// Decode a bounded window of the row at a time so a row of any length
	// stays in cache and never needs a heap allocation.
	const uint32_t windowBlocks = (c_superBlockSize + blockSize - 1) / blockSize;
	const uint32_t windowSize = windowBlocks * blockSize;

	T_ALIGN16 float window[c_superBlockSize + 32];
	const uint8_t* src = (const uint8_t*)row;

	float sum = 0.0f;
	for (uint32_t i = 0; i < count; i += windowSize)
	{
		const uint32_t n = std::min(windowSize, count - i);
		const uint32_t blocks = (n + blockSize - 1) / blockSize;

		dequantizeBlocks(type, src, window, blocks);
		sum += dotF32(window, v + i, n);

		src += blocks * blockBytes;
	}

	return sum;
}

float dotF32(const float* a, const float* b, uint32_t count)
{
	Vector4 acc = Vector4::zero();

	uint32_t i = 0;
	for (; i + 4 <= count; i += 4)
	{
		const Vector4 va = Vector4::loadUnaligned(a + i);
		const Vector4 vb = Vector4::loadUnaligned(b + i);
		acc += va * vb;
	}

	float sum = (float)horizontalAdd4(acc);
	for (; i < count; ++i)
		sum += a[i] * b[i];

	return sum;
}

}
