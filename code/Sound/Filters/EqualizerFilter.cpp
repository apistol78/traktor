#include <cstring>
#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "Core/Memory/IAllocator.h"
#include "Core/Memory/MemoryConfig.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Sound/Filters/EqualizerFilter.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

struct EqualizerFilterInstance : public RefCountImpl< IFilterInstance >
{
	float m_historySamples[2][2];
	float m_historyFiltered[2][2];

	void* operator new (size_t size) {
		return getAllocator()->alloc(size, 16, T_FILE_LINE);
	}

	void operator delete (void* ptr) {
		getAllocator()->free(ptr);
	}
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.EqualizerFilter", 0, EqualizerFilter, IFilter)

EqualizerFilter::EqualizerFilter(float gain)
:	m_gain(gain)
{
}

Ref< IFilterInstance > EqualizerFilter::createInstance() const
{
	Ref< EqualizerFilterInstance > instance = new EqualizerFilterInstance();
	std::memset(instance->m_historySamples, 0, sizeof(instance->m_historySamples));
	std::memset(instance->m_historyFiltered, 0, sizeof(instance->m_historyFiltered));
	return instance;
}

void EqualizerFilter::apply(IFilterInstance* instance, SoundBlock& outBlock) const
{
	EqualizerFilterInstance* efi = static_cast< EqualizerFilterInstance* >(instance);
	for (uint32_t i = 0; i < outBlock.maxChannel; ++i)
	{
		float* samples = outBlock.samples[i];
		for (uint32_t j = 0; j < outBlock.samplesCount; ++j)
		{
			float filtered = samples[j] + 2 * efi->m_historySamples[i][0] + efi->m_historySamples[i][1] - (1.0f / 4.0f) * efi->m_historyFiltered[i][0] + (3.0f / 8.0f) * efi->m_historyFiltered[i][1];
			
			efi->m_historySamples[i][1] = efi->m_historySamples[i][0];
			efi->m_historySamples[i][0] = samples[j];

			efi->m_historyFiltered[i][1] = efi->m_historyFiltered[i][0];
			efi->m_historyFiltered[i][0] = filtered;

			samples[j] = filtered * m_gain;
		}
	}
}

void EqualizerFilter::serialize(ISerializer& s)
{
	s >> Member< float >(L"gain", m_gain);
}

	}
}
