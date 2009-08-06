#include "Database/Remote/Messages/DbmOpen.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.db.DbmOpen", DbmOpen, IMessage)

DbmOpen::DbmOpen(const std::wstring& name)
:	m_name(name)
{
}

bool DbmOpen::serialize(Serializer& s)
{
	return s >> Member< std::wstring >(L"name", m_name);
}

	}
}
