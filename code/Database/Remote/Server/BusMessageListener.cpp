/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Database/Remote/Server/BusMessageListener.h"
#include "Database/Remote/Server/Connection.h"
#include "Database/Remote/Messages/DbmPutEvent.h"
#include "Database/Remote/Messages/DbmGetEvent.h"
#include "Database/Remote/Messages/MsgStatus.h"
#include "Database/Remote/Messages/DbmGetEventResult.h"
#include "Database/Provider/IProviderBus.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.BusMessageListener", BusMessageListener, IMessageListener)

BusMessageListener::BusMessageListener(Connection* connection)
:	m_connection(connection)
{
	registerMessage< DbmPutEvent >(&BusMessageListener::messagePutEvent);
	registerMessage< DbmGetEvent >(&BusMessageListener::messageGetEvent);
}

bool BusMessageListener::messagePutEvent(const DbmPutEvent* message)
{
	uint32_t busHandle = message->getHandle();
	Ref< IProviderBus > bus = m_connection->getObject< IProviderBus >(busHandle);
	if (!bus)
	{
		m_connection->sendReply(MsgStatus(StFailure));
		return true;
	}

	bool result = bus->putEvent(message->getEvent());
	m_connection->sendReply(MsgStatus(result ? StSuccess : StFailure));
	return true;
}

bool BusMessageListener::messageGetEvent(const DbmGetEvent* message)
{
	uint32_t busHandle = message->getHandle();
	Ref< IProviderBus > bus = m_connection->getObject< IProviderBus >(busHandle);
	if (!bus)
	{
		m_connection->sendReply(MsgStatus(StFailure));
		return true;
	}

	Ref< const IEvent > event;
	bool remote;

	bool result = bus->getEvent(event, remote);
	if (!result)
	{
		m_connection->sendReply(DbmGetEventResult());
		return true;
	}

	m_connection->sendReply(DbmGetEventResult(
		event,
		remote
	));
	return true;
}

	}
}
