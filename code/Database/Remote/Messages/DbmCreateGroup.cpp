#include "Database/Remote/Messages/DbmCreateGroup.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.DbmCreateGroup", 0, DbmCreateGroup, IMessage)

DbmCreateGroup::DbmCreateGroup(uint32_t handle, const std::wstring& name)
:	m_handle(handle)
,	m_name(name)
{
}

void DbmCreateGroup::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"handle", m_handle);
	s >> Member< std::wstring >(L"name", m_name);
}

	}
}
