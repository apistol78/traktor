#include "Database/Remote/Messages/DbmGetInstanceRevision.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.DbmGetInstanceRevision", 0, DbmGetInstanceRevision, IMessage)

DbmGetInstanceRevision::DbmGetInstanceRevision(uint32_t handle)
:	m_handle(handle)
{
}

void DbmGetInstanceRevision::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"handle", m_handle);
}

	}
}
