/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_online_TaskRemoveSaveData_H
#define traktor_online_TaskRemoveSaveData_H

#include "Online/Types.h"
#include "Online/Impl/ITask.h"

namespace traktor
{
	namespace online
	{

class ISaveDataProvider;
class Result;

class TaskRemoveSaveData : public ITask
{
	T_RTTI_CLASS;

public:
	TaskRemoveSaveData(
		ISaveDataProvider* provider,
		const std::wstring& saveDataId,
		Result* result
	);

	virtual void execute(TaskQueue* taskQueue) T_OVERRIDE T_FINAL;

private:
	Ref< ISaveDataProvider > m_provider;
	std::wstring m_saveDataId;
	Ref< Result > m_result;
};

	}
}

#endif	// traktor_online_TaskRemoveSaveData_H
