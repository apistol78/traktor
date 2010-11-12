#include "Core/Memory/BlockAllocator.h"
#include "Core/Misc/Align.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Semaphore.h"
#include "Sound/StaticSoundBuffer.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

const uint32_t c_blockSize = sizeof(float) * (4096 + 16);
const uint32_t c_blockCount = 32;

class StaticBufferHeap
{
public:
	static StaticBufferHeap& getInstance()
	{
		static StaticBufferHeap s_instance;
		return s_instance;
	}

	void* alloc()
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		return m_blockAllocator.alloc();
	}

	void free(void* ptr)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		m_blockAllocator.free(ptr);
	}

private:
	void* m_block;
	BlockAllocator m_blockAllocator;
	Semaphore m_lock;

	StaticBufferHeap()
	:	m_block(Alloc::acquireAlign(c_blockSize * c_blockCount, 16, T_FILE_LINE))
	,	m_blockAllocator(m_block, c_blockCount, c_blockSize)
	{
	}

	~StaticBufferHeap()
	{
		Alloc::freeAlign(m_block);
	}
};

struct StaticSoundBufferCursor : public RefCountImpl< ISoundBufferCursor >
{
	uint32_t m_channelsCount;
	uint32_t m_position;
	float* m_samples[SbcMaxChannelCount];

	StaticSoundBufferCursor(uint32_t channelsCount)
	:	m_channelsCount(channelsCount)
	,	m_position(0)
	{
		std::memset(m_samples, 0, sizeof(m_samples));
	}

	virtual ~StaticSoundBufferCursor()
	{
		for (uint32_t i = 0; i < sizeof_array(m_samples); ++i)
		{
			if (m_samples[i])
				StaticBufferHeap::getInstance().free(m_samples[i]);
		}
	}

	bool create()
	{
		for (uint32_t i = 0; i < m_channelsCount; ++i)
		{
			m_samples[i] = (float*)StaticBufferHeap::getInstance().alloc();
			if (!m_samples[i])
				return false;
		}

		reset();
		return true;
	}

	virtual void reset()
	{
		m_position = 0;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.StaticSoundBuffer", StaticSoundBuffer, ISoundBuffer)

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
	Ref< StaticSoundBufferCursor > cursor = new StaticSoundBufferCursor(m_channelsCount);
	if (cursor->create())
		return cursor;
	else
		return 0;
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
		outBlock.samples[i] = ssbc->m_samples[i];
		for (uint32_t j = 0; j < samplesCount; ++j)
			ssbc->m_samples[i][j] = float(m_samples[i][position + j] / 32767.0f);
	}

	outBlock.samplesCount = samplesCount;
	outBlock.sampleRate = m_sampleRate;
	outBlock.maxChannel = m_channelsCount;

	ssbc->m_position += samplesCount;
	return true;
}

	}
}
