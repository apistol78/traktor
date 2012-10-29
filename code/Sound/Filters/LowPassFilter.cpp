#include <cstring>
#include "Core/Math/Const.h"
#include "Core/Memory/IAllocator.h"
#include "Core/Memory/MemoryConfig.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Sound/Filters/LowPassFilter.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

struct LowPassFilterInstance : public RefCountImpl< IFilterInstance >
{
	float m_history[SbcMaxChannelCount];

	void* operator new (size_t size) {
		return getAllocator()->alloc(size, 16, T_FILE_LINE);
	}

	void operator delete (void* ptr) {
		getAllocator()->free(ptr);
	}
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.LowPassFilter", 0, LowPassFilter, IFilter)

LowPassFilter::LowPassFilter(float cutOff)
:	m_cutOff(cutOff)
{
}

Ref< IFilterInstance > LowPassFilter::createInstance() const
{
	Ref< LowPassFilterInstance > lpfi = new LowPassFilterInstance();
	std::memset(lpfi->m_history, 0, sizeof(lpfi->m_history));
	return lpfi;
}

void LowPassFilter::apply(IFilterInstance* instance, SoundBlock& outBlock) const
{
	LowPassFilterInstance* lpfi = static_cast< LowPassFilterInstance* >(instance);
	if (m_cutOff > FUZZY_EPSILON)
	{
		float dt = 1.0f / outBlock.sampleRate;
		float alpha = dt / (dt + 1.0f / m_cutOff);

		for (uint32_t j = 0; j < outBlock.maxChannel; ++j)
		{
			float* samples = outBlock.samples[j];
			float& history = lpfi->m_history[j];

			for (uint32_t i = 0; i < outBlock.samplesCount; i += 4)
			{
				samples[i+0] = (samples[i+0] - history) * alpha + history;
				samples[i+1] = (samples[i+1] - samples[i+0]) * alpha + samples[i+0];
				samples[i+2] = (samples[i+2] - samples[i+1]) * alpha + samples[i+1];
				samples[i+3] = (samples[i+3] - samples[i+2]) * alpha + samples[i+2];
				history = samples[i+3];
			}
		}
	}
}

bool LowPassFilter::serialize(ISerializer& s)
{
	return s >> Member< float >(L"cutOff", m_cutOff);
}

	}
}
