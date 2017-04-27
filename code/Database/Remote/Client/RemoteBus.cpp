/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Database/Remote/Client/RemoteBus.h"
#include "Database/Remote/Client/RemoteConnection.h"
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

RemoteBus::RemoteBus(RemoteConnection* connection, uint32_t handle)
:	m_connection(connection)
,	m_handle(handle)
{
}

RemoteBus::~RemoteBus()
{
	if (m_connection)
		m_connection->sendMessage< MsgStatus >(CnmReleaseObject(m_handle));
}

bool RemoteBus::putEvent(const IEvent* event)
{
	Ref< MsgStatus > result = m_connection->sendMessage< MsgStatus >(DbmPutEvent(m_handle, event));
	return result ? result->getStatus() == StSuccess : false;
}

bool RemoteBus::getEvent(Ref< const IEvent >& outEvent, bool& outRemote)
{
	Ref< DbmGetEventResult > result = m_connection->sendMessage< DbmGetEventResult >(DbmGetEvent(m_handle));
	if (!result)
		return false;

	if (!result->getEvent())
		return false;

	outEvent = result->getEvent();
	outRemote = result->getRemote();

	return true;
}

	}
}
