#include "Database/Remote/Messages/DbmRemoveAllData.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.DbmRemoveAllData", 0, DbmRemoveAllData, IMessage)

DbmRemoveAllData::DbmRemoveAllData(uint32_t handle)
:	m_handle(handle)
{
}

void DbmRemoveAllData::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"handle", m_handle);
}

	}
}
