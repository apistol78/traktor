/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Theater/ActData.h"

#include "Core/Math/Const.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Theater/Act.h"
#include "Theater/Track.h"
#include "Theater/TrackData.h"
#include "World/IEntityBuilder.h"

#include <algorithm>

namespace traktor::theater
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.theater.ActData", 0, ActData, ISerializable)

Ref< Act > ActData::createInstance(float start, float end, const world::IEntityBuilder* entityBuilder) const
{
	RefArray< const Track > tracks(m_tracks.size());
	for (size_t i = 0; i < m_tracks.size(); ++i)
	{
		// Instantiate events of track; require a builder as events are
		// created through the entity factories.
		AlignedVector< Track::EventKey > events;
		if (entityBuilder != nullptr)
		{
			for (const auto& eventKey : m_tracks[i]->getEvents())
			{
				if (!eventKey.event)
					continue;

				Ref< world::IEntityEvent > event = entityBuilder->create(eventKey.event);
				if (event)
					events.push_back({ eventKey.T, event });
			}

			// Keys are evaluated in order of time.
			std::sort(events.begin(), events.end(), [](const Track::EventKey& lh, const Track::EventKey& rh) {
				return lh.T < rh.T;
			});
		}

		tracks[i] = new Track(
			m_tracks[i]->getEntityId(),
			m_tracks[i]->getLookAtEntityId(),
			m_tracks[i]->getPath(),
			events
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
