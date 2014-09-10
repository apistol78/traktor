#include "Core/Log/Log.h"
#include "Core/Math/MathUtils.h"
#include "Sound/PNaCl/SoundDriverPNaCl.h"

#undef min
#undef max

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.SoundDriverPNaCl", 0, SoundDriverPNaCl, ISoundDriver)

SoundDriverPNaCl::SoundDriverPNaCl()
:	m_pendingSoundBlock(0)
{
}

bool SoundDriverPNaCl::create(void* nativeHandle, const SoundDriverCreateDesc& desc, Ref< ISoundMixer >& outMixer)
{
	pp::Instance* instance = (pp::Instance*)nativeHandle;
	if (!instance)
	{
		log::error << L"Unable to create PNaCl sound driver; Invalid instance handle." << Endl;
		return false;
	}

	if (desc.hwChannels != 2)
	{
		log::error << L"Unable to create PNaCl sound driver; Only 2 hardware channels supported." << Endl;
		return false;
	}
	if (desc.sampleRate != 44100 && desc.sampleRate != 48000)
	{
		log::error << L"Unable to create PNaCl sound driver; Invalid sample rate, must be either 44100 or 48000." << Endl;
		return false;
	}

	m_desc = desc;

	int32_t recommended = pp::AudioConfig::RecommendSampleFrameCount(
		instance, (PP_AudioSampleRate)m_desc.sampleRate, m_desc.frameSamples
	);
	if (recommended <= 0)
	{
		log::error << L"Unable to create PNaCl sound driver; Unable to determine sample frame count." << Endl;
		log::error << L"\tSample rate: " << m_desc.sampleRate << Endl;
		log::error << L"\tFrame samples: " << m_desc.frameSamples << Endl;
		return false;
	}

	m_audio = pp::Audio(
		instance,
		pp::AudioConfig(instance, (PP_AudioSampleRate)m_desc.sampleRate, m_desc.frameSamples),
		&streamCallback,
		this
	);
	if (m_audio.is_null())
	{
		log::error << L"Unable to create PNaCl sound driver; Failed to create audio resource." << Endl;
		return false;
	}

	if (!m_audio.StartPlayback())
	{
		log::error << L"Unable to create PNaCl sound driver; Failed to start playback." << Endl;
		return false;
	}

	return true;
}

void SoundDriverPNaCl::destroy()
{
	m_audio.StopPlayback();
}

void SoundDriverPNaCl::wait()
{
}

void SoundDriverPNaCl::submit(const SoundBlock& soundBlock)
{
	m_pendingSoundBlock = &soundBlock;
	m_eventPending.broadcast();
	m_eventReady.wait(1000);
}

void SoundDriverPNaCl::streamCallback(void* samples, uint32_t bufferSize, void* data)
{
	SoundDriverPNaCl* this_ = reinterpret_cast< SoundDriverPNaCl* >(data);

	// Wait for pending sound block.
	if (this_->m_eventPending.wait(100))
	{
		// Convert pending sound block into output data.
		const SoundBlock* block = this_->m_pendingSoundBlock;
		T_ASSERT (block != 0);

		int16_t* write = reinterpret_cast< int16_t* >(samples);
		for (int32_t i = 0; i < block->samplesCount; ++i)
		{
			for (int32_t channel = 0; channel < 2; ++channel)
			{
				if (block->samples[channel])
					*write++ = int16_t(clamp(block->samples[channel][i], -1.0f, 1.0f) * 32767.0f);
				else
					*write++ = 0;
			}
		}
	}
	else
	{
		// No block queued in reasonable time; feed muted block.
		int16_t* write = reinterpret_cast< int16_t* >(samples);
		for (int32_t i = 0; i < this_->m_desc.frameSamples; ++i)
		{
			for (int32_t channel = 0; channel < 2; ++channel)
			{
				*write++ = 0;
			}
		}
	}

	// Finished converting pending; ready for another submission.
	this_->m_eventReady.broadcast();
}


	}
}
