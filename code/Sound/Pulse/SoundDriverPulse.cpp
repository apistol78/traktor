/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <pulse/error.h>
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Sound/Pulse/SoundDriverPulse.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.SoundDriverPulse", 0, SoundDriverPulse, ISoundDriver)

SoundDriverPulse::SoundDriverPulse()
:	m_pa(0)
,	m_pendingSize(0)
{
}

bool SoundDriverPulse::create(const SystemApplication& sysapp, const SoundDriverCreateDesc& desc, Ref< ISoundMixer >& outMixer)
{
	int error;

	m_ss.format = PA_SAMPLE_FLOAT32LE;
	m_ss.rate = desc.sampleRate;
	m_ss.channels = desc.hwChannels;

	m_pa = pa_simple_new(NULL, "Traktor", PA_STREAM_PLAYBACK, NULL, "playback", &m_ss, NULL, NULL, &error);
	if (!m_pa)
	{
		log::error << L"Unable to create PulseAudio stream; " << mbstows(pa_strerror(error)) << Endl;
		return false;
	}

	m_pending.reset(new float [desc.frameSamples * desc.hwChannels]);
	m_pendingSize = 0;

	return true;
}

void SoundDriverPulse::destroy()
{
	int error;
	if (m_pa)
	{
		pa_simple_drain(m_pa, &error);
		pa_simple_free(m_pa);
		m_pa = 0;
	}
	m_pending.release();
	m_pendingSize = 0;
}

void SoundDriverPulse::wait()
{
	int error;
	if (m_pendingSize > 0)
	{
		pa_simple_write(m_pa, m_pending.ptr(), m_pendingSize, &error);
		m_pendingSize = 0;
	}
}

void SoundDriverPulse::submit(const SoundBlock& soundBlock)
{
	float* ptr = m_pending.ptr();
	for (uint32_t j = 0; j < soundBlock.samplesCount; ++j)
	{
		for (uint32_t i = 0; i < m_ss.channels; ++i)
		{
			if (soundBlock.samples[i])
				*ptr++ = soundBlock.samples[i][j];
			else
				*ptr++ = 0.0f;
		}
	}
	m_pendingSize = soundBlock.samplesCount * m_ss.channels * sizeof(float);
}

	}
}
