#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Flash/FlashSound.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.FlashSound", 0, FlashSound, ISerializable)

FlashSound::FlashSound()
:	m_channels(0)
,	m_sampleRate(0)
,	m_samplesCount(0)
{
}

bool FlashSound::create(uint8_t channels, uint32_t sampleRate, uint32_t samplesCount)
{
	T_ASSERT (channels <= sizeof_array(m_samples));

	for (uint8_t i = 0; i < channels; ++i)
		m_samples[i].reset(new int16_t [samplesCount]);

	m_channels = channels;
	m_sampleRate = sampleRate;
	m_samplesCount = samplesCount;
	return true;
}

void FlashSound::serialize(ISerializer& s)
{
	s >> Member< uint8_t >(L"channels", m_channels);
	s >> Member< uint32_t >(L"sampleRate", m_sampleRate);
	s >> Member< uint32_t >(L"samplesCount", m_samplesCount);

	for (uint8_t i = 0; i < m_channels; ++i)
	{
		if (s.getDirection() == ISerializer::SdRead)
			m_samples[i].reset(new int16_t [m_samplesCount]);

		void* data = m_samples[i].ptr();
		uint32_t size = m_samplesCount * sizeof(int16_t);

		s >> Member< void* >(L"samples", data, size);
	}
}

	}
}
