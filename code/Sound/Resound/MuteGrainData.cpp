/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Math/Random.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComposite.h"
#include "Sound/Resound/MuteGrain.h"
#include "Sound/Resound/MuteGrainData.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

Random s_random;

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.MuteGrainData", 1, MuteGrainData, IGrainData)

MuteGrainData::MuteGrainData()
:	m_duration(0.0f, 0.0f)
{
}

Ref< IGrain > MuteGrainData::createInstance(IGrainFactory* grainFactory) const
{
	return new MuteGrain(m_duration.random(s_random));
}

void MuteGrainData::serialize(ISerializer& s)
{
	if (s.getVersion() >= 1)
		s >> MemberComposite< Range< float > >(L"duration", m_duration);
	else
	{
		double duration;
		s >> Member< double >(L"duration", duration);
		m_duration = Range< float >(float(duration), float(duration));
	}
}

	}
}
