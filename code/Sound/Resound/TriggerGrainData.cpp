#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRef.h"
#include "Sound/Resound/IGrainFactory.h"
#include "Sound/Resound/TriggerGrain.h"
#include "Sound/Resound/TriggerGrainData.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.TriggerGrainData", 1, TriggerGrainData, IGrainData)

TriggerGrainData::TriggerGrainData()
:	m_position(0.0f)
,	m_rate(0.0f)
{
}

Ref< IGrain > TriggerGrainData::createInstance(IGrainFactory* grainFactory) const
{
	Ref< IGrain > grain = grainFactory->createInstance(m_grain);
	if (!grain)
		return 0;

	return new TriggerGrain(
		getParameterHandle(m_id),
		m_position,
		m_rate,
		grain
	);
}

void TriggerGrainData::serialize(ISerializer& s)
{
	if (s.getVersion() >= 1)
		s >> Member< std::wstring >(L"id", m_id);

	s >> Member< float >(L"position", m_position);
	s >> Member< float >(L"rate", m_rate);
	s >> MemberRef< IGrainData >(L"grain", m_grain);
}

	}
}
