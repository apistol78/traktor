#ifndef traktor_online_TaskLeaderboard_H
#define traktor_online_TaskLeaderboard_H

#include "Online/Impl/ITask.h"

namespace traktor
{
	namespace online
	{

class ILeaderboardsProvider;
class Result;

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

	virtual void execute(TaskQueue* taskQueue) T_OVERRIDE T_FINAL;

private:
	Ref< ILeaderboardsProvider > m_provider;
	uint64_t m_handle;
	int32_t m_score;
	Ref< Result > m_result;
};

	}
}

#endif	// traktor_online_TaskLeaderboard_H
