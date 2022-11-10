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

	virtual void execute(TaskQueue* taskQueue) override final;

private:
	Ref< IStatisticsProvider > m_provider;
	std::wstring m_statId;
	int32_t m_value;
	Ref< Result > m_result;
};

	}
}

