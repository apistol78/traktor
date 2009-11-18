#include "Database/Remote/Messages/DbmCloseTransaction.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.DbmCloseTransaction", 0, DbmCloseTransaction, IMessage)

DbmCloseTransaction::DbmCloseTransaction(uint32_t handle)
:	m_handle(handle)
{
}

bool DbmCloseTransaction::serialize(ISerializer& s)
{
	return s >> Member< uint32_t >(L"handle", m_handle);
}

	}
}
