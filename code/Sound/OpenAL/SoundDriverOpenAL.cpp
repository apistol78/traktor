#include <limits>
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Timer/Timer.h"
#include "Sound/OpenAL/SoundDriverOpenAL.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{
		
template < typename SampleType >
void writeSamples(
	void* dest,
	const float* samples,
	uint32_t samplesCount,
	uint32_t writeStride
)
{
	SampleType* write = static_cast< SampleType* >(dest);
	for (uint32_t i = 0; i < samplesCount; ++i)
	{
		float sample = samples[i];
		sample = std::max< float >(sample, -1.0f);
		sample = std::min< float >(sample,  1.0f);
		write[i * writeStride] = static_cast< SampleType >(sample * std::numeric_limits< SampleType >::max());
	}
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.SoundDriverOpenAL", 0, SoundDriverOpenAL, ISoundDriver)

SoundDriverOpenAL::SoundDriverOpenAL()
:	m_device(0)
,	m_context(0)
,	m_format(0)
,	m_source(0)
,	m_submitted(0)
{
	for (int i = 0; i < sizeof_array(m_buffers); ++i)
		m_buffers[i] = 0;
}

bool SoundDriverOpenAL::create(const SoundDriverCreateDesc& desc, Ref< ISoundMixer >& outMixer)
{
	const ALuint c_formats[][2] =
	{
		{ AL_FORMAT_MONO8, AL_FORMAT_STEREO8 },
		{ AL_FORMAT_MONO16, AL_FORMAT_STEREO16 }
	};

	// Open device and create context.
	m_device = alcOpenDevice(0);
	if (!m_device)
		return false;
		
	m_context = alcCreateContext(m_device, NULL);
	if (!m_context)
		return false;
		
	alcMakeContextCurrent(m_context);
	
	// Determine data format.
	int r = -1, c = -1;
	
	if (desc.bitsPerSample == 8)
		r = 0;
	else if (desc.bitsPerSample == 16)
		r = 1;
		
	if (desc.hwChannels == 1)
		c = 0;
	else if (desc.hwChannels == 2)
		c = 1;

	if (r < 0 || c < 0)
		return false;
	
	m_desc = desc;
	m_format = c_formats[r][c];
	m_submitted = 0;
	m_data.reset(
		new uint8_t [desc.frameSamples * desc.hwChannels * desc.bitsPerSample / 8]
	);
	
	// Generate buffers.
	alGenBuffers(sizeof_array(m_buffers), m_buffers);
	alGenSources(1, &m_source);
	
	alSource3f(m_source, AL_POSITION, 0.0f, 0.0f, 0.0f);
	alSource3f(m_source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
	alSource3f(m_source, AL_DIRECTION, 0.0f, 0.0f, 0.0f);
	alSourcef(m_source, AL_ROLLOFF_FACTOR, 0.0f);
	alSourcei(m_source, AL_SOURCE_RELATIVE, AL_TRUE);

	alSourcePlay(m_source);
	return true;
}

void SoundDriverOpenAL::destroy()
{
	alSourceStop(m_source);
	alDeleteSources(1, &m_source);
	alDeleteBuffers(sizeof_array(m_buffers), m_buffers);
	
	alcMakeContextCurrent(0);
	alcDestroyContext(m_context);
	alcCloseDevice(m_device);
}

void SoundDriverOpenAL::wait()
{
	ALint processed;
	Timer timer;

	if (m_submitted < sizeof_array(m_buffers))
		return;

	Thread* currentThread = ThreadManager::getInstance().getCurrentThread();

	// Wait until at least one buffer has been processed by OpenAL.
	double timeout = timer.getElapsedTime() + 1.0;
	while (timer.getElapsedTime() < timeout)
	{
		alGetSourcei(m_source, AL_BUFFERS_PROCESSED, &processed);
		if (processed > 0)
			break;
			
		currentThread->yield();
	}
}

void SoundDriverOpenAL::submit(const SoundBlock& soundBlock)
{
	ALuint buffer;

	// Pop processed buffer from queue.
	if (m_submitted >= sizeof_array(m_buffers))
		alSourceUnqueueBuffers(m_source, 1, &buffer);
	else
		buffer = m_buffers[m_submitted];
	
	if (m_desc.bitsPerSample == 8)
	{
		for (int i = 0; i < soundBlock.maxChannel; ++i)
		{
			writeSamples< uint8_t >(
				m_data.ptr() + i,
				soundBlock.samples[i],
				soundBlock.samplesCount,
				m_desc.hwChannels
			);
		}
	}
	else if (m_desc.bitsPerSample == 16)
	{
		for (int i = 0; i < soundBlock.maxChannel; ++i)
		{
			writeSamples< int16_t >(
				m_data.ptr() + i * sizeof(int16_t),
				soundBlock.samples[i],
				soundBlock.samplesCount,
				m_desc.hwChannels
			);
		}
	}
	
	// Fill buffer with sound block.
	alBufferData(
		buffer,
		m_format,
		m_data.ptr(),
		soundBlock.samplesCount * m_desc.hwChannels * m_desc.bitsPerSample / 8,
		m_desc.sampleRate
	);
	
	// Push buffer onto queue.
	alSourceQueueBuffers(m_source, 1, &buffer);
	
	// Ensure source is still playing.
	ALint state;
	alGetSourcei(m_source, AL_SOURCE_STATE, &state);
	if (state != AL_PLAYING)
		alSourcePlay(m_source);
		
	++m_submitted;
}

	}
}
