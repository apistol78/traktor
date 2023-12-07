/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma warning( disable: 4752 )

#include <immintrin.h>
#include <xmmintrin.h>
#include "Core/Math/Vector4.h"
#include "Core/Misc/Align.h"
#include "Sound/Avx/AudioMixerAvx.h"

#if defined(_MSC_VER)
#	include <intrin.h>
#endif

namespace traktor::sound
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.AudioMixerAvx", AudioMixerAvx, IAudioMixer)

bool AudioMixerAvx::supported()
{
#if defined(_MSC_VER)
#	define OSXSAVEFlag (1UL<<27)
#	define AVXFlag ((1UL<<28)|OSXSAVEFlag)

	int ci[4] = { 0 };
	__cpuid(ci, 1);

	const bool osUsesXSAVE_XRSTORE = ci[2] & (1 << 27) || false;
	const bool cpuAVXSuport = ci[2] & (1 << 28) || false;

	if (osUsesXSAVE_XRSTORE && cpuAVXSuport)
	{
		const unsigned long long xcrFeatureMask = _xgetbv(_XCR_XFEATURE_ENABLED_MASK);
		return (xcrFeatureMask & 0x6) || false;
	}
#endif
	return false;
}

void AudioMixerAvx::mul(float* lsb, const float* rsb, uint32_t count) const
{
	T_ASSERT(alignUp(lsb, 16) == lsb);
	T_ASSERT(alignUp(rsb, 16) == rsb);
	T_ASSERT(alignUp(count, 4) == count);

	int32_t s = 0;

	for (; s < int32_t(count) - 8 * 4; s += 8 * 4)
	{
		const __m256 rs8_0 = _mm256_load_ps(&rsb[s]);
		const __m256 rs8_1 = _mm256_load_ps(&rsb[s + 8]);
		const __m256 rs8_2 = _mm256_load_ps(&rsb[s + 16]);
		const __m256 rs8_3 = _mm256_load_ps(&rsb[s + 24]);

		const __m256 ls8_0 = _mm256_load_ps(&lsb[s]);
		const __m256 ls8_1 = _mm256_load_ps(&lsb[s + 8]);
		const __m256 ls8_2 = _mm256_load_ps(&lsb[s + 16]);
		const __m256 ls8_3 = _mm256_load_ps(&lsb[s + 24]);

		_mm256_store_ps(&lsb[s], _mm256_mul_ps(rs8_0, ls8_0));
		_mm256_store_ps(&lsb[s + 8], _mm256_mul_ps(rs8_1, ls8_1));
		_mm256_store_ps(&lsb[s + 16], _mm256_mul_ps(rs8_2, ls8_2));
		_mm256_store_ps(&lsb[s + 24], _mm256_mul_ps(rs8_3, ls8_3));
	}

	for (; s < int32_t(count) - 8; s += 8)
	{
		const __m256 rs8_0 = _mm256_load_ps(&rsb[s]);
		const __m256 ls8_0 = _mm256_load_ps(&lsb[s]);
		_mm256_store_ps(&lsb[s], _mm256_mul_ps(rs8_0, ls8_0));
	}

	for (; s < int32_t(count); s += 4)
	{
		const Vector4 rs4 = Vector4::loadAligned(&rsb[s]);
		const Vector4 ls4 = Vector4::loadAligned(&lsb[s]);
		(ls4 * rs4).storeAligned(&lsb[s]);
	}
}

void AudioMixerAvx::mulConst(float* sb, uint32_t count, float factor) const
{
	T_ASSERT(alignUp(sb, 16) == sb);
	T_ASSERT(alignUp(count, 4) == count);

	const __m256 sf8 = _mm256_set1_ps(factor);

	const Scalar sf(factor);
	int32_t s = 0;

	for (; s < int32_t(count) - 8 * 4; s += 8 * 4)
	{
		const __m256 s8_0 = _mm256_load_ps(&sb[s]);
		const __m256 s8_1 = _mm256_load_ps(&sb[s + 8]);
		const __m256 s8_2 = _mm256_load_ps(&sb[s + 16]);
		const __m256 s8_3 = _mm256_load_ps(&sb[s + 24]);
		_mm256_store_ps(&sb[s], _mm256_mul_ps(s8_0, sf8));
		_mm256_store_ps(&sb[s + 8], _mm256_mul_ps(s8_1, sf8));
		_mm256_store_ps(&sb[s + 16], _mm256_mul_ps(s8_2, sf8));
		_mm256_store_ps(&sb[s + 24], _mm256_mul_ps(s8_3, sf8));
	}

	for (; s < int32_t(count); s += 4)
	{
		const Vector4 s4 = Vector4::loadAligned(&sb[s]);
		(s4 * sf).storeAligned(&sb[s]);
	}
}

