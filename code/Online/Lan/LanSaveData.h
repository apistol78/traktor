/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_online_LanSaveData_H
#define traktor_online_LanSaveData_H

#include "Online/Provider/ISaveDataProvider.h"

namespace traktor
{
	namespace online
	{

class LanSaveData : public ISaveDataProvider
{
	T_RTTI_CLASS;

public:
	LanSaveData();

	virtual bool enumerate(std::set< std::wstring >& outSaveDataIds) override final;

	virtual bool get(const std::wstring& saveDataId, Ref< ISerializable >& outAttachment) override final;

	virtual bool set(const std::wstring& saveDataId, const SaveDataDesc& saveDataDesc, const ISerializable* attachment, bool replace) override final;

	virtual bool remove(const std::wstring& saveDataId) override final;
};

	}
}

#endif	// traktor_online_LanSaveData_H
