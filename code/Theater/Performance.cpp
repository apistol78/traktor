/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Theater/Performance.h"

#include "Core/Containers/AlignedVector.h"
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "Theater/Act.h"
#include "Theater/ActData.h"

#include <algorithm>
#include <random>

namespace traktor::theater
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.theater.Performance", Performance, Object)

Ref< Performance > Performance::create(const RefArray< ActData >& actsData, bool randomizeActs, const world::IEntityBuilder* entityBuilder)
{
	float time = 0.0f;

	// Create act instances.
	RefArray< const Act > acts(actsData.size());
	for (size_t i = 0; i < actsData.size(); ++i)
	{
		const float duration = actsData[i]->getDuration();
		if (duration < 0.0f)
			return nullptr;

		acts[i] = actsData[i]->createInstance(time, time + duration, entityBuilder);
		if (!acts[i])
			return nullptr;

		time += duration;
	}

	if (randomizeActs)
	{
		AlignedVector< size_t > indices(acts.size());
		for (size_t i = 0; i < indices.size(); ++i)
			indices[i] = i;

		std::random_device rd;
		std::mt19937 g(rd());
		std::shuffle(indices.begin(), indices.end(), g);

		RefArray< const Act > tmp(acts.size());
		for (size_t i = 0; i < indices.size(); ++i)
			tmp[i] = acts[indices[i]];

		acts = tmp;
	}

	return new Performance(acts, time);
}

Performance::Performance(const RefArray< const Act >& acts, double totalDuration)
:	m_acts(acts)
,	m_totalDuration(totalDuration)
{
}

bool Performance::play(const std::wstring& actName)
{
	auto it = std::find_if(m_acts.begin(), m_acts.end(), [&](const Act* act) {
		return actName == act->getName();
	});
	if (it == m_acts.end())
	{
		log::warning << L"No act \"" << actName << L"\" found." << Endl;
		return false;
	}

	if (*it != m_act)
	{
		m_act = *it;
		m_timeLast = -1.0;
	}

	m_timeStart = -1.0;
	m_raiseEvents = true;
	return true;
}

void Performance::stop()
{
	m_act = nullptr;
}

bool Performance::preview(const std::wstring& actName)
{
	if (!play(actName))
		return false;

	// Pin start of performance at zero in order to evaluate acts at
	// absolute time. Do not issue any events as previewing is scrubbing
	// back and forth in time.
	m_timeStart = 0.0;
	m_raiseEvents = false;
	return true;
}

void Performance::update(const IEntityResolver& resolver, world::EventManagerComponent* eventManager, world::Entity* eventSender, const Transform& base, double totalTime, double deltaTime)
{
	if (m_timeStart < 0.0)
		m_timeStart = totalTime;

	if (m_act != nullptr)
	{
		const double timeAct = totalTime - m_timeStart;
		const double duration = m_act->getEnd() - m_act->getStart();
		if (timeAct < -FUZZY_EPSILON || timeAct > duration + FUZZY_EPSILON)
			m_act = nullptr;
	}

	if (m_act == nullptr)
		return;

	// Do nothing if no time has passed since last update.
	if (traktor::abs(totalTime - m_timeLast) <= FUZZY_EPSILON)
		return;

	// Evaluate current act; time of previous evaluation is negative if act
	// hasn't been evaluated yet so events at zero are issued.
	const double timePrevious = (m_timeLast >= 0.0) ? m_timeLast - m_timeStart : -1.0;

	if (!m_act->update(
			resolver,
			m_raiseEvents ? eventManager : nullptr,
			eventSender,
			base,
			(float)timePrevious,
			(float)(totalTime - m_timeStart),
			(float)deltaTime
		))
		m_act = nullptr;

	m_timeLast = totalTime;
}

}
