/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/Path.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Semaphore.h"
#include "Database/ConnectionString.h"
#include "Database/Provider/IProviderBus.h"
#include "Database/Provider/IProviderDatabase.h"
#include "Database/Provider/IProviderGroup.h"
#include "Database/Remote/Messages/DbmOpen.h"
#include "Database/Remote/Messages/DbmClose.h"
#include "Database/Remote/Messages/DbmGetBus.h"
#include "Database/Remote/Messages/DbmGetRootGroup.h"
#include "Database/Remote/Messages/MsgHandleResult.h"
#include "Database/Remote/Messages/MsgIntResult.h"
#include "Database/Remote/Messages/MsgStatus.h"
#include "Database/Remote/Server/DatabaseMessageListener.h"
#include "Database/Remote/Server/Connection.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.DatabaseMessageListener", DatabaseMessageListener, IMessageListener)

DatabaseMessageListener::DatabaseMessageListener(
	Semaphore& connectionStringsLock,
	const std::map< std::wstring, std::wstring >& connectionStrings,
	uint16_t streamServerPort,
	Connection* connection
)
:	m_connectionStringsLock(connectionStringsLock)
,	m_connectionStrings(connectionStrings)
,	m_streamServerPort(streamServerPort)
,	m_connection(connection)
{
	registerMessage< DbmOpen >(&DatabaseMessageListener::messageOpen);
	registerMessage< DbmClose >(&DatabaseMessageListener::messageClose);
	registerMessage< DbmGetBus >(&DatabaseMessageListener::messageGetBus);
	registerMessage< DbmGetRootGroup >(&DatabaseMessageListener::messageGetRootGroup);
}

bool DatabaseMessageListener::messageOpen(const DbmOpen* message)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_connectionStringsLock);

	if (m_connection->getDatabase())
	{
		m_connection->sendReply(MsgStatus(StFailure));
		return true;
	}

	std::map< std::wstring, std::wstring >::const_iterator i = m_connectionStrings.find(message->getName());
	if (i == m_connectionStrings.end())
	{
		m_connection->sendReply(MsgStatus(StFailure));
		return true;
	}

	ConnectionString connectionString = i->second;

	if (!connectionString.have(L"provider"))
	{
		m_connection->sendReply(MsgStatus(StFailure));
		return true;
	}

	const TypeInfo* providerType = TypeInfo::find(connectionString.get(L"provider"));
	if (!providerType)
	{
		m_connection->sendReply(MsgStatus(StFailure));
		return true;
	}

	Ref< IProviderDatabase > providerDatabase = checked_type_cast< IProviderDatabase* >(providerType->createInstance());
	if (!providerDatabase)
	{
		m_connection->sendReply(MsgStatus(StFailure));
		return true;
	}

	if (!providerDatabase->open(connectionString))
	{
		m_connection->sendReply(MsgStatus(StFailure));
		return true;
	}

	m_connection->setDatabase(providerDatabase);

	log::info << L"Database \"" << message->getName() << L"\" opened successfully" << Endl;
	m_connection->sendReply(MsgIntResult(m_streamServerPort));

	return true;
}

bool DatabaseMessageListener::messageClose(const DbmClose* message)
{
	if (!m_connection->getDatabase())
	{
		m_connection->sendReply(MsgStatus(StFailure));
		return true;
	}

	m_connection->getDatabase()->close();
	m_connection->setDatabase(0);

	m_connection->sendReply(MsgStatus(StSuccess));
	return true;
}

bool DatabaseMessageListener::messageGetBus(const DbmGetBus* message)
{
	if (!m_connection->getDatabase())
	{
		m_connection->sendReply(MsgStatus(StFailure));
		return true;
	}

	Ref< IProviderBus > bus = m_connection->getDatabase()->getBus();
	if (!bus)
	{
		m_connection->sendReply(MsgStatus(StFailure));
		return true;
	}

	uint32_t busHandle = m_connection->putObject(bus);
	m_connection->sendReply(MsgHandleResult(busHandle));
	return true;
}

bool DatabaseMessageListener::messageGetRootGroup(const DbmGetRootGroup* message)
{
	if (!m_connection->getDatabase())
	{
		m_connection->sendReply(MsgStatus(StFailure));
		return true;
	}

	Ref< IProviderGroup > rootGroup = m_connection->getDatabase()->getRootGroup();
	if (!rootGroup)
	{
		m_connection->sendReply(MsgStatus(StFailure));
		return true;
	}

	uint32_t rootGroupHandle = m_connection->putObject(rootGroup);
	m_connection->sendReply(MsgHandleResult(rootGroupHandle));
	return true;
}

	}
}
