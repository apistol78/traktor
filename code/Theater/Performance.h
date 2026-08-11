/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/RefArray.h"

#include <string>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_THEATER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Transform;

}

namespace traktor::world
{

class Entity;
class EventManagerComponent;
class IEntityBuilder;

}

namespace traktor::theater
{

class Act;
class ActData;
class IEntityResolver;

/*! Performance of a sequence of acts.
 * \ingroup Theater
 *
 * A performance own the playback state of a set of acts; it is agnostic
 * to which entities it animate as those are resolved through an entity
 * resolver, and in which space it animate as all acts are evaluated
 * relative a given base transform.
 */
class T_DLLCLASS Performance : public Object
{
	T_RTTI_CLASS;

public:
	/*! Create a performance from act descriptions.
	 *
	 * \param actsData Act descriptions.
	 * \param randomizeActs Play acts in random order.
	 * \param entityBuilder Builder used to instantiate track events; if null then no events are instantiated.
	 * \return Performance instance, null if acts are invalid.
	 */
	static Ref< Performance > create(const RefArray< ActData >& actsData, bool randomizeActs, const world::IEntityBuilder* entityBuilder);

	/*! Start playing named act. */
	bool play(const std::wstring& actName);

	/*! Stop playing. */
	void stop();

	/*! Select named act and evaluate it at absolute time.
	 *
	 * Used by the editor in order to scrub through an act; the act is
	 * evaluated at the time given to \a update instead of relative
	 * the time at which playback did start. Events are never issued
	 * while previewing.
	 */
	bool preview(const std::wstring& actName);

	bool isPlaying() const { return m_act != nullptr; }

	/*! Evaluate current act.
	 *
	 * \param resolver Resolver of entities referenced from tracks.
	 * \param eventManager Manager through which track events are issued; if null then no events are issued.
	 * \param eventSender Entity issuing track events; if null then each track's own entity is the sender.
	 * \param base Transform of which all tracks are relative.
	 * \param totalTime Total time.
	 * \param deltaTime Delta time since last update.
	 */
	void update(const IEntityResolver& resolver, world::EventManagerComponent* eventManager, world::Entity* eventSender, const Transform& base, double totalTime, double deltaTime);

	double getTotalDuration() const { return m_totalDuration; }

private:
	RefArray< const Act > m_acts;
	double m_totalDuration = 0.0;
	const Act* m_act = nullptr;
	double m_timeStart = -1.0;
	double m_timeLast = -1.0;
	bool m_raiseEvents = false;

	explicit Performance(const RefArray< const Act >& acts, double totalDuration);
};

}
