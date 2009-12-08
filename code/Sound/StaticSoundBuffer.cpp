#include "Sound/StaticSoundBuffer.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

struct StaticSoundBufferCursor : public RefCountImpl< ISoundBufferCursor >
{
	double m_time;

	StaticSoundBufferCursor()
	:	m_time(0.0)
	{
	}

	virtual void setCursor(double time)
	{
		m_time = time;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.StaticSoundBuffer", StaticSoundBuffer, ISoundBuffer)

StaticSoundBuffer::StaticSoundBuffer()
:	m_sampleRate(0)
,	m_samplesCount(0)
,	m_channelsCount(0)
{
}

StaticSoundBuffer::~StaticSoundBuffer()
{
	destroy();
}

bool StaticSoundBuffer::create(uint32_t sampleRate, uint32_t samplesCount, uint32_t channelsCount)
{
	m_sampleRate = sampleRate;
	m_samplesCount = samplesCount;
	m_channelsCount = channelsCount;

	for (uint32_t i = 0; i < m_channelsCount; ++i)
	{
		m_samples[i].reset(new int16_t [m_samplesCount]);
		if (!m_samples[i].ptr())
			return false;
	}

	return true;
}

void StaticSoundBuffer::destroy()
{
	for (uint32_t i = 0; i < m_channelsCount; ++i)
		m_samples[i].release();
}

int16_t* StaticSoundBuffer::getSamplesData(uint32_t channel)
{
	return m_samples[channel].ptr();
}

Ref< ISoundBufferCursor > StaticSoundBuffer::createCursor()
{
	return new StaticSoundBufferCursor();
}

bool StaticSoundBuffer::getBlock(const ISoundBufferCursor* cursor, SoundBlock& outBlock)
{
	double time = static_cast< const StaticSoundBufferCursor* >(cursor)->m_time;
	uint32_t samplePosition = uint32_t(time * m_sampleRate);

	if (samplePosition >= m_samplesCount)
		return false;

	uint32_t samplesCount = m_samplesCount - samplePosition;
	samplesCount = std::min< uint32_t >(samplesCount, outBlock.samplesCount);
	samplesCount = std::min< uint32_t >(samplesCount, sizeof_array(m_blocks[0]));

	for (uint32_t i = 0; i < m_channelsCount; ++i)
	{
		outBlock.samples[i] = m_blocks[i];
		for (uint32_t j = 0; j < samplesCount; ++j)
			m_blocks[i][j] = float(m_samples[i][samplePosition + j] / 32767.0f);
	}

	outBlock.samplesCount = samplesCount;
	outBlock.sampleRate = m_sampleRate;
	outBlock.maxChannel = m_channelsCount;

	return true;
}

	}
}
