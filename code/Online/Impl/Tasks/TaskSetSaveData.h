#ifndef traktor_online_TaskSetSaveData_H
#define traktor_online_TaskSetSaveData_H

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
		const ISerializable* attachment,
		bool replace,
		Result* result
	);

	virtual void execute();

private:
	Ref< ISaveDataProvider > m_provider;
	std::wstring m_saveDataId;
	Ref< const ISerializable > m_attachment;
	bool m_replace;
	Ref< Result > m_result;
};

	}
}

#endif	// traktor_online_TaskSetSaveData_H
