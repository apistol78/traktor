/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Core/Thread/Acquire.h"
#include "Online/Impl/Statistics.h"
#include "Online/Impl/TaskQueue.h"
#include "Online/Impl/Tasks/TaskEnumStatistics.h"
#include "Online/Impl/Tasks/TaskStatistics.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.Statistics", Statistics, IStatistics)

bool Statistics::ready() const
{
	return m_ready;
}

bool Statistics::enumerate(std::set< std::wstring >& outStatIds) const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	for (std::map< std::wstring, int32_t >::const_iterator i = m_statistics.begin(); i != m_statistics.end(); ++i)
		outStatIds.insert(i->first);
	return true;
}

bool Statistics::get(const std::wstring& statId, int32_t& outValue) const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	std::map< std::wstring, int32_t >::const_iterator i = m_statistics.find(statId);
	if (i == m_statistics.end())
	{
		log::warning << L"Statistics error; No such statistic provided, \"" << statId << L"\"" << Endl;
		return false;
	}

	outValue = i->second;
	return true;
}

Ref< Result > Statistics::set(const std::wstring& statId, int32_t value)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	std::map< std::wstring, int32_t >::iterator i = m_statistics.find(statId);
	if (i == m_statistics.end())
	{
		log::warning << L"Statistics error; No such statistic provided, \"" << statId << L"\"" << Endl;
		return 0;
	}

	if (i->second == value)
		return new Result(true);

	Ref< Result > result = new Result();
	if (m_taskQueue->add(new TaskStatistics(
		m_provider,
		statId,
		value,
		result
	)))
	{
		i->second = value;
		return result;
	}
	else
		return 0;
}

Ref< Result > Statistics::add(const std::wstring& statId, int32_t valueDelta)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	std::map< std::wstring, int32_t >::iterator i = m_statistics.find(statId);
	if (i == m_statistics.end())
	{
		log::warning << L"Statistics error; No such statistic provided, \"" << statId << L"\"" << Endl;
		return 0;
	}

	Ref< Result > result = new Result();
	if (m_taskQueue->add(new TaskStatistics(
		m_provider,
		statId,
		i->second + valueDelta,
		result
	)))
	{
		i->second += valueDelta;
		return result;
	}
	else
		return 0;
}

Statistics::Statistics(IStatisticsProvider* provider, TaskQueue* taskQueue)
:	m_provider(provider)
,	m_taskQueue(taskQueue)
,	m_ready(false)
{
}

void Statistics::enqueueEnumeration()
{
	m_taskQueue->add(new TaskEnumStatistics(
		m_provider,
		this,
		(TaskEnumStatistics::sink_method_t)&Statistics::callbackEnumStatistics
	));
}

void Statistics::callbackEnumStatistics(const std::map< std::wstring, int32_t >& statistics)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_statistics = statistics;
	m_ready = true;
}

	}
}
