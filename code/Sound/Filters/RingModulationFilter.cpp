#include "Sound/Filters/RingModulationFilter.h"
#include "Core/Math/MathUtils.h"
#include "Core/Math/Const.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.RingModulationFilter", RingModulationFilter, Filter)

RingModulationFilter::RingModulationFilter(uint32_t sampleRate, uint32_t ringFrequency)
:	m_sampleRate(sampleRate)
,	m_ringFrequency(ringFrequency)
,	m_time(0.0f)
{
}

void RingModulationFilter::apply(SoundBlock& outBlock)
{
	float sampleDeltaTime = 1.0f / m_sampleRate;
	for (uint32_t i = 0; i < outBlock.samplesCount; ++i)
	{
		float ringAmplitude = sinf(m_ringFrequency * m_time * 2.0f * PI);
		for (uint32_t j = 0; j < outBlock.maxChannel; ++j)
			outBlock.samples[j][i] *= ringAmplitude;
		m_time += sampleDeltaTime;
	}
}

	}
}
