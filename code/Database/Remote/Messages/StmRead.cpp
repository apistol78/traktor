#include "Database/Remote/Messages/StmRead.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.StmRead", StmRead, IMessage)

StmRead::StmRead(uint32_t handle, int32_t blockSize)
:	m_handle(handle)
,	m_blockSize(blockSize)
{
}

bool StmRead::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"handle", m_handle);
	s >> Member< int32_t >(L"blockSize", m_blockSize);
	return true;
}

	}
}
