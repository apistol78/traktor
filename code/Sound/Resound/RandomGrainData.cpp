#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Sound/Resound/IGrainFactory.h"
#include "Sound/Resound/RandomGrain.h"
#include "Sound/Resound/RandomGrainData.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.RandomGrainData", 0, RandomGrainData, IGrainData)

RandomGrainData::RandomGrainData()
:	m_humanize(false)
{
}

Ref< IGrain > RandomGrainData::createInstance(IGrainFactory* grainFactory) const
{
	RefArray< IGrain > grains;

	grains.resize(m_grains.size());
	for (uint32_t i = 0; i < m_grains.size(); ++i)
	{
		grains[i] = grainFactory->createInstance(m_grains[i]);
		if (!grains[i])
			return 0;
	}

	return new RandomGrain(grains, m_humanize);
}

void RandomGrainData::serialize(ISerializer& s)
{
	s >> MemberRefArray< IGrainData >(L"grains", m_grains);
	s >> Member< bool >(L"humanize", m_humanize);
}

	}
}
