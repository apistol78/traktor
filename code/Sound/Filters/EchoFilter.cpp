#include <cstring>
#include "Core/Containers/CircularVector.h"
#include "Core/Math/Const.h"
#include "Core/Memory/Alloc.h"
#include "Core/Misc/Align.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Sound/Filters/EchoFilter.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

struct EchoFilterInstance : public RefCountImpl< IFilterInstance >
{
	float* m_history[SbcMaxChannelCount];
	int32_t m_count[SbcMaxChannelCount];
	int32_t m_front[SbcMaxChannelCount];
	int32_t m_size;

	EchoFilterInstance()
	{
		m_history[0] = 0;
	}

	virtual ~EchoFilterInstance()
	{
		T_EXCEPTION_GUARD_BEGIN

		if (m_history[0])
			Alloc::freeAlign(m_history[0]);

		T_EXCEPTION_GUARD_END
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
:	m_delay(delay)
,	m_decay(decay)
,	m_wetMix(wetMix)
,	m_dryMix(dryMix)
{
}

Ref< IFilterInstance > EchoFilter::createInstance() const
{
	int32_t nechos = int32_t(1.0f / m_decay);
	int32_t samples = nechos * alignUp(int32_t(m_delay * 48000), 4);

	float* history = static_cast< float* >(Alloc::acquireAlign(samples * sizeof(float) * SbcMaxChannelCount, 16, T_FILE_LINE));
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

	return efi;
}

void EchoFilter::apply(IFilterInstance* instance, SoundBlock& outBlock) const
{
	EchoFilterInstance* efi = static_cast< EchoFilterInstance* >(instance);

	int32_t nechos = int32_t(1.0f / m_decay);
	int32_t delay = int32_t(m_delay * outBlock.sampleRate);
	Scalar decay0(m_decay);

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
			Scalar decay(1.0f);

			for (int32_t k = 1; k < nechos; ++k)
			{
				decay -= decay0;
				int32_t offset = alignUp(k * delay, 4);
				if (offset < maxOffset)
				{
					int32_t index = front - offset;
					if (index < 0)
						index += efi->m_size;

					Vector4 h = Vector4::loadAligned(&history[index]);
					echo += h * decay;
				}
			}

			Vector4 r = s * m_dryMix + echo * m_wetMix;
			r.storeAligned(&samples[j]);
		}
	}
}

bool EchoFilter::serialize(ISerializer& s)
{
	s >> Member< float >(L"delay", m_delay);
	s >> Member< float >(L"decay", m_decay);
	s >> Member< Scalar >(L"wetMix", m_wetMix);
	s >> Member< Scalar >(L"dryMix", m_dryMix);
	return true;
}

	}
}
