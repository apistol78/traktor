/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Float.h"
#include "Core/Memory/IAllocator.h"
#include "Core/Memory/MemoryConfig.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "Sound/Filters/ReverbFilter.h"

namespace traktor::sound
{
	namespace
	{

class Delay
{
public:
	explicit Delay(uint32_t samples)
	:	m_count(0)
	{
		m_buffer.resize(samples, 0.0f);
	}

	void put(float s)
	{
		m_buffer[m_count] = s;
		m_count = (m_count + 1) % m_buffer.size();
	}

	float get() const
	{
		const int32_t index = (m_count + 1) % m_buffer.size();
		return m_buffer[index];
	}

private:
	AlignedVector< float > m_buffer;
	int32_t m_count;
};

struct ReverbFilterInstance : public RefCountImpl< IAudioFilterInstance >
{
	struct Channel
	{
		AutoPtr< Delay > delay[4];
	};

	Channel m_channels[SbcMaxChannelCount];

	ReverbFilterInstance(const int32_t* delay)
	{
		for (auto& channel : m_channels)
		{
			for (int32_t i = 0; i < 4; ++i)
			{
				const int32_t delaySamples = (int32_t)(delay[i] * 44.1f);
				if (delaySamples > 0)
					channel.delay[i].reset(new Delay(delaySamples));
				else
					break;
			}
		}
	}

	void* operator new (size_t size) {
		return getAllocator()->alloc(size, 16, T_FILE_LINE);
	}

	void operator delete (void* ptr) {
		getAllocator()->free(ptr);
	}
};

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.ReverbFilter", 0, ReverbFilter, IAudioFilter)

ReverbFilter::ReverbFilter()
:	m_decay(0.1f)
,	m_feedback(0.1f)
{
	m_delay[0] = 17;
	m_delay[1] = 37;
	m_delay[2] = 61;
	m_delay[3] = 97;
}

Ref< IAudioFilterInstance > ReverbFilter::createInstance() const
{
	return new ReverbFilterInstance(m_delay);
}

void ReverbFilter::apply(IAudioFilterInstance* instance, AudioBlock& outBlock) const
{
	ReverbFilterInstance* rfi = static_cast< ReverbFilterInstance* >(instance);
	for (uint32_t i = 0; i < outBlock.maxChannel; ++i)
	{
		ReverbFilterInstance::Channel& channel = rfi->m_channels[i];
		float* samples = outBlock.samples[i];
		for (uint32_t j = 0; j < outBlock.samplesCount; ++j)
		{
			float S = samples[j];
			for (uint32_t k = 0; k < 4; ++k)
			{
				if (channel.delay[k].c_ptr() == nullptr)
					break;

				const float Sdelay = channel.delay[k]->get();
				const float Sneg = Sdelay + S * -m_feedback;
				channel.delay[k]->put(S + Sneg * m_decay);

				S = Sneg;

			}
			samples[j] = S;
		}
	}
}

void ReverbFilter::serialize(ISerializer& s)
{
	s >> MemberStaticArray< int32_t, 4 >(L"delay", m_delay);
	s >> Member< float >(L"decay", m_decay, AttributeRange(0.0f, 1.0f) | AttributeUnit(UnitType::Percent));
	s >> Member< float >(L"feedback", m_feedback, AttributeRange(0.0f, 1.0f) | AttributeUnit(UnitType::Percent));
}

}
