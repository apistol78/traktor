#include "Database/Remote/Messages/StmStatResult.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.StmStatResult", StmStatResult, IMessage)

StmStatResult::StmStatResult(bool canRead, bool canWrite, bool canSeek, int32_t tell, int32_t available)
:	m_canRead(canRead)
,	m_canWrite(canWrite)
,	m_canSeek(canSeek)
,	m_tell(tell)
,	m_available(available)
{
}

bool StmStatResult::serialize(ISerializer& s)
{
	s >> Member< bool >(L"canRead", m_canRead);
	s >> Member< bool >(L"canWrite", m_canWrite);
	s >> Member< bool >(L"canSeek", m_canSeek);
	s >> Member< int32_t >(L"tell", m_tell);
	s >> Member< int32_t >(L"available", m_available);
	return true;
}

	}
}
