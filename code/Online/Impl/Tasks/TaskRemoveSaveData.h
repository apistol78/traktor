#pragma once

#include "Online/Types.h"
#include "Online/Impl/ITask.h"

namespace traktor
{

class Result;

	namespace online
	{

class ISaveDataProvider;

class TaskRemoveSaveData : public ITask
{
	T_RTTI_CLASS;

public:
	TaskRemoveSaveData(
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
}

