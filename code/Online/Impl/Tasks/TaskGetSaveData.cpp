/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Online/AttachmentResult.h"
#include "Online/Impl/Tasks/TaskGetSaveData.h"
#include "Online/Provider/ISaveDataProvider.h"

namespace traktor
{
	namespace online
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
	T_ASSERT (m_provider);
	T_ASSERT (m_result);
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
}
