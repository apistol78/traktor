#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Sound/Resound/IGrainFactory.h"
#include "Sound/Resound/SimultaneousGrain.h"
#include "Sound/Resound/SimultaneousGrainData.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.SimultaneousGrainData", 0, SimultaneousGrainData, IGrainData)

Ref< IGrain > SimultaneousGrainData::createInstance(IGrainFactory* grainFactory) const
{
	RefArray< IGrain > grains;

	grains.resize(m_grains.size());
	for (uint32_t i = 0; i < m_grains.size(); ++i)
	{
		grains[i] = grainFactory->createInstance(m_grains[i]);
		if (!grains[i])
			return 0;
	}

	return new SimultaneousGrain(grains);
}

void SimultaneousGrainData::serialize(ISerializer& s)
{
	s >> MemberRefArray< IGrainData >(L"grains", m_grains);
}

	}
}
