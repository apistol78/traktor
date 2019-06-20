#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRef.h"
#include "Database/IEvent.h"
#include "Database/Remote/Messages/DbmGetEventResult.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.DbmGetEventResult", 0, DbmGetEventResult, IMessage)

DbmGetEventResult::DbmGetEventResult()
:	m_sqnr(0)
,	m_remote(false)
{
}

DbmGetEventResult::DbmGetEventResult(uint64_t sqnr, const IEvent* event, bool remote)
:	m_sqnr(sqnr)
,	m_event(event)
,	m_remote(remote)
{
}

void DbmGetEventResult::serialize(ISerializer& s)
{
	s >> Member< uint64_t >(L"sqnr", m_sqnr);
	s >> MemberRef< const IEvent >(L"event", m_event);
	s >> Member< bool >(L"remote", m_remote);
}

	}
}
