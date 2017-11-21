/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "Sound/Resound/BlendGrain.h"
#include "Sound/Resound/BlendGrainData.h"
#include "Sound/Resound/IGrainFactory.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.BlendGrainData", 2, BlendGrainData, IGrainData)

BlendGrainData::BlendGrainData()
:	m_response(1.0f)
{
}

Ref< IGrain > BlendGrainData::createInstance(IGrainFactory* grainFactory) const
{
	Ref< IGrain > grains[2];

	grains[0] = grainFactory->createInstance(m_grains[0]);
	if (!grains[0])
		return 0;

	grains[1] = grainFactory->createInstance(m_grains[1]);
	if (!grains[1])
		return 0;

	return new BlendGrain(
		getParameterHandle(m_id),
		m_response,
		grains[0],
		grains[1]
	);
}

void BlendGrainData::serialize(ISerializer& s)
{
	if (s.getVersion< BlendGrainData >() >= 1)
		s >> Member< std::wstring >(L"id", m_id);

	if (s.getVersion< BlendGrainData >() >= 2)
		s >> Member< float >(L"response", m_response);

	s >> MemberStaticArray< Ref< IGrainData >, 2, MemberRef< IGrainData > >(L"grains", m_grains);
}

	}
}
