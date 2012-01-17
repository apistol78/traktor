#include "Flash/FlashSound.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashSound", FlashSound, Object)

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

	}
}
