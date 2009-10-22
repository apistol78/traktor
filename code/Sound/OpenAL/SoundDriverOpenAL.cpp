#include "Sound/OpenAL/SoundDriverOpenAL.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.SoundDriverOpenAL", SoundDriverOpenAL, ISoundDriver)

SoundDriverOpenAL::SoundDriverOpenAL()
:	m_source(0)
,	m_submitted(0)
{
	m_buffers[0] =
	m_buffers[1] = 0;
}

bool SoundDriverOpenAL::create(const SoundDriverCreateDesc& desc)
{
	alGenBuffers(sizeof_array(m_buffers), m_buffers);
	alGenSources(1, &m_source);
	
	alSource3f(m_source, AL_POSITION, 0.0f, 0.0f, 0.0f);
	alSource3f(m_source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
	alSource3f(m_source, AL_DIRECTION, 0.0f, 0.0f, 0.0f);
	alSourcef(m_source, AL_ROLLOFF_FACTOR, 0.0f);
	alSourcei(m_source, AL_SOURCE_RELATIVE, AL_TRUE);
	
	alSourceQueueBuffers(m_source, 2, m_buffers);
	
	return true;
}

void SoundDriverOpenAL::destroy()
{
	alSourceStop(m_source);
	alDeleteSources(1, &m_source);
	alDeleteBuffers(sizeof_array(m_buffers), m_buffers);
}

void SoundDriverOpenAL::wait()
{
	if (m_submitted < sizeof_array(m_buffers))
		return;
	
	// Wait until at least one buffer has been processed by OpenAL.
	ALint processed;
	do
	{
		alGetSourcei(m_source, AL_BUFFERS_PROCESSED, &processed);
	}
	while (processed <= 0);	
}

void SoundDriverOpenAL::submit(const SoundBlock& soundBlock)
{
	ALuint buffer;

	if (m_submitted < sizeof_array(m_buffers))
		// Buffers havn't been queued yet, just pick em.
		buffer = m_buffers[m_submitted];
	else	
		// Pop ready buffer from queue.
		alSourceUnqueueBuffers(m_source, 1, &buffer);
	
	// Fill buffer with sound block.
	//alBufferData(buffer, m_format, data, size, rate);
	
	// Push buffer onto queue.
	alSourceQueueBuffers(m_source, 1, &buffer);
	
	if (!m_submitted)
		alSourcePlay(m_source);

	m_submitted++;
}

	}
}
