#include <cstring>
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

	float dt = 1.0f / outBlock.sampleRate;
	float alpha = dt / (dt + 1.0f / m_cutOff);

	for (uint32_t i = 0; i < outBlock.samplesCount; ++i)
	{
		for (uint32_t j = 0; j < outBlock.maxChannel; ++j)
		{
			outBlock.samples[j][i] = outBlock.samples[j][i] * alpha + lpfi->m_history[j] * (1.0f - alpha);
			lpfi->m_history[j] = outBlock.samples[j][i];
		}
	}
}

bool LowPassFilter::serialize(ISerializer& s)
{
	return s >> Member< float >(L"cutOff", m_cutOff);
}

	}
}
