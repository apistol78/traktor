/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

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
	virtual bool ready() const override final;

	virtual bool enumerate(std::set< std::wstring >& outSaveDataIds) const override final;

	virtual Ref< AttachmentResult > get(const std::wstring& saveDataId) const override final;

	virtual Ref< ISerializable > getNow(const std::wstring& saveDataId) const override final;

	virtual Ref< Result > set(const std::wstring& saveDataId, const SaveDataDesc& saveDataDesc, const ISerializable* attachment, bool replace) override final;

	virtual bool setNow(const std::wstring& saveDataId, const SaveDataDesc& saveDataDesc, const ISerializable* attachment, bool replace) override final;

	virtual Ref< Result > remove(const std::wstring& saveDataId) override final;

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

