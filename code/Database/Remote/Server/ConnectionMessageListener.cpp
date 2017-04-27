/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Database/Remote/Server/ConnectionMessageListener.h"
#include "Database/Remote/Server/Connection.h"
#include "Database/Remote/Messages/CnmReleaseObject.h"
#include "Database/Remote/Messages/MsgStatus.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.ConnectionMessageListener", ConnectionMessageListener, IMessageListener)

ConnectionMessageListener::ConnectionMessageListener(Connection* connection)
:	m_connection(connection)
{
	registerMessage< CnmReleaseObject >(&ConnectionMessageListener::messageReleaseObject);
}

bool ConnectionMessageListener::messageReleaseObject(const CnmReleaseObject* message)
{
	m_connection->releaseObject(message->getHandle());
	m_connection->sendReply(MsgStatus(StSuccess));
	return true;
}

	}
}
