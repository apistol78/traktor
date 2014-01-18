#ifndef traktor_online_LocalSaveData_H
#define traktor_online_LocalSaveData_H

#include "Online/Provider/ISaveDataProvider.h"

namespace traktor
{
	namespace sql
	{

class IConnection;

	}

	namespace online
	{

class LocalSaveData : public ISaveDataProvider
{
	T_RTTI_CLASS;

public:
	LocalSaveData(sql::IConnection* db);

	virtual bool enumerate(std::set< std::wstring >& outSaveDataIds);

	virtual bool get(const std::wstring& saveDataId, Ref< ISerializable >& outAttachment);

	virtual bool set(const std::wstring& saveDataId, const SaveDataDesc& saveDataDesc, const ISerializable* attachment, bool replace);

	virtual bool remove(const std::wstring& saveDataId);

private:
	Ref< sql::IConnection > m_db;
};

	}
}

#endif	// traktor_online_LocalSaveData_H
