#include "Database/Remote/Messages/DbmGetEvent.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.DbmGetEvent", DbmGetEvent, IMessage)

DbmGetEvent::DbmGetEvent(uint32_t handle)
:	m_handle(handle)
{
}

bool DbmGetEvent::serialize(ISerializer& s)
{
	return s >> Member< uint32_t >(L"handle", m_handle);
}

	}
}
