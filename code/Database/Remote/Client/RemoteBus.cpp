#include "Database/Remote/Client/RemoteBus.h"
#include "Database/Remote/Client/Connection.h"
#include "Database/Remote/Messages/CnmReleaseObject.h"
#include "Database/Remote/Messages/DbmPutEvent.h"
#include "Database/Remote/Messages/DbmGetEvent.h"
#include "Database/Remote/Messages/MsgStatus.h"
#include "Database/Remote/Messages/DbmGetEventResult.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.RemoteBus", RemoteBus, IProviderBus)

RemoteBus::RemoteBus(Connection* connection, uint32_t handle)
:	m_connection(connection)
,	m_handle(handle)
{
}

RemoteBus::~RemoteBus()
{
	if (m_connection)
		m_connection->sendMessage< MsgStatus >(CnmReleaseObject(m_handle));
}

bool RemoteBus::putEvent(ProviderEvent event, const Guid& eventId)
{
	Ref< MsgStatus > result = m_connection->sendMessage< MsgStatus >(DbmPutEvent(m_handle, event, eventId));
	return result ? result->getStatus() == StSuccess : false;
}

bool RemoteBus::getEvent(ProviderEvent& outEvent, Guid& outEventId, bool& outRemote)
{
	Ref< DbmGetEventResult > result = m_connection->sendMessage< DbmGetEventResult >(DbmGetEvent(m_handle));
	if (!result)
		return false;

	if (!result->haveEvent())
		return false;

	outEvent = result->getEvent();
	outEventId = result->getEventId();
	outRemote = result->getRemote();

	return true;
}

	}
}
