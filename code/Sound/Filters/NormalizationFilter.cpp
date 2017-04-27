/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cmath>
#include "Core/Memory/IAllocator.h"
#include "Core/Memory/MemoryConfig.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Sound/Filters/NormalizationFilter.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

struct NormalizationFilterInstance : public RefCountImpl< IFilterInstance >
{
	float m_currentGain;

	void* operator new (size_t size) {
		return getAllocator()->alloc(size, 16, T_FILE_LINE);
	}

	void operator delete (void* ptr) {
		getAllocator()->free(ptr);
	}
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.NormalizationFilter", 0, NormalizationFilter, IFilter)

NormalizationFilter::NormalizationFilter(float targetEnergy, float energyThreshold, float attackRate)
:	m_targetEnergy(targetEnergy)
,	m_energyThreshold(energyThreshold)
,	m_attackRate(attackRate)
{
}

Ref< IFilterInstance > NormalizationFilter::createInstance() const
{
	Ref< NormalizationFilterInstance > nfi = new NormalizationFilterInstance();
	nfi->m_currentGain = 1.0f;
	return nfi;
}

void NormalizationFilter::apply(IFilterInstance* instance, SoundBlock& outBlock) const
{
	NormalizationFilterInstance* nfi = static_cast< NormalizationFilterInstance* >(instance);

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
				outBlock.samples[j][i] *= nfi->m_currentGain;
			nfi->m_currentGain = middleGain * attackRate + nfi->m_currentGain * (1.0f - attackRate);
		}
	}
	else
	{
		// Energy below threshold, keep current gain.
		for (uint32_t i = 0; i < outBlock.samplesCount; ++i)
		{
			for (uint32_t j = 0; j < outBlock.maxChannel; ++j)
				outBlock.samples[j][i] *= nfi->m_currentGain;
		}
	}
}

void NormalizationFilter::serialize(ISerializer& s)
{
	s >> Member< float >(L"targetEnergy", m_targetEnergy);
	s >> Member< float >(L"energyThreshold", m_energyThreshold);
	s >> Member< float >(L"attackRate", m_attackRate);
}

	}
}
