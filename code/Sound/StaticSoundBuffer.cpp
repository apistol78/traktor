#include "Compress/Zip/InflateStream.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Memory/Alloc.h"
#include "Core/Memory/BlockAllocator.h"
#include "Core/Misc/Align.h"
#include "Core/Misc/Endian.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Semaphore.h"
#include "Database/Instance.h"
#include "Sound/StaticSoundBuffer.h"
#include "Sound/StaticSoundResource.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

const uint32_t c_blockSize = sizeof(float) * (4096 + 16);
const uint32_t c_blockCount = 64;

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
	Ref< const StaticSoundResource > m_resource;
	Ref< db::Instance > m_resourceInstance;
	Ref< IStream > m_resourceStream;
	Ref< IStream > m_stream;
	float* m_samples[SbcMaxChannelCount];
	uint32_t m_position;

	StaticSoundBufferCursor(const StaticSoundResource* resource, db::Instance* resourceInstance)
	:	m_resource(resource)
	,	m_resourceInstance(resourceInstance)
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
		for (uint32_t i = 0; i < m_resource->getChannelsCount(); ++i)
		{
			m_samples[i] = (float*)StaticBufferHeap::getInstance().alloc();
			if (!m_samples[i])
				return false;
		}

		m_resourceStream = m_resourceInstance->readData(L"Data");
		if (!m_resourceStream)
			return false;

		reset();
		return true;
	}

	virtual void reset()
	{
		if (m_resourceStream)
		{
			m_resourceStream->seek(IStream::SeekSet, 0);
			if (m_resource->getFlags() & SrfZLib)
				m_stream = new compress::InflateStream(m_resourceStream);
			else
				m_stream = m_resourceStream;
		}
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.StaticSoundBuffer", StaticSoundBuffer, ISoundBuffer)

StaticSoundBuffer::~StaticSoundBuffer()
{
	destroy();
}

bool StaticSoundBuffer::create(const StaticSoundResource* resource, db::Instance* resourceInstance)
{
	m_resource = resource;
	m_resourceInstance = resourceInstance;
	
	uint32_t readBufferSize = m_resource->getChannelsCount() * 4096;
	m_readBuffer.reset(new int16_t [readBufferSize]);

	return true;
}

void StaticSoundBuffer::destroy()
{
	m_resource = 0;
	m_resourceInstance = 0;
	m_readBuffer.release();
}

Ref< ISoundBufferCursor > StaticSoundBuffer::createCursor() const
{
	Ref< StaticSoundBufferCursor > cursor = new StaticSoundBufferCursor(
		m_resource,
		m_resourceInstance
	);
	if (cursor->create())
		return cursor;
	else
		return 0;
}

bool StaticSoundBuffer::getBlock(ISoundBufferCursor* cursor, SoundBlock& outBlock) const
{
	StaticSoundBufferCursor* ssbc = static_cast< StaticSoundBufferCursor* >(cursor);

	uint32_t samplesCount = m_resource->getSamplesCount();
	uint32_t channelCount = m_resource->getChannelsCount();

	if (ssbc->m_position >= samplesCount)
		return false;

	samplesCount = samplesCount - ssbc->m_position;
	samplesCount = std::min< uint32_t >(samplesCount, outBlock.samplesCount);
	samplesCount = std::min< uint32_t >(samplesCount, 4096);
	samplesCount = alignUp(samplesCount, 4);

	uint32_t byteCount = channelCount * samplesCount * sizeof(int16_t);
	int32_t bytesRead = ssbc->m_stream->read(m_readBuffer.ptr(), byteCount);
	if (bytesRead < 0)
		return false;
		
	const int16_t* bufferPtr = m_readBuffer.c_ptr();
	for (uint32_t i = 0; i < samplesCount; ++i)
	{
		for (uint32_t j = 0; j < channelCount; ++j)
		{
			ssbc->m_samples[j][i] = float(*bufferPtr++) / 32767.0f;
		}
	}
	
	for (uint32_t i = 0; i < channelCount; ++i)
		outBlock.samples[i] = ssbc->m_samples[i];

	outBlock.samplesCount = samplesCount;
	outBlock.sampleRate = m_resource->getSampleRate();
	outBlock.maxChannel = channelCount;

	ssbc->m_position += samplesCount;
	return true;
}

	}
}
