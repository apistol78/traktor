#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Sound/Resound/MuteGrain.h"
#include "Sound/Resound/MuteGrainData.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.MuteGrainData", 0, MuteGrainData, IGrainData)

MuteGrainData::MuteGrainData()
:	m_duration(0.0)
{
}

Ref< IGrain > MuteGrainData::createInstance(resource::IResourceManager* resourceManager) const
{
	return new MuteGrain(m_duration);
}

void MuteGrainData::serialize(ISerializer& s)
{
	s >> Member< double >(L"duration", m_duration);
}

	}
}
