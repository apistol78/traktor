#include "Database/Remote/Messages/DbmRemoveInstance.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.DbmRemoveInstance", 0, DbmRemoveInstance, IMessage)

DbmRemoveInstance::DbmRemoveInstance(uint32_t handle)
:	m_handle(handle)
{
}

void DbmRemoveInstance::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"handle", m_handle);
}

	}
}
