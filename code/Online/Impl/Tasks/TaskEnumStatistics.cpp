/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Online/Impl/Tasks/TaskEnumStatistics.h"
#include "Online/Provider/IStatisticsProvider.h"

namespace traktor::online
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.TaskEnumStatistics", TaskEnumStatistics, ITask)

TaskEnumStatistics::TaskEnumStatistics(
	IStatisticsProvider* provider,
	Object* sinkObject,
	sink_method_t sinkMethod
)
:	m_provider(provider)
,	m_sinkObject(sinkObject)
,	m_sinkMethod(sinkMethod)
{
}

void TaskEnumStatistics::execute(TaskQueue* taskQueue)
{
	T_ASSERT(m_provider);
	T_DEBUG(L"Online; Begin enumerating statistics");
	std::map< std::wstring, int32_t > statistics;
	m_provider->enumerate(statistics);
	(m_sinkObject->*m_sinkMethod)(statistics);
	T_DEBUG(L"Online; Finished enumerating statistics");
}

}
