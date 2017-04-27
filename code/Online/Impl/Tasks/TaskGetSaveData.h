/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_online_TaskGetSaveData_H
#define traktor_online_TaskGetSaveData_H

#include "Online/Impl/ITask.h"

namespace traktor
{
	namespace online
	{

class ISaveDataProvider;
class AttachmentResult;

class TaskGetSaveData : public ITask
{
	T_RTTI_CLASS;

public:
	TaskGetSaveData(
		ISaveDataProvider* provider,
		const std::wstring& saveDataId,
		AttachmentResult* result
	);

	virtual void execute(TaskQueue* taskQueue) T_OVERRIDE T_FINAL;

private:
	Ref< ISaveDataProvider > m_provider;
	std::wstring m_saveDataId;
	Ref< AttachmentResult > m_result;
};

	}
}

#endif	// traktor_online_TaskGetSaveData_H
