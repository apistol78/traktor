/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Online/Types.h"
#include "Online/Impl/ITask.h"

namespace traktor
{

class Result;

}

namespace traktor::online
{

class ISaveDataProvider;

class TaskRemoveSaveData : public ITask
{
	T_RTTI_CLASS;

public:
	explicit TaskRemoveSaveData(
		ISaveDataProvider* provider,
		const std::wstring& saveDataId,
		Result* result
	);

	virtual void execute(TaskQueue* taskQueue) override final;

private:
	Ref< ISaveDataProvider > m_provider;
	std::wstring m_saveDataId;
	Ref< Result > m_result;
};

}
