#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRef.h"
#include "Sound/Resound/RepeatGrain.h"
#include "Sound/Resound/RepeatGrainData.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.RepeatGrainData", 0, RepeatGrainData, IGrainData)

RepeatGrainData::RepeatGrainData()
:	m_count(0)
{
}

Ref< IGrain > RepeatGrainData::createInstance(resource::IResourceManager* resourceManager) const
{
	Ref< IGrain > grain = m_grain->createInstance(resourceManager);
	if (!grain)
		return 0;

	return new RepeatGrain(m_count, grain);
}

void RepeatGrainData::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"count", m_count);
	s >> MemberRef< IGrainData >(L"grain", m_grain);
}

	}
}
