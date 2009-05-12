#include "Sound/Filters/CombFilter.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.CombFilter", CombFilter, Filter)

CombFilter::CombFilter(uint32_t samplesLength, float feedback, float damp)
:	m_feedback(feedback)
,	m_damp(damp)
{
	m_history[0].resize(samplesLength);
	m_history[1].resize(samplesLength);

	memset(&m_history[0][0], 0, samplesLength * sizeof(float));
	memset(&m_history[1][0], 0, samplesLength * sizeof(float));

	m_last[0] =
	m_last[1] = 0.0f;

	m_index[0] =
	m_index[1] = 0;
}

void CombFilter::apply(SoundBlock& outBlock)
{
	for (uint32_t i = 0; i < outBlock.samplesCount; ++i)
	{
		for (uint32_t j = 0; j < outBlock.channels; ++j)
		{
			m_last[j] = m_history[j][m_index[j]] * (1.0f - m_damp) + m_last[j] * m_damp;
			m_history[j][m_index[j]] = outBlock.samples[j][i] + m_last[j] * m_feedback;
			if (++m_index[j] >= m_history[j].size())
				m_index[j] = 0;
			outBlock.samples[j][i] = m_history[j][m_index[j]];
		}
	}
}

	}
}
