#include "Core/Memory/Alloc.h"
#include "Core/Misc/Align.h"
#include "Sound/StaticSoundBuffer.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

struct StaticSoundBufferCursor : public RefCountImpl< ISoundBufferCursor >
{
	uint32_t m_position;
	float* m_blocks[SbcMaxChannelCount];

	StaticSoundBufferCursor(uint32_t channelsCount)
	:	m_position(0)
	{
		const uint32_t blockSize = sizeof(float) * (4096 + 16);	// 16 samples padding.

		std::memset(m_blocks, 0, sizeof(m_blocks));
		for (uint32_t i = 0; i < channelsCount; ++i)
		{
			m_blocks[i] = (float*)Alloc::acquireAlign(blockSize, 16, T_FILE_LINE);
			T_FATAL_ASSERT_M (m_blocks[i], L"Out of memory");

			std::memset(m_blocks[i], 0, blockSize);
		}
	}

	virtual ~StaticSoundBufferCursor()
	{
		for (uint32_t i = 0; i < sizeof_array(m_blocks); ++i)
		{
			if (m_blocks[i])
				Alloc::freeAlign(m_blocks[i]);
		}
	}

	virtual void reset()
	{
		m_position = 0;
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

Ref< ISoundBufferCursor > StaticSoundBuffer::createCursor() const
{
	return new StaticSoundBufferCursor(m_channelsCount);
}

bool StaticSoundBuffer::getBlock(ISoundBufferCursor* cursor, SoundBlock& outBlock) const
{
	StaticSoundBufferCursor* ssbc = static_cast< StaticSoundBufferCursor* >(cursor);

	uint32_t position = ssbc->m_position;
	if (position >= m_samplesCount)
		return false;

	uint32_t samplesCount = m_samplesCount - position;
	samplesCount = std::min< uint32_t >(samplesCount, outBlock.samplesCount);
	samplesCount = alignUp(samplesCount, 4);
	samplesCount = std::min< uint32_t >(samplesCount, 4096);

	for (uint32_t i = 0; i < m_channelsCount; ++i)
	{
		outBlock.samples[i] = ssbc->m_blocks[i];
		for (uint32_t j = 0; j < samplesCount; ++j)
			ssbc->m_blocks[i][j] = float(m_samples[i][position + j] / 32767.0f);
	}

	outBlock.samplesCount = samplesCount;
	outBlock.sampleRate = m_sampleRate;
	outBlock.maxChannel = m_channelsCount;

	ssbc->m_position += samplesCount;
	return true;
}

	}
}
