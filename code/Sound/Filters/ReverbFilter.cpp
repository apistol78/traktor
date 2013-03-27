#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Float.h"
#include "Core/Memory/IAllocator.h"
#include "Core/Memory/MemoryConfig.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Sound/Filters/ReverbFilter.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

enum
{
	DelayCount = 8,
	SerialCount = 2
};

class Delay
{
public:
	Delay(uint32_t samples)
	:	m_count(0)
	{
		m_buffer.resize(samples, 0.0f);
	}

	void put(float s)
	{
		m_buffer[m_count] = s;
		m_count = (m_count + 1) % m_buffer.size();
	}

	float get() const
	{
		int32_t index = (m_count + 1) % m_buffer.size();
		return m_buffer[index];
	}

private:
	AlignedVector< float > m_buffer;
	int32_t m_count;
};

struct ReverbFilterInstance : public RefCountImpl< IFilterInstance >
{
	struct Channel
	{
		AutoPtr< Delay > delay[DelayCount];
		float history;
		float historyLF[4];
	};

	Channel m_channels[SbcMaxChannelCount];

	ReverbFilterInstance(int32_t delay)
	{
		for (uint32_t i = 0; i < sizeof_array(m_channels); ++i)
		{
			for (int32_t j = 0; j < DelayCount; ++j)
			{
				m_channels[i].delay[j].reset(new Delay(
					((j + 1) * delay * 441) / (DelayCount * 10)
				));
			}
			m_channels[i].history = 0.0f;
			m_channels[i].historyLF[0] = 0.0f;
			m_channels[i].historyLF[1] = 0.0f;
			m_channels[i].historyLF[2] = 0.0f;
			m_channels[i].historyLF[3] = 0.0f;
		}
	}

	void* operator new (size_t size) {
		return getAllocator()->alloc(size, 16, T_FILE_LINE);
	}

	void operator delete (void* ptr) {
		getAllocator()->free(ptr);
	}
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.ReverbFilter", 0, ReverbFilter, IFilter)

ReverbFilter::ReverbFilter()
:	m_delay(100)
,	m_duration(1.0f)
,	m_cutOff(22050.0f)
,	m_wet(1.0f)
{
}

ReverbFilter::ReverbFilter(
	int32_t delay,
	float duration,
	float cutOff,
	float wet
)
:	m_delay(delay)
,	m_duration(duration)
,	m_cutOff(cutOff)
,	m_wet(wet)
{
}

Ref< IFilterInstance > ReverbFilter::createInstance() const
{
	Ref< ReverbFilterInstance > instance = new ReverbFilterInstance(m_delay);
	return instance;
}

void ReverbFilter::apply(IFilterInstance* instance, SoundBlock& outBlock) const
{
	ReverbFilterInstance* rfi = static_cast< ReverbFilterInstance* >(instance);
	
	float dT = 1.0f / outBlock.sampleRate;
	float gain = 0.5f;
	float Krt = 1.0f - (dT * 1000.0f) / m_duration;
	float alpha = dT / (dT + 1.0f / m_cutOff);

	for (uint32_t i = 0; i < outBlock.maxChannel; ++i)
	{
		ReverbFilterInstance::Channel& channel = rfi->m_channels[i];
		float* samples = outBlock.samples[i];

		for (uint32_t j = 0; j < outBlock.samplesCount; ++j)
		{
			float Sin = samples[j];
			float Sout = 0.0f;

			float S = Sin + channel.history;

			for (uint32_t ii = 0; ii < 4; ++ii)
			{
				S = (S - channel.historyLF[ii]) * alpha + channel.historyLF[ii];
				channel.historyLF[ii] = S;

				for (uint32_t k = 0; k < 2; ++k)
				{
					float Sd = channel.delay[ii * 2 + k]->get();
					float S0 = S + Sd * -gain;
					S = Sd + S0 * gain;
					channel.delay[ii * 2 + k]->put(S0);
				}

				Sout += S;

				channel.history = S * Krt;

				S = S * Krt + samples[j];
			}

			Sout /= 4.0f;

			samples[j] = lerp(Sin, Sout, m_wet);
		}
	}
}

bool ReverbFilter::serialize(ISerializer& s)
{
	s >> Member< int32_t >(L"delay", m_delay, AttributeRange(1));
	s >> Member< float >(L"duration", m_duration, AttributeRange(0.0f));
	s >> Member< float >(L"cutOff", m_cutOff, AttributeRange(0.0f));
	s >> Member< float >(L"wet", m_wet, AttributeRange(0.0f, 1.0f));
	return true;
}

	}
}
