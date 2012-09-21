#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRef.h"
#include "Sound/Resound/TriggerGrain.h"
#include "Sound/Resound/TriggerGrainData.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.TriggerGrainData", 0, TriggerGrainData, IGrainData)

TriggerGrainData::TriggerGrainData()
:	m_position(0.0f)
,	m_rate(0.0f)
{
}

Ref< IGrain > TriggerGrainData::createInstance(resource::IResourceManager* resourceManager) const
{
	Ref< IGrain > grain = m_grain->createInstance(resourceManager);
	if (!grain)
		return 0;

	return new TriggerGrain(m_position, m_rate, grain);
}

bool TriggerGrainData::serialize(ISerializer& s)
{
	s >> Member< float >(L"position", m_position);
	s >> Member< float >(L"rate", m_rate);
	s >> MemberRef< IGrainData >(L"grain", m_grain);
	return true;
}

	}
}
