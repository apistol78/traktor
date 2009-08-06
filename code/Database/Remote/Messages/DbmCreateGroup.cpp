#include "Database/Remote/Messages/DbmCreateGroup.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.db.DbmCreateGroup", DbmCreateGroup, IMessage)

DbmCreateGroup::DbmCreateGroup(uint32_t handle, const std::wstring& name)
:	m_handle(handle)
,	m_name(name)
{
}

bool DbmCreateGroup::serialize(Serializer& s)
{
	s >> Member< uint32_t >(L"handle", m_handle);
	s >> Member< std::wstring >(L"name", m_name);
	return true;
}

	}
}
