/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Sound/Filters/DitherFilter.h"

namespace traktor::sound
{
	namespace
	{

struct DitherFilterInstance : public RefCountImpl< IAudioFilterInstance >
{
};

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.DitherFilter", 0, DitherFilter, IAudioFilter)

DitherFilter::DitherFilter(uint32_t bitsPerSample)
:	m_ditherAmplitude(1.0f / (1 << bitsPerSample))
{
}

Ref< IAudioFilterInstance > DitherFilter::createInstance() const
{
	return new DitherFilterInstance();
}

void DitherFilter::apply(IAudioFilterInstance* instance, AudioBlock& outBlock) const
{
	for (uint32_t i = 0; i < outBlock.samplesCount; ++i)
	{
		const float r = (float)((m_random.nextDouble() * 2.0 - 1.0) * m_ditherAmplitude);
		for (uint32_t j = 0; j < outBlock.maxChannel; ++j)
			outBlock.samples[j][i] += r;
	}
}

void DitherFilter::serialize(ISerializer& s)
{
}

}
