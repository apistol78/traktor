#include "Database/Remote/Messages/DbmGetDataNames.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.DbmGetDataNames", 0, DbmGetDataNames, IMessage)

DbmGetDataNames::DbmGetDataNames(uint32_t handle)
:	m_handle(handle)
{
}

void DbmGetDataNames::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"handle", m_handle);
}

	}
}
