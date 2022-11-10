/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
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

