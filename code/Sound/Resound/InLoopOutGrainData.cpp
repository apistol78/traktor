#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRef.h"
#include "Sound/Resound/InLoopOutGrain.h"
#include "Sound/Resound/InLoopOutGrainData.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.InLoopOutGrainData", 0, InLoopOutGrainData, IGrainData)

InLoopOutGrainData::InLoopOutGrainData()
:	m_inital(false)
{
}

Ref< IGrain > InLoopOutGrainData::createInstance(resource::IResourceManager* resourceManager) const
{
	Ref< IGrain > inGrain, inLoopGrain, outGrain, outLoopGrain;

	inGrain = m_inGrain ? m_inGrain->createInstance(resourceManager) : 0;
	inLoopGrain = m_inLoopGrain ? m_inLoopGrain->createInstance(resourceManager) : 0;
	outGrain = m_outGrain ? m_outGrain->createInstance(resourceManager) : 0;
	outLoopGrain = m_outLoopGrain ? m_outLoopGrain->createInstance(resourceManager) : 0;

	return new InLoopOutGrain(
		getParameterHandle(m_id),
		m_inital,
		inGrain,
		inLoopGrain,
		outGrain,
		outLoopGrain
	);
}

bool InLoopOutGrainData::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"id", m_id);
	s >> Member< bool >(L"inital", m_inital);
	s >> MemberRef< IGrainData >(L"inGrain", m_inGrain);
	s >> MemberRef< IGrainData >(L"inLoopGrain", m_inLoopGrain);
	s >> MemberRef< IGrainData >(L"outGrain", m_outGrain);
	s >> MemberRef< IGrainData >(L"outLoopGrain", m_outLoopGrain);
	return true;
}

	}
}
