#include "Sound/Filters/DitherFilter.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.DitherFilter", DitherFilter, IFilter)

DitherFilter::DitherFilter(uint32_t bitsPerSample)
:	m_ditherAmplitude(1.0f / (1 << bitsPerSample))
{
}

void DitherFilter::apply(SoundBlock& outBlock)
{
	for (uint32_t i = 0; i < outBlock.samplesCount; ++i)
	{
		float r = float((m_random.nextDouble() * 2.0 - 1.0) * m_ditherAmplitude);
		for (uint32_t j = 0; j < outBlock.maxChannel; ++j)
			outBlock.samples[j][i] += r;
	}
}

	}
}
