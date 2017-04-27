/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_online_GcSaveData_H
#define traktor_online_GcSaveData_H

#include "Online/Provider/ISaveDataProvider.h"

namespace traktor
{
	namespace online
	{

class GcSaveData : public ISaveDataProvider
{
	T_RTTI_CLASS;

public:
	GcSaveData();

	virtual bool enumerate(std::set< std::wstring >& outSaveDataIds) T_OVERRIDE T_FINAL;

	virtual bool get(const std::wstring& saveDataId, Ref< ISerializable >& outAttachment) T_OVERRIDE T_FINAL;

	virtual bool set(const std::wstring& saveDataId, const SaveDataDesc& saveDataDesc, const ISerializable* attachment, bool replace) T_OVERRIDE T_FINAL;

	virtual bool remove(const std::wstring& saveDataId) T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_online_GcSaveData_H
