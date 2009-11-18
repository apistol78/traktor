#include "Database/Remote/Messages/DbmGetInstancePrimaryType.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.DbmGetInstancePrimaryType", DbmGetInstancePrimaryType, IMessage)

DbmGetInstancePrimaryType::DbmGetInstancePrimaryType(uint32_t handle)
:	m_handle(handle)
{
}

bool DbmGetInstancePrimaryType::serialize(ISerializer& s)
{
	return s >> Member< uint32_t >(L"handle", m_handle);
}

	}
}
