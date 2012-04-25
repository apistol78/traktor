#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Sound/Resound/SequenceGrain.h"
#include "Sound/Resound/SequenceGrainData.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.SequenceGrainData", 0, SequenceGrainData, IGrainData)

Ref< IGrain > SequenceGrainData::createInstance(resource::IResourceManager* resourceManager) const
{
	RefArray< IGrain > grains;

	grains.resize(m_grains.size());
	for (uint32_t i = 0; i < m_grains.size(); ++i)
	{
		grains[i] = m_grains[i]->createInstance(resourceManager);
		if (!grains[i])
			return 0;
	}

	return new SequenceGrain(grains);
}

bool SequenceGrainData::serialize(ISerializer& s)
{
	return s >> MemberRefArray< IGrainData >(L"grains", m_grains);
}

	}
}
