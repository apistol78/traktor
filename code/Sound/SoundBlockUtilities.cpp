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
		for (; s + 4 < sb.samplesCount; s += 4)
		{
			Vector4 s4(&sb.samples[ch][s]);
			s4 *= sf;
			s4.store(&sb.samples[ch][s]);
		}
		for (; s < sb.samplesCount; ++s)
			sb.samples[ch][s] *= factor;
	}
}

void soundBlockAddMulConst(SoundBlock& lsb, int lch, const SoundBlock& rsb, int32_t rch, float factor)
{
	Scalar sf(factor);
	uint32_t s = 0;
	for (; s + 4 < rsb.samplesCount; s += 4)
	{
		const Vector4 rs4(&rsb.samples[rch][s]);
		Vector4 ls4(&lsb.samples[lch][s]);
		ls4 += rs4 * sf;
		ls4.store(&lsb.samples[lch][s]);
	}
	for (; s < rsb.samplesCount; ++s)
		lsb.samples[lch][s] += rsb.samples[rch][s] * factor;
}

	}
}
