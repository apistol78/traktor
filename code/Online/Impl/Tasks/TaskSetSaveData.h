#ifndef traktor_online_TaskSetSaveData_H
#define traktor_online_TaskSetSaveData_H

#include "Online/Types.h"
#include "Online/Impl/ITask.h"

namespace traktor
{
	namespace online
	{

class ISaveDataProvider;
class Result;

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

	virtual void execute(TaskQueue* taskQueue) T_OVERRIDE T_FINAL;

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

#endif	// traktor_online_TaskSetSaveData_H
