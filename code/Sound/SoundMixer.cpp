/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Math/Vector4.h"
#include "Core/Misc/Align.h"
#include "Sound/SoundMixer.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.SoundMixer", SoundMixer, ISoundMixer)

void SoundMixer::mulConst(float* sb, uint32_t count, float factor) const
{
	T_ASSERT (alignUp(sb, 16) == sb);
	T_ASSERT (alignUp(count, 4) == count);

	Scalar sf(factor);
	int32_t s = 0;

	for (; s < int32_t(count) - 7 * 4; s += 7 * 4)
	{
		Vector4 s4_0 = Vector4::loadAligned(&sb[s]);
		Vector4 s4_1 = Vector4::loadAligned(&sb[s + 4]);
		Vector4 s4_2 = Vector4::loadAligned(&sb[s + 8]);
		Vector4 s4_3 = Vector4::loadAligned(&sb[s + 12]);
		Vector4 s4_4 = Vector4::loadAligned(&sb[s + 16]);
		Vector4 s4_5 = Vector4::loadAligned(&sb[s + 20]);
		Vector4 s4_6 = Vector4::loadAligned(&sb[s + 24]);

		(s4_0 * sf).storeAligned(&sb[s]);
		(s4_1 * sf).storeAligned(&sb[s + 4]);
		(s4_2 * sf).storeAligned(&sb[s + 8]);
		(s4_3 * sf).storeAligned(&sb[s + 12]);
		(s4_4 * sf).storeAligned(&sb[s + 16]);
		(s4_5 * sf).storeAligned(&sb[s + 20]);
		(s4_6 * sf).storeAligned(&sb[s + 24]);
	}

	for (; s < int32_t(count); s += 4)
	{
		Vector4 s4 = Vector4::loadAligned(&sb[s]);
		(s4 * sf).storeAligned(&sb[s]);
	}
}

void SoundMixer::mulConst(float* lsb, const float* rsb, uint32_t count, float factor) const
{
	T_ASSERT (alignUp(lsb, 16) == lsb);
	T_ASSERT (alignUp(count, 4) == count);

	Scalar sf(factor);
	int32_t s = 0;

	if (alignUp(rsb, 16) == rsb)
	{
		for (; s <= int32_t(count) - 7 * 4; s += 7 * 4)
		{
			Vector4 rs4_0 = Vector4::loadAligned(&rsb[s]);
			Vector4 rs4_1 = Vector4::loadAligned(&rsb[s + 4]);
			Vector4 rs4_2 = Vector4::loadAligned(&rsb[s + 8]);
			Vector4 rs4_3 = Vector4::loadAligned(&rsb[s + 12]);
			Vector4 rs4_4 = Vector4::loadAligned(&rsb[s + 16]);
			Vector4 rs4_5 = Vector4::loadAligned(&rsb[s + 20]);
			Vector4 rs4_6 = Vector4::loadAligned(&rsb[s + 24]);

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
			Vector4 rs4 = Vector4::loadAligned(&rsb[s]);
			(rs4 * sf).storeAligned(&lsb[s]);
		}
	}
	else
	{
		for (; s <= int32_t(count) - 7 * 4; s += 7 * 4)
		{
			Vector4 rs4_0 = Vector4::loadUnaligned(&rsb[s]);
			Vector4 rs4_1 = Vector4::loadUnaligned(&rsb[s + 4]);
			Vector4 rs4_2 = Vector4::loadUnaligned(&rsb[s + 8]);
			Vector4 rs4_3 = Vector4::loadUnaligned(&rsb[s + 12]);
			Vector4 rs4_4 = Vector4::loadUnaligned(&rsb[s + 16]);
			Vector4 rs4_5 = Vector4::loadUnaligned(&rsb[s + 20]);
			Vector4 rs4_6 = Vector4::loadUnaligned(&rsb[s + 24]);

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
			Vector4 rs4 = Vector4::loadUnaligned(&rsb[s]);
			(rs4 * sf).storeAligned(&lsb[s]);
		}
	}
}

void SoundMixer::addMulConst(float* lsb, const float* rsb, uint32_t count, float factor) const
{
	T_ASSERT (alignUp(lsb, 16) == lsb);
	T_ASSERT (alignUp(rsb, 16) == rsb);
	T_ASSERT (alignUp(count, 4) == count);

	Scalar sf(factor);
	int32_t s = 0;

	for (; s <= int32_t(count) - 3 * 4; s += 3 * 4)
	{
		Vector4 rs4_0 = Vector4::loadAligned(&rsb[s]);
		Vector4 rs4_1 = Vector4::loadAligned(&rsb[s + 4]);
		Vector4 rs4_2 = Vector4::loadAligned(&rsb[s + 8]);

		Vector4 ls4_0 = Vector4::loadAligned(&lsb[s]);
		Vector4 ls4_1 = Vector4::loadAligned(&lsb[s + 4]);
		Vector4 ls4_2 = Vector4::loadAligned(&lsb[s + 8]);

		(ls4_0 + rs4_0 * sf).storeAligned(&lsb[s]);
		(ls4_1 + rs4_1 * sf).storeAligned(&lsb[s + 4]);
		(ls4_2 + rs4_2 * sf).storeAligned(&lsb[s + 8]);
	}

	for (; s < int32_t(count); s += 4)
	{
		Vector4 rs4 = Vector4::loadAligned(&rsb[s]);
		Vector4 ls4 = Vector4::loadAligned(&lsb[s]);
		(ls4 + rs4 * sf).storeAligned(&lsb[s]);
	}
}

void SoundMixer::stretch(float* lsb, uint32_t lcount, const float* rsb, uint32_t rcount, float factor) const
{
	T_ASSERT (alignUp(lsb, 16) == lsb);
	T_ASSERT (alignUp(rsb, 16) == rsb);
	T_ASSERT (alignUp(lcount, 4) == lcount);
	T_ASSERT (alignUp(rcount, 4) == rcount);

	if (lcount != rcount)
	{
		T_ASSERT (rcount < 65536);
		uint32_t f = (rcount << 16) / lcount;

		for (uint32_t s = 0; s < lcount; s += 4)
		{
			uint32_t s0 = s * f;
			uint32_t s1 = s0 + f;
			uint32_t s2 = s1 + f;
			uint32_t s3 = s2 + f;

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

void SoundMixer::mute(float* sb, uint32_t count) const
{
	T_ASSERT (alignUp(sb, 16) == sb);
	T_ASSERT (alignUp(count, 4) == count);

	const Vector4 zero = Vector4::zero();
	for (uint32_t s = 0; s < count; s += 4)
		zero.storeAligned(&sb[s]);
}

void SoundMixer::synchronize() const
{
}

	}
}
