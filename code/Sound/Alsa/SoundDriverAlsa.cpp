#include <cstring>
#include <limits>
#include <alsa/asoundlib.h>
#include "Core/Log/Log.h"
#include "Core/Math/MathUtils.h"
#include "Core/Misc/TString.h"
#include "Sound/Alsa/SoundDriverAlsa.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

const char* c_device = "default"; // "plughw:0,0";

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.SoundDriverAlsa", 0, SoundDriverAlsa, ISoundDriver)

SoundDriverAlsa::SoundDriverAlsa()
:	m_handle(0)
,	m_hw_params(0)
,	m_buffer(0)
,	m_started(false)
{
}

SoundDriverAlsa::~SoundDriverAlsa()
{
	T_ASSERT (!m_handle);
}

bool SoundDriverAlsa::create(const SystemApplication& sysapp, const SoundDriverCreateDesc& desc, Ref< ISoundMixer >& outMixer)
{
	int res;

	res = snd_pcm_open(&m_handle, c_device, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
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

	snd_pcm_uframes_t frames = snd_pcm_uframes_t(desc.frameSamples);
	res = snd_pcm_hw_params_set_period_size_near(m_handle, m_hw_params, &frames, NULL);
	if (res < 0)
	{
		log::error << L"Unable to set period size - " << mbstows(snd_strerror(res)) << Endl;
		return false;
	}

	uint32_t periods = 2;
	res = snd_pcm_hw_params_set_periods_near(m_handle, m_hw_params, &periods, NULL);
	if (res < 0)
	{
		log::error << L"Unable to set periods - " << mbstows(snd_strerror(res)) << Endl;
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

	res = snd_pcm_sw_params_set_start_threshold(m_handle, m_sw_params, 1);
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
	m_buffer = new int16_t [desc.frameSamples * desc.hwChannels];

	log::info << L"ALSA sound driver initialized successfully" << Endl;
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
	snd_pcm_wait(m_handle, -1);
}

void SoundDriverAlsa::submit(const SoundBlock& soundBlock)
{
	// Step 1) Swizzle into intermediate output buffer.

	// How many samples can we consume, discard samples if not enough space.
	int32_t samplesCount = soundBlock.samplesCount;
	samplesCount = std::min< int32_t >(samplesCount, m_desc.frameSamples);

	// Convert samples into output buffer.
	int16_t* bufferAt = m_buffer;
	for (uint32_t j = 0; j < samplesCount; ++j)
	{
		for (uint32_t i = 0; i < m_desc.hwChannels; ++i)
		{
			if (soundBlock.samples[i])
				*bufferAt++ = int16_t(clamp(soundBlock.samples[i][j], -1.0f, 1.0f) * std::numeric_limits< int16_t >::max());
			else
				*bufferAt++ = 0;
		}
	}

	// Step 2) Write as many samples into Alsa as we can.

	// Feed samples into alsa.
	int32_t status = 0;
	do
	{
		status = snd_pcm_writei(m_handle, m_buffer, samplesCount);
		if (-status == EAGAIN)
			continue;
	}
	while (false);

	if (status < 0)
	{
		status = snd_pcm_recover(m_handle, status, 0);
		if (status < 0)
			log::error << L"Write PCM failed; unable to recover" << Endl;
	}
	else if (status < samplesCount)
		log::warning << L"Not all samples written; " << status << L" of " << samplesCount << Endl;
}

	}
}
