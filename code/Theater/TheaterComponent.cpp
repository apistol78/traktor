/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "Theater/Act.h"
#include "Theater/TheaterComponent.h"

namespace traktor::theater
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.theater.TheaterComponent", TheaterComponent, world::IWorldComponent)

TheaterComponent::TheaterComponent(const RefArray< const Act >& acts, float totalDuration)
:	m_acts(acts)
,	m_totalDuration(totalDuration)
{
}

void TheaterComponent::destroy()
{
}

void TheaterComponent::update(world::World* world, const world::UpdateParams& update)
{
	if (m_timeStart < 0.0)
		m_timeStart = update.totalTime;

	if (m_act != nullptr)
	{
		const double timeAct = update.totalTime - m_timeStart;
		const double duration = m_act->getEnd() - m_act->getStart();
		if (timeAct < -FUZZY_EPSILON || timeAct > duration + FUZZY_EPSILON)
			m_act = nullptr;
	}

	if (m_act == nullptr)
		return;

	// Do nothing if no time has passed since last update.
	if (traktor::abs(update.totalTime - m_timeLast) <= FUZZY_EPSILON)
		return;

	// Evaluate current act.
	m_act->update(world, update.totalTime - m_timeStart, update.deltaTime);

	m_timeLast = update.totalTime;
}

bool TheaterComponent::play(const std::wstring& actName)
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
	return true;
}

}
