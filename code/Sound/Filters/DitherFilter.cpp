/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Sound/Filters/DitherFilter.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.DitherFilter", 0, DitherFilter, IFilter)

DitherFilter::DitherFilter(uint32_t bitsPerSample)
:	m_ditherAmplitude(1.0f / (1 << bitsPerSample))
{
}

Ref< IFilterInstance > DitherFilter::createInstance() const
{
	return 0;
}

void DitherFilter::apply(IFilterInstance* instance, SoundBlock& outBlock) const
{
	for (uint32_t i = 0; i < outBlock.samplesCount; ++i)
	{
		float r = float((m_random.nextDouble() * 2.0 - 1.0) * m_ditherAmplitude);
		for (uint32_t j = 0; j < outBlock.maxChannel; ++j)
			outBlock.samples[j][i] += r;
	}
}

void DitherFilter::serialize(ISerializer& s)
{
}

	}
}
