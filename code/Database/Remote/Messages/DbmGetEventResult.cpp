#include "Database/Remote/Messages/DbmGetEventResult.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.DbmGetEventResult", DbmGetEventResult, IMessage)

DbmGetEventResult::DbmGetEventResult()
:	m_haveEvent(false)
,	m_event(PeInvalid)
,	m_remote(false)
{
}

DbmGetEventResult::DbmGetEventResult(ProviderEvent event, const Guid& eventId, bool remote)
:	m_haveEvent(true)
,	m_event(int32_t(event))
,	m_eventId(eventId)
,	m_remote(remote)
{
}

bool DbmGetEventResult::serialize(ISerializer& s)
{
	s >> Member< bool >(L"haveEvent", m_haveEvent);
	if (m_haveEvent)
	{
		s >> Member< int32_t >(L"event", m_event);
		s >> Member< Guid >(L"eventId", m_eventId);
		s >> Member< bool >(L"remote", m_remote);
	}
	return true;
}

	}
}
