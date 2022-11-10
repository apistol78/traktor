/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Sound/Sound.h"
#include "Sound/Tracker/Play.h"
#include "Sound/Tracker/PlayData.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.PlayData", 0, PlayData, ISerializable)

PlayData::PlayData()
:	m_note(57)	// A4
,	m_repeatFrom(0)
,	m_repeatLength(0)
{
}

PlayData::PlayData(const resource::Id< Sound >& sound, int32_t note, int32_t repeatFrom, int32_t repeatLength)
:	m_sound(sound)
,	m_note(note)
,	m_repeatFrom(repeatFrom)
,	m_repeatLength(repeatLength)
{
}

Ref< Play > PlayData::createInstance(resource::IResourceManager* resourceManager) const
{
	resource::Proxy< Sound > sound;

	if (!resourceManager->bind(m_sound, sound))
		return nullptr;

	return new Play(sound, m_note, m_repeatFrom, m_repeatLength);
}

void PlayData::serialize(ISerializer& s)
{
	s >> resource::Member< Sound >(L"sound", m_sound);
	s >> Member< int32_t >(L"note", m_note);
	s >> Member< int32_t >(L"repeatFrom", m_repeatFrom);
	s >> Member< int32_t >(L"repeatLength", m_repeatLength);
}

	}
}
