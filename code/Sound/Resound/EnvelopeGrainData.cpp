#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberStl.h"
#include "Sound/Resound/EnvelopeGrain.h"
#include "Sound/Resound/EnvelopeGrainData.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.EnvelopeGrainData", 0, EnvelopeGrainData, IGrainData)

void EnvelopeGrainData::addGrain(IGrainData* grain, float in, float out, float easeIn, float easeOut)
{
	GrainData gd;
	gd.grain = grain;
	gd.in = in;
	gd.out = out;
	gd.easeIn = easeIn;
	gd.easeOut = easeOut;
	m_grains.push_back(gd);
}

void EnvelopeGrainData::removeGrain(IGrainData* grain)
{
	T_FATAL_ERROR;
}

Ref< IGrain > EnvelopeGrainData::createInstance(resource::IResourceManager* resourceManager) const
{
	std::vector< EnvelopeGrain::Grain > grains;

	grains.resize(m_grains.size());
	for (uint32_t i = 0; i < m_grains.size(); ++i)
	{
		grains[i].grain = m_grains[i].grain->createInstance(resourceManager);
		if (!grains[i].grain)
			return 0;

		grains[i].in = m_grains[i].in;
		grains[i].out = m_grains[i].out;
		grains[i].easeIn = m_grains[i].easeIn;
		grains[i].easeOut = m_grains[i].easeOut;
	}

	return new EnvelopeGrain(grains);
}

bool EnvelopeGrainData::serialize(ISerializer& s)
{
	return s >> MemberStlVector< GrainData, MemberComposite< GrainData > >(L"grains", m_grains);
}

bool EnvelopeGrainData::GrainData::serialize(ISerializer& s)
{
	s >> MemberRef< IGrainData >(L"grain", grain);
	s >> Member< float >(L"in", in);
	s >> Member< float >(L"out", out);
	s >> Member< float >(L"easeIn", easeIn);
	s >> Member< float >(L"easeOut", easeOut);
	return true;
}

	}
}
