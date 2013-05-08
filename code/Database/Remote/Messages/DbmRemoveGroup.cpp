#include "Database/Remote/Messages/DbmRemoveGroup.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.DbmRemoveGroup", 0, DbmRemoveGroup, IMessage)

DbmRemoveGroup::DbmRemoveGroup(uint32_t handle)
:	m_handle(handle)
{
}

void DbmRemoveGroup::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"handle", m_handle);
}

	}
}
