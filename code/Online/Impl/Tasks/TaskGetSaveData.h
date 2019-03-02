#pragma once

#include <string>
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

	virtual void execute(TaskQueue* taskQueue) override final;

private:
	Ref< ISaveDataProvider > m_provider;
	std::wstring m_saveDataId;
	Ref< AttachmentResult > m_result;
};

	}
}

