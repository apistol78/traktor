#include "Core/Math/Vector4.h"
#include "Core/Misc/Align.h"
#include "Sound/SoundMixer.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

uint32_t alignmentOffset(const float* s)
{
	return uint32_t(alignUp(s, 16) - s);
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.SoundMixer", SoundMixer, ISoundMixer)

void SoundMixer::mulConst(float* sb, uint32_t count, float factor) const
{
	Scalar sf(factor);

	uint32_t s = 0;

	for (; s < alignmentOffset(sb); ++s)
		sb[s] *= factor;

	for (; s < (count & ~3UL); s += 4)
	{
		Vector4 s4(&sb[s]);
		s4 *= sf;
		s4.storeAligned(&sb[s]);
	}

	for (; s < count; ++s)
		sb[s] *= factor;
}

void SoundMixer::mulConst(float* lsb, const float* rsb, uint32_t count, float factor) const
{
	Scalar sf(factor);

	uint32_t s = 0;

	for (; s < alignmentOffset(rsb); ++s)
		lsb[s] = rsb[s] * factor;

	for (; s < (count & ~3UL); s += 4)
	{
		Vector4 rs4(&rsb[s]);
		rs4 *= sf;
		rs4.storeAligned(&lsb[s]);
	}

	for (; s < count; ++s)
		lsb[s] = rsb[s] * factor;
}

void SoundMixer::addMulConst(float* lsb, const float* rsb, uint32_t count, float factor) const
{
	Scalar sf(factor);

	uint32_t s = 0;

	for (; s < alignmentOffset(rsb); ++s)
		lsb[s] += rsb[s] * factor;

	for (; s < (count & ~3UL); s += 4)
	{
		const Vector4 rs4(&rsb[s]);
		Vector4 ls4(&lsb[s]);
		ls4 += rs4 * sf;
		ls4.storeAligned(&lsb[s]);
	}

	for (; s < count; ++s)
		lsb[s] += rsb[s] * factor;
}

void SoundMixer::mute(float* sb, uint32_t count) const
{
	uint32_t s = 0;

	for (; s < alignmentOffset(sb); ++s)
		sb[s] = 0.0f;

	for (; s < (count & ~3UL); s += 4)
		Vector4::zero().storeAligned(&sb[s]);

	for (; s < count; ++s)
		sb[s] = 0.0f;
}

	}
}
