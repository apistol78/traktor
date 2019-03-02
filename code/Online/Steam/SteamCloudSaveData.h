#pragma once

#include "Online/Provider/ISaveDataProvider.h"

namespace traktor
{
	namespace online
	{

class SteamCloudSaveData : public ISaveDataProvider
{
	T_RTTI_CLASS;

public:
	virtual bool enumerate(std::set< std::wstring >& outSaveDataIds) override final;

	virtual bool get(const std::wstring& saveDataId, Ref< ISerializable >& outAttachment) override final;

	virtual bool set(const std::wstring& saveDataId, const SaveDataDesc& saveDataDesc, const ISerializable* attachment, bool replace) override final;

	virtual bool remove(const std::wstring& saveDataId) override final;
};

	}
}

