#include "Database/Remote/Messages/DbmGetDataNames.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.DbmGetDataNames", DbmGetDataNames, IMessage)

DbmGetDataNames::DbmGetDataNames(uint32_t handle)
:	m_handle(handle)
{
}

bool DbmGetDataNames::serialize(ISerializer& s)
{
	return s >> Member< uint32_t >(L"handle", m_handle);
}

	}
}
