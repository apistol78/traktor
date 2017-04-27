/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Resource/Member.h"
#include "Sound/Sound.h"
#include "Spray/SoundEventData.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.spray.SoundEventData", 1, SoundEventData, world::IEntityEventData)

SoundEventData::SoundEventData()
:	m_positional(true)
,	m_follow(true)
,	m_autoStopFar(true)
{
}

void SoundEventData::serialize(ISerializer& s)
{
	s >> resource::Member< sound::Sound >(L"sound", m_sound);
	s >> Member< bool >(L"positional", m_positional);
	s >> Member< bool >(L"follow", m_follow);

	if (s.getVersion() >= 1)
		s >> Member< bool >(L"autoStopFar", m_autoStopFar);
}

	}
}
