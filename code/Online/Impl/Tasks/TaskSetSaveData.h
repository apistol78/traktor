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

	namespace online
	{

class ISaveDataProvider;

class TaskSetSaveData : public ITask
{
	T_RTTI_CLASS;

public:
	TaskSetSaveData(
		ISaveDataProvider* provider,
		const std::wstring& saveDataId,
		const SaveDataDesc& saveDataDesc,
		const ISerializable* attachment,
		bool replace,
		Result* result
	);

	virtual void execute(TaskQueue* taskQueue) override final;

private:
	Ref< ISaveDataProvider > m_provider;
	std::wstring m_saveDataId;
	SaveDataDesc m_saveDataDesc;
	Ref< const ISerializable > m_attachment;
	bool m_replace;
	Ref< Result > m_result;
};

	}
}

