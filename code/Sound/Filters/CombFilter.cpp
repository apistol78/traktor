/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cstring>
#include "Core/Containers/AlignedVector.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Sound/Filters/CombFilter.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

struct CombFilterInstance : public RefCountImpl< IFilterInstance >
{
	AlignedVector< float > m_history[2];
	float m_last[2];
	uint32_t m_index[2];

	void* operator new (size_t size) {
		return getAllocator()->alloc(size, 16, T_FILE_LINE);
	}

	void operator delete (void* ptr) {
		getAllocator()->free(ptr);
	}
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.CombFilter", 0, CombFilter, IFilter)

CombFilter::CombFilter(uint32_t samplesLength, float feedback, float damp)
:	m_samplesLength(samplesLength)
,	m_feedback(feedback)
,	m_damp(damp)
{

}

Ref< IFilterInstance > CombFilter::createInstance() const
{
	Ref< CombFilterInstance > instance = new CombFilterInstance();

	instance->m_history[0].resize(m_samplesLength, 0.0f);
	instance->m_history[1].resize(m_samplesLength, 0.0f);

	instance->m_last[0] =
	instance->m_last[1] = 0.0f;

	instance->m_index[0] =
	instance->m_index[1] = 0;

	return instance;
}

void CombFilter::apply(IFilterInstance* instance, SoundBlock& outBlock) const
{
	CombFilterInstance* cfi = static_cast< CombFilterInstance* >(instance);
	for (uint32_t i = 0; i < outBlock.samplesCount; ++i)
	{
		for (uint32_t j = 0; j < outBlock.maxChannel; ++j)
		{
			cfi->m_last[j] = cfi->m_history[j][cfi->m_index[j]] * (1.0f - m_damp) + cfi->m_last[j] * m_damp;
			cfi->m_history[j][cfi->m_index[j]] = outBlock.samples[j][i] + cfi->m_last[j] * m_feedback;
			
			if (++cfi->m_index[j] >= cfi->m_history[j].size())
				cfi->m_index[j] = 0;

			outBlock.samples[j][i] = cfi->m_history[j][cfi->m_index[j]];
		}
	}
}

void CombFilter::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"samplesLength", m_samplesLength);
	s >> Member< float >(L"feedBack", m_feedback);
	s >> Member< float >(L"damp", m_damp);
}

	}
}
