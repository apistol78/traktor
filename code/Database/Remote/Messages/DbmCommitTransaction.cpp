#include "Database/Remote/Messages/DbmCommitTransaction.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.DbmCommitTransaction", 0, DbmCommitTransaction, IMessage)

DbmCommitTransaction::DbmCommitTransaction(uint32_t handle)
:	m_handle(handle)
{
}

void DbmCommitTransaction::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"handle", m_handle);
}

	}
}
