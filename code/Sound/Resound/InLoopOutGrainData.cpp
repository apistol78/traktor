/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRef.h"
#include "Sound/Resound/InLoopOutGrain.h"
#include "Sound/Resound/InLoopOutGrainData.h"
#include "Sound/Resound/IGrainFactory.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.InLoopOutGrainData", 0, InLoopOutGrainData, IGrainData)

InLoopOutGrainData::InLoopOutGrainData()
:	m_inital(false)
{
}

Ref< IGrain > InLoopOutGrainData::createInstance(IGrainFactory* grainFactory) const
{
	Ref< IGrain > inGrain, inLoopGrain, outGrain, outLoopGrain;

	inGrain = grainFactory->createInstance(m_inGrain);
	inLoopGrain = grainFactory->createInstance(m_inLoopGrain);
	outGrain = grainFactory->createInstance(m_outGrain);
	outLoopGrain = grainFactory->createInstance(m_outLoopGrain);

	return new InLoopOutGrain(
		getParameterHandle(m_id),
		m_inital,
		inGrain,
		inLoopGrain,
		outGrain,
		outLoopGrain
	);
}

void InLoopOutGrainData::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"id", m_id);
	s >> Member< bool >(L"inital", m_inital);
	s >> MemberRef< IGrainData >(L"inGrain", m_inGrain);
	s >> MemberRef< IGrainData >(L"inLoopGrain", m_inLoopGrain);
	s >> MemberRef< IGrainData >(L"outGrain", m_outGrain);
	s >> MemberRef< IGrainData >(L"outLoopGrain", m_outLoopGrain);
}

	}
}
