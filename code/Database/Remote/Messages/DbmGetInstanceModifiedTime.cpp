#include "Database/Remote/Messages/DbmGetInstanceModifiedTime.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.DbmGetInstanceModifiedTime", 0, DbmGetInstanceModifiedTime, IMessage)

DbmGetInstanceModifiedTime::DbmGetInstanceModifiedTime(uint32_t handle)
:	m_handle(handle)
{
}

void DbmGetInstanceModifiedTime::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"handle", m_handle);
}

	}
}
