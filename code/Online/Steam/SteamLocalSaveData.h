#ifndef traktor_online_SteamLocalSaveData_H
#define traktor_online_SteamLocalSaveData_H

#include "Online/Provider/ISaveDataProvider.h"

namespace traktor
{
	namespace online
	{

class SteamLocalSaveData : public ISaveDataProvider
{
	T_RTTI_CLASS;

public:
	SteamLocalSaveData();

	virtual bool enumerate(std::set< std::wstring >& outSaveDataIds);

	virtual bool get(const std::wstring& saveDataId, Ref< ISerializable >& outAttachment);

	virtual bool set(const std::wstring& saveDataId, const SaveDataDesc& saveDataDesc, const ISerializable* attachment, bool replace);

	virtual bool remove(const std::wstring& saveDataId);
};

	}
}

#endif	// traktor_online_SteamLocalSaveData_H
