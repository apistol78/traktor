#include "Database/Remote/Messages/DbmOpen.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.DbmOpen", 0, DbmOpen, IMessage)

DbmOpen::DbmOpen(const std::wstring& name)
:	m_name(name)
{
}

void DbmOpen::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", m_name);
}

	}
}
