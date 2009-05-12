#include <limits>
#include <alsa/asoundlib.h>
#include "Sound/Alsa/SoundDriverAlsa.h"
#include "Core/Misc/TString.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{
			
const char* c_device = "plughw:0,0";
			
		}

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.sound.SoundDriverAlsa", SoundDriverAlsa, SoundDriver)
	
SoundDriverAlsa::SoundDriverAlsa()
:	m_handle(0)
,	m_hw_params(0)
,	m_buffer(0)
,	m_bufferCount(0)
{
}

SoundDriverAlsa::~SoundDriverAlsa()
{
	T_ASSERT (!m_handle);
}

bool SoundDriverAlsa::create(const SoundDriverCreateDesc& desc)
{
	int res;
	
	res = snd_pcm_open(&m_handle, c_device, SND_PCM_STREAM_PLAYBACK, 0);
	if (res < 0)
	{
		log::error << L"Unable to open audio device \"" << mbstows(c_device) << L" - " << mbstows(snd_strerror(res)) << Endl;
		return false;
	}
	
	res = snd_pcm_hw_params_malloc(&m_hw_params);
	if (res < 0)
	{
		log::error<< L"Unable to allocate hardware parameter structure - " << mbstows(snd_strerror(res)) << Endl;
		return false;
	}
	
	res = snd_pcm_hw_params_any(m_handle, m_hw_params);
	if (res < 0)
	{
		log::error << L"Unable to initialize hardware parameter structure - " << mbstows(snd_strerror(res)) << Endl;
		return false;
	}
	
	res = snd_pcm_hw_params_set_access(m_handle, m_hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if (res < 0)
	{
		log::error << L"Unable to set access type - " << mbstows(snd_strerror(res)) << Endl;
		return false;
	}
	
	res = snd_pcm_hw_params_set_format(m_handle, m_hw_params, SND_PCM_FORMAT_S16_LE);
	if (res < 0)
	{
		log::error << L"Unable to set sample format - " << mbstows(snd_strerror(res)) << Endl;
		return false;
	}
	
	uint32_t sampleRate = desc.sampleRate;
	res = snd_pcm_hw_params_set_rate_near(m_handle, m_hw_params, &sampleRate, 0);
	if (res < 0)
	{
		log::error << L"Unable to set sample rate - " << mbstows(snd_strerror(res)) << Endl;
		return false;
	}
	
	res = snd_pcm_hw_params_set_channels(m_handle, m_hw_params, desc.hwChannels);
	if (res < 0)
	{
		log::error << L"Unable to set channel count - " << mbstows(snd_strerror(res)) << Endl;
		return false;
	}
	
	res = snd_pcm_hw_params(m_handle, m_hw_params);
	if (res < 0)
	{
		log::error << L"Unable to set parameters - " << mbstows(snd_strerror(res)) << Endl;
		return false;
	}
	
	res = snd_pcm_sw_params_malloc(&m_sw_params);
	if (res < 0)
	{
		log::error << L"Unable to allocate software parameters - " << mbstows(snd_strerror(res)) << Endl;
		return false;
	}
	
	res = snd_pcm_sw_params_current(m_handle, m_sw_params);
	if (res < 0)
	{
		log::error << L"Unable to get current software parameters - " << mbstows(snd_strerror(res)) << Endl;
		return false;
	}
	
	res = snd_pcm_sw_params_set_avail_min(m_handle, m_sw_params, desc.frameSamples);
	if (res < 0)
	{
		log::error << L"Unable to set minimum available - " << mbstows(snd_strerror(res)) << Endl;
		return false;
	}
	
	res = snd_pcm_sw_params_set_start_threshold(m_handle, m_sw_params, desc.frameSamples);
	if (res < 0)
	{
		log::error << L"Unable to set start threshold - " << mbstows(snd_strerror(res)) << Endl;
		return false;
	}
	
	res = snd_pcm_sw_params(m_handle, m_sw_params);
	if (res < 0)
	{
		log::error << L"Unable to set software parameters - " << mbstows(snd_strerror(res)) << Endl;
		return false;
	}
	
	res = snd_pcm_prepare(m_handle);
	if (res < 0)
	{
		log::error << L"Unable to prepare audio interface for use - " << mbstows(snd_strerror(res)) << Endl;
		return false;
	}

	m_desc = desc;

	m_buffer = new int16_t [desc.frameSamples * 3 * desc.hwChannels];
	m_bufferCount = 0;

	std::memset(m_buffer, 0, desc.frameSamples * 3 * desc.hwChannels * sizeof(int16_t));
	
	return true;
}

void SoundDriverAlsa::destroy()
{
	if (m_handle)
	{
		snd_pcm_close(m_handle);
		m_handle = 0;
	}
	if (m_buffer)
	{
		delete[] m_buffer;
		m_buffer = 0;
	}
}

void SoundDriverAlsa::wait()
{
	snd_pcm_wait(m_handle, 5000);
}

void SoundDriverAlsa::submit(const SoundBlock& soundBlock)
{
	// Convert samples into buffer.
	if (m_desc.frameSamples * 3 - m_bufferCount >= soundBlock.samplesCount)
	{
		int16_t* bufferAt = &m_buffer[m_bufferCount * m_desc.hwChannels];

		for (uint32_t j = 0; j < soundBlock.samplesCount; ++j)		
		{
			for (uint32_t i = 0; i < soundBlock.channels; ++i)
				*bufferAt++ = int16_t(soundBlock.samples[i][j] * std::numeric_limits< int16_t >::max());
		}

		m_bufferCount += soundBlock.samplesCount;
	}
	else
		log::error << L"Out of buffer space, sound block skipped" << Endl;

	// Feed samples from buffer into Alsa.
	int32_t framesAvail = snd_pcm_avail_update(m_handle);
	if (framesAvail == -EPIPE)
		return;

	if (framesAvail > m_bufferCount)
	{
		framesAvail = m_bufferCount;
		log::warning << L"Buffer unrun, cannot keep up with playback" << Endl;
	}
	
	int32_t framesWritten = snd_pcm_writei(m_handle, m_buffer, framesAvail);
	if (framesWritten > 0)
	{
		std::memmove(m_buffer, &m_buffer[framesWritten * m_desc.hwChannels], sizeof(int16_t) * ((m_desc.frameSamples * 3 - framesWritten) * m_desc.hwChannels));
		m_bufferCount -= framesWritten;
	}

	log::info << m_bufferCount << Endl;
}

	}
}
