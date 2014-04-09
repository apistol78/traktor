#include <cstring>
#include "Core/Containers/CircularVector.h"
#include "Core/Math/Const.h"
#include "Core/Memory/Alloc.h"
#include "Core/Memory/BlockAllocator.h"
#include "Core/Memory/IAllocator.h"
#include "Core/Memory/MemoryConfig.h"
#include "Core/Misc/Align.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Semaphore.h"
#include "Sound/Filters/EchoFilter.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

const int32_t c_maxEchos = 4;
const float c_maxDelay = 1.0f;
const int32_t c_maxSimultaneousEchos = 8;

Semaphore s_historyAllocLock;
BlockAllocator* s_historyAlloc = 0;

float* allocHistory()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(s_historyAllocLock);
	if (!s_historyAlloc)
	{
		int32_t samples = c_maxEchos * alignUp(int32_t(c_maxDelay * 48000), 4);
		int32_t size = alignUp(samples * sizeof(float) * SbcMaxChannelCount, 16);

		void* ptr = Alloc::acquireAlign(c_maxSimultaneousEchos * size, 16, T_FILE_LINE);
		T_FATAL_ASSERT (ptr);

		s_historyAlloc = new BlockAllocator(ptr, c_maxSimultaneousEchos, size);
		T_FATAL_ASSERT (s_historyAlloc);
	}

	return static_cast< float* >(s_historyAlloc->alloc());
}

void freeHistory(float* ptr)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(s_historyAllocLock);
	T_ASSERT (s_historyAlloc);
	s_historyAlloc->free(ptr);
}

struct EchoFilterInstance : public RefCountImpl< IFilterInstance >
{
	float* m_history[SbcMaxChannelCount];
	int32_t m_count[SbcMaxChannelCount];
	int32_t m_front[SbcMaxChannelCount];
	int32_t m_size;
	Scalar m_decay[16];

	EchoFilterInstance()
	{
		m_history[0] = 0;
	}

	virtual ~EchoFilterInstance()
	{
		T_EXCEPTION_GUARD_BEGIN

		if (m_history[0])
			freeHistory(m_history[0]);

		T_EXCEPTION_GUARD_END
	}

	void* operator new (size_t size) {
		return getAllocator()->alloc(size, 16, T_FILE_LINE);
	}

	void operator delete (void* ptr) {
		getAllocator()->free(ptr);
	}
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.EchoFilter", 0, EchoFilter, IFilter)

EchoFilter::EchoFilter()
:	m_delay(0.0f)
,	m_decay(0.0f)
,	m_wetMix(0.0f)
,	m_dryMix(1.0f)
{
}

EchoFilter::EchoFilter(float delay, float decay, float wetMix, float dryMix)
:	m_delay(min(delay, c_maxDelay))
,	m_decay(decay)
,	m_wetMix(wetMix)
,	m_dryMix(dryMix)
{
}

Ref< IFilterInstance > EchoFilter::createInstance() const
{
	int32_t nechos = min(int32_t(1.0f / m_decay), c_maxEchos);
	int32_t samples = nechos * alignUp(int32_t(m_delay * 48000), 4);

	float* history = allocHistory();
	if (!history)
		return 0;

	Ref< EchoFilterInstance > efi = new EchoFilterInstance();

	for (int32_t i = 0; i < SbcMaxChannelCount; ++i)
	{
		efi->m_history[i] = history + i * samples;
		efi->m_count[i] = 0;
		efi->m_front[i] = 0;
	}

	efi->m_size = samples;

	for (int32_t k = 1; k < nechos; ++k)
		efi->m_decay[k] = Scalar(1.0f - std::pow(m_decay * k, 0.3f));

	return efi;
}

void EchoFilter::apply(IFilterInstance* instance, SoundBlock& outBlock) const
{
	EchoFilterInstance* efi = static_cast< EchoFilterInstance* >(instance);

	int32_t nechos = min(int32_t(1.0f / m_decay), c_maxEchos);
	int32_t delay = int32_t(m_delay * outBlock.sampleRate);

	for (uint32_t i = 0; i < outBlock.maxChannel; ++i)
	{
		float* samples = outBlock.samples[i];
		if (!samples)
			continue;

		float* history = efi->m_history[i];
		T_ASSERT (history);

		int32_t& front = efi->m_front[i];
		int32_t& count = efi->m_count[i];

		for (uint32_t j = 0; j < outBlock.samplesCount; j += 4)
		{
			Vector4 s = Vector4::loadAligned(&samples[j]);

			s.storeAligned(&history[front]);
			front = (front + 4) % efi->m_size;

			if (count < efi->m_size)
				count += 4;

			int32_t maxOffset = count;
			Vector4 echo = Vector4::zero();

			for (int32_t k = 1; k < nechos; ++k)
			{
				int32_t offset = alignUp(k * delay, 4);
				if (offset < maxOffset)
				{
					int32_t index = front - offset;
					if (index < 0)
						index += efi->m_size;

					Vector4 h = Vector4::loadAligned(&history[index]);
					echo += h * efi->m_decay[k];
				}
			}

			Vector4 r = s * m_dryMix + echo * m_wetMix;
			r.storeAligned(&samples[j]);
		}
	}
}

void EchoFilter::serialize(ISerializer& s)
{
	s >> Member< float >(L"delay", m_delay);
	s >> Member< float >(L"decay", m_decay);
	s >> Member< Scalar >(L"wetMix", m_wetMix);
	s >> Member< Scalar >(L"dryMix", m_dryMix);
}

	}
}
