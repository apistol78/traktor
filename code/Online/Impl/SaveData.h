#ifndef traktor_online_SaveData_H
#define traktor_online_SaveData_H

#include "Core/Thread/Semaphore.h"
#include "Online/ISaveData.h"

namespace traktor
{
	namespace online
	{

class ISaveDataProvider;
class TaskQueue;

class SaveData : public ISaveData
{
	T_RTTI_CLASS;

public:
	virtual bool ready() const;

	virtual bool enumerate(std::set< std::wstring >& outSaveDataIds) const;

	virtual Ref< AttachmentResult > get(const std::wstring& saveDataId) const;

	virtual Ref< ISerializable > getNow(const std::wstring& saveDataId) const;

	virtual Ref< Result > set(const std::wstring& saveDataId, const SaveDataDesc& saveDataDesc, const ISerializable* attachment, bool replace);

	virtual bool setNow(const std::wstring& saveDataId, const SaveDataDesc& saveDataDesc, const ISerializable* attachment, bool replace);

	virtual Ref< Result > remove(const std::wstring& saveDataId);

private:
	friend class SessionManager;

	Ref< ISaveDataProvider > m_provider;
	Ref< TaskQueue > m_taskQueue;
	mutable Semaphore m_lock;
	std::set< std::wstring > m_saveDataIds;
	bool m_ready;

	SaveData(ISaveDataProvider* provider, TaskQueue* taskQueue);

	void enqueueEnumeration();

	void callbackEnumSaveData(const std::set< std::wstring >& saveDataIds);
};

	}
}

#endif	// traktor_online_SaveData_H
