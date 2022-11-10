/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
	return nullptr;
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
