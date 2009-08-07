#include "Database/Remote/Messages/DbmPutEvent.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.db.DbmPutEvent", DbmPutEvent, IMessage)

DbmPutEvent::DbmPutEvent(uint32_t handle, ProviderEvent event, const Guid& eventId)
:	m_handle(handle)
,	m_event(int32_t(event))
,	m_eventId(eventId)
{
}

bool DbmPutEvent::serialize(Serializer& s)
{
	s >> Member< uint32_t >(L"handle", m_handle);
	s >> Member< int32_t >(L"event", m_event);
	s >> Member< Guid >(L"eventId", m_eventId);
	return true;
}

	}
}
