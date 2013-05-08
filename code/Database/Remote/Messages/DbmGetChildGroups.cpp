#include "Database/Remote/Messages/DbmGetChildGroups.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.DbmGetChildGroups", 0, DbmGetChildGroups, IMessage)

DbmGetChildGroups::DbmGetChildGroups(uint32_t handle)
:	m_handle(handle)
{
}

void DbmGetChildGroups::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"handle", m_handle);
}

	}
}
