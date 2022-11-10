/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Sound/Tracker/Pattern.h"
#include "Sound/Tracker/PatternData.h"
#include "Sound/Tracker/Track.h"
#include "Sound/Tracker/TrackData.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.PatternData", 0, PatternData, ISerializable)

PatternData::PatternData()
:	m_duration(0)
{
}

PatternData::PatternData(int32_t duration)
:	m_duration(duration)
{
}

Ref< Pattern > PatternData::createInstance(resource::IResourceManager* resourceManager) const
{
	RefArray< Track > tracks;
	for (auto trackData : m_tracks)
	{
		Ref< Track > track = trackData->createInstance(resourceManager);
		if (!track)
			return nullptr;

		tracks.push_back(track);
	}
	return new Pattern(m_duration, tracks);
}

void PatternData::addTrack(const TrackData* track)
{
	m_tracks.push_back(track);
}

void PatternData::serialize(ISerializer& s)
{
	s >> Member< int32_t >(L"duration", m_duration);
	s >> MemberRefArray< const TrackData >(L"tracks", m_tracks);
}

	}
}
