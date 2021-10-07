#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Spark/Sound.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spark.Sound", 0, Sound, ISerializable)

bool Sound::create(uint8_t channels, uint32_t sampleRate, uint32_t sampleCount)
{
	T_ASSERT(channels <= sizeof_array(m_samples));

	for (uint8_t i = 0; i < channels; ++i)
		m_samples[i].reset(new int16_t [sampleCount]);

	m_channels = channels;
	m_sampleRate = sampleRate;
	m_sampleCount = sampleCount;
	return true;
}

void Sound::serialize(ISerializer& s)
{
	s >> Member< uint8_t >(L"channels", m_channels);
	s >> Member< uint32_t >(L"sampleRate", m_sampleRate);
	s >> Member< uint32_t >(L"sampleCount", m_sampleCount);

	for (uint8_t i = 0; i < m_channels; ++i)
	{
		if (s.getDirection() == ISerializer::Direction::Read)
			m_samples[i].reset(new int16_t [m_sampleCount]);

		void* data = m_samples[i].ptr();
		uint32_t size = m_sampleCount * sizeof(int16_t);

		s >> Member< void* >(L"samples", data, size);
	}
}

	}
}
