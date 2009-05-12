#include "Sound/Filters/EqualizerFilter.h"
#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.EqualizerFilter", EqualizerFilter, Filter)

EqualizerFilter::EqualizerFilter(float gain)
:	m_gain(gain)
{
	memset(m_historySamples, 0, sizeof(m_historySamples));
	memset(m_historyFiltered, 0, sizeof(m_historyFiltered));
}

void EqualizerFilter::apply(SoundBlock& outBlock)
{
	for (uint32_t i = 0; i < outBlock.channels; ++i)
	{
		float* samples = outBlock.samples[i];
		for (uint32_t j = 0; j < outBlock.samplesCount; ++j)
		{
			float filtered = samples[j] + 2 * m_historySamples[i][0] + m_historySamples[i][1] - (1.0f / 4.0f) * m_historyFiltered[i][0] + (3.0f / 8.0f) * m_historyFiltered[i][1];
			
			m_historySamples[i][1] = m_historySamples[i][0];
			m_historySamples[i][0] = samples[j];

			m_historyFiltered[i][1] = m_historyFiltered[i][0];
			m_historyFiltered[i][0] = filtered;

			samples[j] = filtered * m_gain;
		}
	}
}

	}
}
