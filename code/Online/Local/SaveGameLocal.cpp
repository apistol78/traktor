#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Misc/Base64.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Online/Local/SaveGameLocal.h"
#include "Sql/IConnection.h"
#include "Sql/IResultSet.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.SaveGameLocal", SaveGameLocal, ISaveGame)

SaveGameLocal::SaveGameLocal(sql::IConnection* db, int32_t id, const std::wstring& name)
:	m_db(db)
,	m_id(id)
,	m_name(name)
{
}

std::wstring SaveGameLocal::getName() const
{
	return m_name;
}

Ref< ISerializable > SaveGameLocal::getAttachment() const
{
	Ref< sql::IResultSet > rs;

	rs = m_db->executeQuery(L"select attachment from SaveGames where id=" + toString(m_id));
	if (!rs || !rs->next())
		return 0;

	std::wstring ab64 = rs->getString(0);
	std::vector< uint8_t > ad = Base64().decode(ab64);

	DynamicMemoryStream dms(ad, true, false);
	return BinarySerializer(&dms).readObject();
}

	}
}
