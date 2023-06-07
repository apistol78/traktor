/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "Theater/Act.h"
#include "Theater/TheaterController.h"

namespace traktor::theater
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.theater.TheaterController", TheaterController, scene::ISceneController)

TheaterController::TheaterController(const RefArray< const Act >& acts, float totalDuration)
:	m_acts(acts)
,	m_totalDuration(totalDuration)
{
}

bool TheaterController::play(const std::wstring& actName)
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

void TheaterController::update(scene::Scene* scene, double time, double deltaTime)
{
	if (m_timeStart < 0.0f)
		m_timeStart = time;

	if (m_act != nullptr)
	{
		double timeAct = time - m_timeStart;
		double duration = m_act->getEnd() - m_act->getStart();
		if (timeAct < -FUZZY_EPSILON || timeAct > duration + FUZZY_EPSILON)
			m_act = nullptr;
	}

	if (m_act == nullptr)
		return;

	// Do nothing if no time has passed since last update.
	if (traktor::abs(time - m_timeLast) <= FUZZY_EPSILON)
		return;

	// Evaluate current act.
	m_act->update(scene, time - m_timeStart, deltaTime);

	m_timeLast = time;
}

}
