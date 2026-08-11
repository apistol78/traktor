/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/RefArray.h"

#include <string>

namespace traktor
{

class Transform;

}

namespace traktor::world
{

class Entity;
class EventManagerComponent;

}

namespace traktor::theater
{

class IEntityResolver;
class Track;

/*! Act
 * \ingroup Theater
 */
class Act : public Object
{
	T_RTTI_CLASS;

public:
	explicit Act(const std::wstring& name, float start, float end, const RefArray< const Track >& tracks);

	/*! Evaluate all tracks of this act.
	 *
	 * \param resolver Resolver of entities referenced from tracks.
	 * \param eventManager Manager through which track events are issued; if null then no events are issued.
	 * \param eventSender Entity issuing track events, offset by the track's transform; if null then each track's own entity is the sender.
	 * \param base Transform of which all tracks are relative.
	 * \param timePrevious Time of previous evaluation; events in (timePrevious, time] are issued. Negative if this is the first evaluation of the act.
	 * \param time Time within this act.
	 * \param deltaTime Delta time since last update.
	 * \return True if act is still valid at given time.
	 */
	bool update(const IEntityResolver& resolver, world::EventManagerComponent* eventManager, world::Entity* eventSender, const Transform& base, float timePrevious, float time, float deltaTime) const;

	const std::wstring& getName() const { return m_name; }

	float getStart() const { return m_start; }

	float getEnd() const { return m_end; }

private:
	std::wstring m_name;
	float m_start;
	float m_end;
	RefArray< const Track > m_tracks;
};

}
