/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Online/AttachmentResult.h"
#include "Online/Impl/Tasks/TaskGetSaveData.h"
#include "Online/Provider/ISaveDataProvider.h"

namespace traktor::online
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.TaskGetSaveData", TaskGetSaveData, ITask)

TaskGetSaveData::TaskGetSaveData(
	ISaveDataProvider* provider,
	const std::wstring& saveDataId,
	AttachmentResult* result
)
:	m_provider(provider)
,	m_saveDataId(saveDataId)
,	m_result(result)
{
}

void TaskGetSaveData::execute(TaskQueue* taskQueue)
{
	T_ASSERT(m_provider);
	T_ASSERT(m_result);
	Ref< ISerializable > attachment;
	if (m_provider->get(
		m_saveDataId,
		attachment
	))
		m_result->succeed(attachment);
	else
		m_result->fail();
}

}
