#include "Core/Math/Vector4.h"
#include "Sound/SoundBlockUtilities.h"

namespace traktor
{
	namespace sound
	{

void soundBlockMulConst(SoundBlock& sb, float factor)
{
	Scalar sf(factor);
	for (uint32_t ch = 0; ch < sb.maxChannel; ++ch)
	{
		if (!sb.samples[ch])
			continue;

		uint32_t s = 0;
		for (; s < (sb.samplesCount & ~3UL); s += 4)
		{
			Vector4 s4(&sb.samples[ch][s]);
			s4 *= sf;
			s4.store(&sb.samples[ch][s]);
		}
		for (; s < sb.samplesCount; ++s)
			sb.samples[ch][s] *= factor;
	}
}

void soundBlockMulConst(float* sb, uint32_t count, float factor)
{
	Scalar sf(factor);
	uint32_t s = 0;
	for (; s < (count & ~3UL); s += 4)
	{
		Vector4 s4(&sb[s]);
		s4 *= sf;
		s4.store(&sb[s]);
	}
	for (; s < count; ++s)
		sb[s] *= factor;
}

void soundBlockMulConst(float* lsb, const float* rsb, uint32_t count, float factor)
{
	Scalar sf(factor);
	uint32_t s = 0;
	for (; s < (count & ~3UL); s += 4)
	{
		Vector4 rs4(&rsb[s]);
		rs4 *= sf;
		rs4.store(&lsb[s]);
	}
	for (; s < count; ++s)
		lsb[s] = rsb[s] * factor;
}

void soundBlockAddMulConst(float* lsb, const float* rsb, uint32_t count, float factor)
{
	Scalar sf(factor);
	uint32_t s = 0;
	for (; s < (count & ~3UL); s += 4)
	{
		const Vector4 rs4(&rsb[s]);
		Vector4 ls4(&lsb[s]);
		ls4 += rs4 * sf;
		ls4.store(&lsb[s]);
	}
	for (; s < count; ++s)
		lsb[s] += rsb[s] * factor;
}

void soundBlockMute(float* sb, uint32_t count)
{
	uint32_t s = 0;
	for (; s < (count & ~3UL); s += 4)
		Vector4::zero().store(&sb[s]);
	for (; s < count; ++s)
		sb[s] = 0.0f;
}

	}
}
