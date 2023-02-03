/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Thread/Acquire.h"
#include "Ui/BackgroundWorkerStatus.h"

namespace traktor::ui
{

BackgroundWorkerStatus::BackgroundWorkerStatus(int32_t steps)
:	m_steps(steps)
{
}

void BackgroundWorkerStatus::setSteps(int32_t steps)
{
	m_steps = steps;
}

void BackgroundWorkerStatus::notify(int32_t step, const std::wstring& status)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_step = step;
	m_status = status;
}

bool BackgroundWorkerStatus::read(int32_t& outStep, std::wstring& outStatus)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	if (m_step < 0)
		return false;

	outStep = int32_t(1000 * m_step) / m_steps;
	outStatus = m_status;

	m_step = -1;
	m_status.clear();

	return true;
}

}
