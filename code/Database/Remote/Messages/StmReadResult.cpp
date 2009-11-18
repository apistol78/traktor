#include <cstring>
#include "Database/Remote/Messages/StmReadResult.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.StmReadResult", 0, StmReadResult, IMessage)

StmReadResult::StmReadResult(const void* block, int32_t blockSize)
:	m_blockSize(blockSize)
{
	if (blockSize > 0)
	{
		T_ASSERT (block);
		std::memcpy(m_block, block, blockSize);
	}
}

bool StmReadResult::serialize(ISerializer& s)
{
	s >> Member< int32_t >(L"blockSize", m_blockSize);
	s >> Member< void* >(L"block", m_block, (uint32_t&)m_blockSize);
	return true;
}

	}
}
