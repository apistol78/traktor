/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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
	virtual bool ready() const T_OVERRIDE T_FINAL;

	virtual bool enumerate(std::set< std::wstring >& outSaveDataIds) const T_OVERRIDE T_FINAL;

	virtual Ref< AttachmentResult > get(const std::wstring& saveDataId) const T_OVERRIDE T_FINAL;

	virtual Ref< ISerializable > getNow(const std::wstring& saveDataId) const T_OVERRIDE T_FINAL;

	virtual Ref< Result > set(const std::wstring& saveDataId, const SaveDataDesc& saveDataDesc, const ISerializable* attachment, bool replace) T_OVERRIDE T_FINAL;

	virtual bool setNow(const std::wstring& saveDataId, const SaveDataDesc& saveDataDesc, const ISerializable* attachment, bool replace) T_OVERRIDE T_FINAL;

	virtual Ref< Result > remove(const std::wstring& saveDataId) T_OVERRIDE T_FINAL;

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
