#include <cstring>
#include "Database/Remote/Messages/StmWrite.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.StmWrite", StmWrite, IMessage)

StmWrite::StmWrite(uint32_t handle, int32_t blockSize, const void* block)
:	m_handle(handle)
,	m_blockSize(blockSize)
{
	std::memcpy(m_block, block, blockSize);
}

bool StmWrite::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"handle", m_handle);
	s >> Member< int32_t >(L"blockSize", m_blockSize);
	s >> Member< void* >(L"block", m_block, (uint32_t&)m_blockSize);
	return true;
}

	}
}
