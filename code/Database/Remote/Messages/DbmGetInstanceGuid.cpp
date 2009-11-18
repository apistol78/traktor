#include "Database/Remote/Messages/DbmGetInstanceGuid.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.DbmGetInstanceGuid", 0, DbmGetInstanceGuid, IMessage)

DbmGetInstanceGuid::DbmGetInstanceGuid(uint32_t handle)
:	m_handle(handle)
{
}

bool DbmGetInstanceGuid::serialize(ISerializer& s)
{
	return s >> Member< uint32_t >(L"handle", m_handle);
}

	}
}
