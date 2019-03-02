#pragma once

#include <string>
#include "Online/Impl/ITask.h"

namespace traktor
{

class Result;

	namespace online
	{

class IAchievementsProvider;

class TaskAchievement : public ITask
{
	T_RTTI_CLASS;

public:
	TaskAchievement(
		IAchievementsProvider* provider,
		const std::wstring& achievementId,
		bool reward,
		Result* result
	);

	virtual void execute(TaskQueue* taskQueue) override final;

private:
	Ref< IAchievementsProvider > m_provider;
	std::wstring m_achievementId;
	bool m_reward;
	Ref< Result > m_result;
};

	}
}

