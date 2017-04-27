/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_online_ISaveData_H
#define traktor_online_ISaveData_H

#include "Online/AttachmentResult.h"
#include "Online/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{

class ISerializable;

	namespace online
	{

class T_DLLCLASS ISaveData : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool ready() const = 0;

	virtual bool enumerate(std::set< std::wstring >& outSaveDataIds) const = 0;

	virtual Ref< AttachmentResult > get(const std::wstring& saveDataId) const = 0;

	virtual Ref< ISerializable > getNow(const std::wstring& saveDataId) const = 0;

	virtual Ref< Result > set(const std::wstring& saveDataId, const SaveDataDesc& saveDataDesc, const ISerializable* attachment, bool replace) = 0;

	virtual bool setNow(const std::wstring& saveDataId, const SaveDataDesc& saveDataDesc, const ISerializable* attachment, bool replace) = 0;

	virtual Ref< Result > remove(const std::wstring& saveDataId) = 0;
};

	}
}

#endif	// traktor_online_ISaveData_H
