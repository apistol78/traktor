#include "Database/Remote/Messages/DbmGetInstanceName.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.DbmGetInstanceName", 0, DbmGetInstanceName, IMessage)

DbmGetInstanceName::DbmGetInstanceName(uint32_t handle)
:	m_handle(handle)
{
}

bool DbmGetInstanceName::serialize(ISerializer& s)
{
	return s >> Member< uint32_t >(L"handle", m_handle);
}

	}
}
