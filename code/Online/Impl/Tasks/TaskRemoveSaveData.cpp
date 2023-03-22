/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Online/AttachmentResult.h"
#include "Online/Impl/Tasks/TaskRemoveSaveData.h"
#include "Online/Provider/ISaveDataProvider.h"

namespace traktor::online
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.TaskRemoveSaveData", TaskRemoveSaveData, ITask)

TaskRemoveSaveData::TaskRemoveSaveData(
	ISaveDataProvider* provider,
	const std::wstring& saveDataId,
	Result* result
)
:	m_provider(provider)
,	m_saveDataId(saveDataId)
,	m_result(result)
{
}

void TaskRemoveSaveData::execute(TaskQueue* taskQueue)
{
	T_ASSERT(m_provider);
	T_ASSERT(m_result);
	if (m_provider->remove(m_saveDataId))
		m_result->succeed();
	else
		m_result->fail();
}

}
