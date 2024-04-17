/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Const.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Theater/Act.h"
#include "Theater/ActData.h"
#include "Theater/Track.h"
#include "Theater/TrackData.h"

namespace traktor::theater
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.theater.ActData", 0, ActData, ISerializable)

Ref< Act > ActData::createInstance(float start, float end) const
{
	RefArray< const Track > tracks(m_tracks.size());
	for (size_t i = 0; i < m_tracks.size(); ++i)
	{
		tracks[i] = new Track(
			m_tracks[i]->getEntityId(),
			m_tracks[i]->getLookAtEntityId(),
			m_tracks[i]->getPath()
		);
	}
	return new Act(m_name, start, end, tracks);
}

void ActData::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", m_name);
	s >> Member< float >(L"duration", m_duration);
	s >> MemberRefArray< TrackData >(L"tracks", m_tracks);
}

}
