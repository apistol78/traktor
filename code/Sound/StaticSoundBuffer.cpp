#include <cstring>

#if defined(_MSC_VER) && !defined(WINCE) && !defined(_XBOX)
#	define USE_XMM_INTRINSICS
#	include <emmintrin.h>
#elif defined(__APPLE__)
#	include <TargetConditionals.h>
#	if TARGET_CPU_X86 && TARGET_OS_MAC
#		define USE_XMM_INTRINSICS
#		include <emmintrin.h>
#	endif
#endif

#include "Core/Math/MathConfig.h"
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
const uint32_t c_blockCount = 512;

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
		StaticBufferHeap& heap = StaticBufferHeap::getInstance();
		for (uint32_t i = 0; i < sizeof_array(m_samples); ++i)
		{
			if (m_samples[i])
				heap.free(m_samples[i]);
		}
	}

	bool create()
	{
		StaticBufferHeap& heap = StaticBufferHeap::getInstance();
		for (uint32_t i = 0; i < m_channelsCount; ++i)
		{
			m_samples[i] = (float*)heap.alloc();
			if (!m_samples[i])
				return false;
		}

		reset();
		return true;
	}

	virtual void setParameter(handle_t id, float parameter)
	{
	}

	virtual void disableRepeat()
	{
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

	for (int32_t i = 0; i < m_channelsCount; ++i)
	{
		m_samples[i].reset((int16_t*)Alloc::acquireAlign(m_samplesCount * sizeof(int16_t), 16, T_FILE_LINE));
		if (!m_samples[i].ptr())
			return false;
	}

	return true;
}

void StaticSoundBuffer::destroy()
{
	for (int32_t i = 0; i < m_channelsCount; ++i)
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

bool StaticSoundBuffer::getBlock(ISoundBufferCursor* cursor, const ISoundMixer* mixer, SoundBlock& outBlock) const
{
	StaticSoundBufferCursor* ssbc = static_cast< StaticSoundBufferCursor* >(cursor);

	int32_t position = ssbc->m_position;
	if (position >= m_samplesCount)
		return false;

	int32_t samplesCount = m_samplesCount - position;
	samplesCount = std::min< int32_t >(samplesCount, outBlock.samplesCount);
	samplesCount = alignDown(samplesCount, 4);
	samplesCount = std::min< int32_t >(samplesCount, 4096);

	if (samplesCount <= 0)
		return false;

#if defined(USE_XMM_INTRINSICS)
	const float T_ALIGN16 c_scale[] = { 1.0f / 32768.0f, 1.0f / 32768.0f, 1.0f / 32768.0f, 1.0f / 32768.0f };
	__m128 scale = _mm_load_ps(c_scale);
#endif

	for (int32_t i = 0; i < m_channelsCount; ++i)
	{
		outBlock.samples[i] = ssbc->m_samples[i];

		int32_t j = 0;

#if defined(USE_XMM_INTRINSICS)
		for (; j < samplesCount - 7; j += 8)
		{
			__m128i is = _mm_load_si128((const __m128i*)&m_samples[i][position + j]);
			__m128i tl0 = _mm_unpacklo_epi16(is, is);
			__m128i isl = _mm_srai_epi32(tl0, 16);
			__m128i th0 = _mm_unpackhi_epi16(is, is);
			__m128i ish = _mm_srai_epi32(th0, 16);
			__m128 fl = _mm_cvtepi32_ps(isl);
			__m128 fh = _mm_cvtepi32_ps(ish);
			fl = _mm_mul_ps(fl, scale);
			fh = _mm_mul_ps(fh, scale);
			_mm_store_ps(&ssbc->m_samples[i][j], fl);
			_mm_store_ps(&ssbc->m_samples[i][j + 4], fh);
		}
#endif

		for (; j < samplesCount; ++j)
			ssbc->m_samples[i][j] = float(m_samples[i][position + j] / 32768.0f);
	}

	outBlock.samplesCount = samplesCount;
	outBlock.sampleRate = m_sampleRate;
	outBlock.maxChannel = m_channelsCount;

	ssbc->m_position += samplesCount;
	return true;
}

	}
}
