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
	for (uint32_t s = 0; s < count; s += 4)
	{
		Vector4 s4 = Vector4::loadAligned(&sb[s]);
		s4 *= sf;
		s4.storeAligned(&sb[s]);
	}
}

void SoundMixer::mulConst(float* lsb, const float* rsb, uint32_t count, float factor) const
{
	T_ASSERT (alignUp(lsb, 16) == lsb);
	T_ASSERT (alignUp(rsb, 16) == rsb);
	T_ASSERT (alignUp(count, 4) == count);

	Scalar sf(factor);
	for (uint32_t s = 0; s < count; s += 4)
	{
		Vector4 rs4 = Vector4::loadAligned(&rsb[s]);
		rs4 *= sf;
		rs4.storeAligned(&lsb[s]);
	}
}

void SoundMixer::addMulConst(float* lsb, const float* rsb, uint32_t count, float factor) const
{
	T_ASSERT (alignUp(lsb, 16) == lsb);
	T_ASSERT (alignUp(rsb, 16) == rsb);
	T_ASSERT (alignUp(count, 4) == count);

	Scalar sf(factor);
	for (uint32_t s = 0; s < count; s += 4)
	{
		Vector4 rs4 = Vector4::loadAligned(&rsb[s]);
		Vector4 ls4 = Vector4::loadAligned(&lsb[s]);
		ls4 += rs4 * sf;
		ls4.storeAligned(&lsb[s]);
	}
}

void SoundMixer::stretch(float* lsb, uint32_t lcount, const float* rsb, uint32_t rcount, float factor) const
{
	T_ASSERT (alignUp(lsb, 16) == lsb);
	T_ASSERT (alignUp(rsb, 16) == rsb);
	T_ASSERT (alignUp(lcount, 4) == lcount);
	T_ASSERT (alignUp(rcount, 4) == rcount);

	for (uint32_t s = 0; s < lcount; ++s)
	{
		uint32_t s0 = (s * rcount) / lcount;
		lsb[s] = rsb[s0] * factor;
	}
}

void SoundMixer::mute(float* sb, uint32_t count) const
{
	T_ASSERT (alignUp(sb, 16) == sb);
	T_ASSERT (alignUp(count, 4) == count);

	for (uint32_t s = 0; s < count; s += 4)
		Vector4::zero().storeAligned(&sb[s]);
}

void SoundMixer::synchronize() const
{
}

	}
}
