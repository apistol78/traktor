/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_online_TaskAchievement_H
#define traktor_online_TaskAchievement_H

#include <string>
#include "Online/Impl/ITask.h"

namespace traktor
{
	namespace online
	{

class IAchievementsProvider;
class Result;

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

	virtual void execute(TaskQueue* taskQueue) T_OVERRIDE T_FINAL;

private:
	Ref< IAchievementsProvider > m_provider;
	std::wstring m_achievementId;
	bool m_reward;
	Ref< Result > m_result;
};

	}
}

#endif	// traktor_online_TaskAchievement_H
