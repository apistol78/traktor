/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Thread/Acquire.h"
#include "Ui/Custom/BackgroundWorkerStatus.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

BackgroundWorkerStatus::BackgroundWorkerStatus()
:	m_steps(0)
,	m_step(-1)
{
}

BackgroundWorkerStatus::BackgroundWorkerStatus(int32_t steps)
:	m_steps(steps)
,	m_step(-1)
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
	}
}