void AudioMixerAvx::mulConst(float* lsb, const float* rsb, uint32_t count, float factor) const
{
	T_ASSERT(alignUp(lsb, 16) == lsb);
	T_ASSERT(alignUp(count, 4) == count);

	const Scalar sf(factor);
	int32_t s = 0;

	if (alignUp(rsb, 16) == rsb)
	{
		const __m256 sf8 = _mm256_set1_ps(factor);
		const __m128 sf4 = _mm_load1_ps(&factor);

		int32_t s = 0;

		for (; s < int32_t(count) - 8 * 4; s += 8 * 4)
		{
			const __m256 s8_0 = _mm256_load_ps(&rsb[s]);
			const __m256 s8_1 = _mm256_load_ps(&rsb[s + 8]);
			const __m256 s8_2 = _mm256_load_ps(&rsb[s + 16]);
			const __m256 s8_3 = _mm256_load_ps(&rsb[s + 24]);
			_mm256_store_ps(&lsb[s], _mm256_mul_ps(s8_0, sf8));
			_mm256_store_ps(&lsb[s + 8], _mm256_mul_ps(s8_1, sf8));
			_mm256_store_ps(&lsb[s + 16], _mm256_mul_ps(s8_2, sf8));
			_mm256_store_ps(&lsb[s + 24], _mm256_mul_ps(s8_3, sf8));
		}

		for (; s < int32_t(count); s += 4)
		{
			const Vector4 rs4 = Vector4::loadAligned(&rsb[s]);
			(rs4 * sf).storeAligned(&lsb[s]);
		}
	}
	else
	{
		for (; s <= int32_t(count) - 7 * 4; s += 7 * 4)
		{
			const Vector4 rs4_0 = Vector4::loadUnaligned(&rsb[s]);
			const Vector4 rs4_1 = Vector4::loadUnaligned(&rsb[s + 4]);
			const Vector4 rs4_2 = Vector4::loadUnaligned(&rsb[s + 8]);
			const Vector4 rs4_3 = Vector4::loadUnaligned(&rsb[s + 12]);
			const Vector4 rs4_4 = Vector4::loadUnaligned(&rsb[s + 16]);
			const Vector4 rs4_5 = Vector4::loadUnaligned(&rsb[s + 20]);
			const Vector4 rs4_6 = Vector4::loadUnaligned(&rsb[s + 24]);

			(rs4_0 * sf).storeAligned(&lsb[s]);
			(rs4_1 * sf).storeAligned(&lsb[s + 4]);
			(rs4_2 * sf).storeAligned(&lsb[s + 8]);
			(rs4_3 * sf).storeAligned(&lsb[s + 12]);
			(rs4_4 * sf).storeAligned(&lsb[s + 16]);
			(rs4_5 * sf).storeAligned(&lsb[s + 20]);
			(rs4_6 * sf).storeAligned(&lsb[s + 24]);
		}

		for (; s < int32_t(count); s += 4)
		{
			const Vector4 rs4 = Vector4::loadUnaligned(&rsb[s]);
			(rs4 * sf).storeAligned(&lsb[s]);
		}
	}
}

void AudioMixerAvx::addMulConst(float* lsb, const float* rsb, uint32_t count, float factor) const
{
	T_ASSERT(alignUp(lsb, 16) == lsb);
	T_ASSERT(alignUp(rsb, 16) == rsb);
	T_ASSERT(alignUp(count, 4) == count);

	const Scalar sf(factor);
	int32_t s = 0;

	for (; s <= int32_t(count) - 3 * 4; s += 3 * 4)
	{
		const Vector4 rs4_0 = Vector4::loadAligned(&rsb[s]);
		const Vector4 rs4_1 = Vector4::loadAligned(&rsb[s + 4]);
		const Vector4 rs4_2 = Vector4::loadAligned(&rsb[s + 8]);

		const Vector4 ls4_0 = Vector4::loadAligned(&lsb[s]);
		const Vector4 ls4_1 = Vector4::loadAligned(&lsb[s + 4]);
		const Vector4 ls4_2 = Vector4::loadAligned(&lsb[s + 8]);

		(ls4_0 + rs4_0 * sf).storeAligned(&lsb[s]);
		(ls4_1 + rs4_1 * sf).storeAligned(&lsb[s + 4]);
		(ls4_2 + rs4_2 * sf).storeAligned(&lsb[s + 8]);
	}

	for (; s < int32_t(count); s += 4)
	{
		const Vector4 rs4 = Vector4::loadAligned(&rsb[s]);
		const Vector4 ls4 = Vector4::loadAligned(&lsb[s]);
		(ls4 + rs4 * sf).storeAligned(&lsb[s]);
	}
}

void AudioMixerAvx::stretch(float* lsb, uint32_t lcount, const float* rsb, uint32_t rcount, float factor) const
{
	T_ASSERT(alignUp(lsb, 16) == lsb);
	T_ASSERT(alignUp(rsb, 16) == rsb);
	T_ASSERT(alignUp(lcount, 4) == lcount);
	T_ASSERT(alignUp(rcount, 4) == rcount);

	if (lcount != rcount)
	{
		T_ASSERT(rcount < 65536);
		const uint32_t f = (rcount << 16) / lcount;

		for (uint32_t s = 0; s < lcount; s += 4)
		{
			const uint32_t s0 = s * f;
			const uint32_t s1 = s0 + f;
			const uint32_t s2 = s1 + f;
			const uint32_t s3 = s2 + f;

			lsb[s + 0] = rsb[s0 >> 16] * factor;
			lsb[s + 1] = rsb[s1 >> 16] * factor;
			lsb[s + 2] = rsb[s2 >> 16] * factor;
			lsb[s + 3] = rsb[s3 >> 16] * factor;
		}
	}
	else
	{
		mulConst(lsb, rsb, lcount, factor);
	}
}

void AudioMixerAvx::mute(float* sb, uint32_t count) const
{
	T_ASSERT(alignUp(sb, 16) == sb);
	T_ASSERT(alignUp(count, 4) == count);

	const static Vector4 zero = Vector4::zero();

	int32_t s = 0;

	for (; s <= int32_t(count) - 3 * 4; s += 3 * 4)
	{
		zero.storeAligned(&sb[s]);
		zero.storeAligned(&sb[s + 4]);
		zero.storeAligned(&sb[s + 8]);
	}

	for (; s < int32_t(count); s += 4)
		zero.storeAligned(&sb[s]);
}

void AudioMixerAvx::synchronize() const
{
}

}
