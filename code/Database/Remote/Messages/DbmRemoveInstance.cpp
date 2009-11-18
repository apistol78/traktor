#include "Database/Remote/Messages/DbmRemoveInstance.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.DbmRemoveInstance", DbmRemoveInstance, IMessage)

DbmRemoveInstance::DbmRemoveInstance(uint32_t handle)
:	m_handle(handle)
{
}

bool DbmRemoveInstance::serialize(ISerializer& s)
{
	return s >> Member< uint32_t >(L"handle", m_handle);
}

	}
}
