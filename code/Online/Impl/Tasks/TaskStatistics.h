/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_online_TaskStatistics_H
#define traktor_online_TaskStatistics_H

#include <string>
#include "Online/Impl/ITask.h"

namespace traktor
{

class Result;

	namespace online
	{

class IStatisticsProvider;

class TaskStatistics : public ITask
{
	T_RTTI_CLASS;

public:
	TaskStatistics(
		IStatisticsProvider* provider,
		const std::wstring& statId,
		int32_t value,
		Result* result
	);

	virtual void execute(TaskQueue* taskQueue) T_OVERRIDE T_FINAL;

private:
	Ref< IStatisticsProvider > m_provider;
	std::wstring m_statId;
	int32_t m_value;
	Ref< Result > m_result;
};

	}
}

#endif	// traktor_online_TaskStatistics_H
