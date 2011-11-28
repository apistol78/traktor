#ifndef traktor_online_TaskStatistics_H
#define traktor_online_TaskStatistics_H

#include "Online/Impl/ITask.h"

namespace traktor
{
	namespace online
	{

class IStatisticsProvider;
class Result;

class TaskStatistics : public ITask
{
	T_RTTI_CLASS;

public:
	TaskStatistics(
		IStatisticsProvider* provider,
		const std::wstring& statId,
		float value,
		Result* result
	);

	virtual void execute(TaskQueue* taskQueue);

private:
	Ref< IStatisticsProvider > m_provider;
	std::wstring m_statId;
	float m_value;
	Ref< Result > m_result;
};

	}
}

#endif	// traktor_online_TaskStatistics_H
