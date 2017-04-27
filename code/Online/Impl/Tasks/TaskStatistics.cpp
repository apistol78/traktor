/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Online/Result.h"
#include "Online/Impl/Tasks/TaskStatistics.h"
#include "Online/Provider/IStatisticsProvider.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.TaskStatistics", TaskStatistics, ITask)

TaskStatistics::TaskStatistics(
	IStatisticsProvider* provider,
	const std::wstring& statId,
	int32_t value,
	Result* result
)
:	m_provider(provider)
,	m_statId(statId)
,	m_value(value)
,	m_result(result)
{
}

void TaskStatistics::execute(TaskQueue* taskQueue)
{
	T_ASSERT (m_provider);
	T_ASSERT (m_result);
	if (m_provider->set(
		m_statId,
		m_value
	))
		m_result->succeed();
	else
		m_result->fail();
}

	}
}
