#include "Database/Remote/Messages/DbmCreateInstance.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.DbmCreateInstance", 0, DbmCreateInstance, IMessage)

DbmCreateInstance::DbmCreateInstance(uint32_t handle, const std::wstring& name, const Guid& guid)
:	m_handle(handle)
,	m_name(name)
,	m_guid(guid)
{
}

void DbmCreateInstance::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"handle", m_handle);
	s >> Member< std::wstring >(L"name", m_name);
	s >> Member< Guid >(L"guid", m_guid);
}

	}
}
