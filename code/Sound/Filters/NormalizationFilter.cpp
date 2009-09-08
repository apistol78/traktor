#include <cmath>
#include "Sound/Filters/NormalizationFilter.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.NormalizationFilter", NormalizationFilter, IFilter)

NormalizationFilter::NormalizationFilter(float targetEnergy, float energyThreshold, float attackRate)
:	m_targetEnergy(targetEnergy)
,	m_energyThreshold(energyThreshold)
,	m_attackRate(attackRate)
,	m_currentGain(1.0f)
{
}

void NormalizationFilter::apply(SoundBlock& outBlock)
{
	// Measure energy in sound block.
	float energy = 0.0f;
	for (uint32_t i = 0; i < outBlock.samplesCount; ++i)
	{
		for (uint32_t j = 0; j < outBlock.maxChannel; ++j)
			energy += std::abs(outBlock.samples[j][i]);
	}
	energy /= outBlock.samplesCount * outBlock.maxChannel;

	if (energy >= m_energyThreshold)
	{
		// Attack rate are expressed in delta per second.
		float attackRate = m_attackRate / outBlock.sampleRate;

		// Normalize energy in sound block, interpolate gain to prevent too quick changes.
		float middleGain = m_targetEnergy / energy;
		for (uint32_t i = 0; i < outBlock.samplesCount; ++i)
		{
			for (uint32_t j = 0; j < outBlock.maxChannel; ++j)
				outBlock.samples[j][i] *= m_currentGain;
			m_currentGain = middleGain * attackRate + m_currentGain * (1.0f - attackRate);
		}
	}
	else
	{
		// Energy below threshold, keep current gain.
		for (uint32_t i = 0; i < outBlock.samplesCount; ++i)
		{
			for (uint32_t j = 0; j < outBlock.maxChannel; ++j)
				outBlock.samples[j][i] *= m_currentGain;
		}
	}
}

	}
}
