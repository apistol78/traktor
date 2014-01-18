#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Misc/Base64.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Online/Local/LocalSaveData.h"
#include "Sql/IConnection.h"
#include "Sql/IResultSet.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.LocalSaveData", LocalSaveData, ISaveDataProvider)

LocalSaveData::LocalSaveData(sql::IConnection* db)
:	m_db(db)
{
}

bool LocalSaveData::enumerate(std::set< std::wstring >& outSaveDataIds)
{
	Ref< sql::IResultSet > rs;

	rs = m_db->executeQuery(L"select id from SaveData");
	if (!rs)
		return false;

	while (rs->next())
	{
		outSaveDataIds.insert(
			rs->getString(0)
		);
	}

	return true;
}

bool LocalSaveData::get(const std::wstring& saveDataId, Ref< ISerializable >& outAttachment)
{
	Ref< sql::IResultSet > rs;

	rs = m_db->executeQuery(L"select attachment from SaveData where id='" + saveDataId + L"'");
	if (!rs || !rs->next())
		return false;

	std::wstring ab64 = rs->getString(0);
	std::vector< uint8_t > ad = Base64().decode(ab64);

	DynamicMemoryStream dms(ad, true, false);
	outAttachment = BinarySerializer(&dms).readObject();

	return true;
}

bool LocalSaveData::set(const std::wstring& saveDataId, const SaveDataDesc& saveDataDesc, const ISerializable* attachment, bool replace)
{
	Ref< sql::IResultSet > rs;

	rs = m_db->executeQuery(L"select count(*) from SaveData where id='" + saveDataId + L"'");
	if (!rs || !rs->next())
		return false;

	bool exists = bool(rs->getInt32(0) > 0);
	if (exists && !replace)
		return false;

	DynamicMemoryStream dms(false, true);
	BinarySerializer(&dms).writeObject(attachment);
	std::wstring ab64 = Base64().encode(dms.getBuffer(), false);

	if (exists)
	{
		if (m_db->executeUpdate(L"update SaveData set attachment='" + ab64 + L"' where id='" + saveDataId + L"'") < 0)
			return false;
	}
	else
	{
		if (m_db->executeUpdate(L"insert into SaveData (id, attachment) values ('" + saveDataId + L"', '" + ab64 + L"')") < 0)
			return false;
	}

	return true;
}

bool LocalSaveData::remove(const std::wstring& saveDataId)
{
	return m_db->executeUpdate(L"delete from SaveData where id='" + saveDataId + L"'") > 0;
}

	}
}
