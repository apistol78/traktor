#pragma once

#include "Online/Impl/ITask.h"

namespace traktor
{

class Result;

	namespace online
	{

class ILeaderboardsProvider;

class TaskLeaderboard : public ITask
{
	T_RTTI_CLASS;

public:
	TaskLeaderboard(
		ILeaderboardsProvider* provider,
		uint64_t handle,
		int32_t score,
		Result* result
	);

	virtual void execute(TaskQueue* taskQueue) override final;

private:
	Ref< ILeaderboardsProvider > m_provider;
	uint64_t m_handle;
	int32_t m_score;
	Ref< Result > m_result;
};

	}
}

