/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cmath>
#include <cstring>
#include "Core/Math/Const.h"
#include "Core/Math/Vector4.h"
#include "Core/Memory/IAllocator.h"
#include "Core/Memory/MemoryConfig.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Sound/Filters/LowPassFilter.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

struct LowPassFilterInstance : public RefCountImpl< IAudioFilterInstance >
{
	Scalar m_history[SbcMaxChannelCount];

	void* operator new (size_t size) {
		return getAllocator()->alloc(size, 16, T_FILE_LINE);
	}

	void operator delete (void* ptr) {
		getAllocator()->free(ptr);
	}
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.LowPassFilter", 0, LowPassFilter, IAudioFilter)

LowPassFilter::LowPassFilter(float cutOff)
:	m_cutOff(cutOff)
{
}

Ref< IAudioFilterInstance > LowPassFilter::createInstance() const
{
	Ref< LowPassFilterInstance > lpfi = new LowPassFilterInstance();
	std::memset(lpfi->m_history, 0, sizeof(lpfi->m_history));
	return lpfi;
}

void LowPassFilter::apply(IAudioFilterInstance* instance, AudioBlock& outBlock) const
{
	LowPassFilterInstance* lpfi = static_cast< LowPassFilterInstance* >(instance);
	if (m_cutOff > FUZZY_EPSILON && outBlock.sampleRate > 0)
	{
		// One pole coefficient of an RC low pass. The cut off is a frequency in hertz,
		// so it is the angular frequency which belong in the time constant; leaving out
		// the turn placed the actual -3 dB point a factor two pi below the one asked for,
		// i.e. every filtered source came out far darker than authored.
		//
		// The exponential form is used rather than dt / (dt + RC) since the latter only
		// hold while the cut off sit well below the sample rate: it saturates around a
		// third as the cut off approach Nyquist instead of reaching one, so a filter meant
		// to pass everything through still muffled down to a couple of kHz. This one
		// approaches unity smoothly, which lets the filter fade out of the way rather than
		// having to be switched off by a threshold, and stays sane above Nyquist too.
		const float a = 1.0f - std::exp(-TWO_PI * m_cutOff / outBlock.sampleRate);

		// Transparent; nothing to do.
		if (a >= 1.0f - FUZZY_EPSILON)
			return;

		const Scalar alpha(a);

		for (uint32_t j = 0; j < outBlock.maxChannel; ++j)
		{
			float* samples = outBlock.samples[j];
			Scalar history = lpfi->m_history[j];

			for (uint32_t i = 0; i < outBlock.samplesCount; i += 4)
			{
				const Vector4 s0123 = Vector4::loadAligned(&samples[i]);

				const Scalar s0 = (s0123.x() - history) * alpha + history;
				const Scalar s1 = (s0123.y() - s0) * alpha + s0;
				const Scalar s2 = (s0123.z() - s1) * alpha + s1;
				const Scalar s3 = (s0123.w() - s2) * alpha + s2;

				Vector4(s0, s1, s2, s3).storeAligned(&samples[i]);
				history = s3;
			}

			lpfi->m_history[j] = history;
		}
	}
}

void LowPassFilter::serialize(ISerializer& s)
{
	s >> Member< float >(L"cutOff", m_cutOff);
}

	}
}
