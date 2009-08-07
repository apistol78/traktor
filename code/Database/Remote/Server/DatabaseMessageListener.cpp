#include "Database/Remote/Server/DatabaseMessageListener.h"
#include "Database/Remote/Server/Configuration.h"
#include "Database/Remote/Server/Connection.h"
#include "Database/Remote/Messages/DbmOpen.h"
#include "Database/Remote/Messages/DbmClose.h"
#include "Database/Remote/Messages/DbmGetBus.h"
#include "Database/Remote/Messages/DbmGetRootGroup.h"
#include "Database/Remote/Messages/MsgStatus.h"
#include "Database/Remote/Messages/MsgHandleResult.h"
#include "Database/Local/LocalDatabase.h"
#include "Database/Compact/CompactDatabase.h"
#include "Database/Provider/IProviderBus.h"
#include "Database/Provider/IProviderGroup.h"
#include "Core/Io/Path.h"
#include "Core/Misc/String.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.DatabaseMessageListener", DatabaseMessageListener, IMessageListener)

DatabaseMessageListener::DatabaseMessageListener(const Configuration* configuration, Connection* connection)
:	m_configuration(configuration)
,	m_connection(connection)
{
	registerMessage< DbmOpen >(&DatabaseMessageListener::messageOpen);
	registerMessage< DbmClose >(&DatabaseMessageListener::messageClose);
	registerMessage< DbmGetBus >(&DatabaseMessageListener::messageGetBus);
	registerMessage< DbmGetRootGroup >(&DatabaseMessageListener::messageGetRootGroup);
}

bool DatabaseMessageListener::messageOpen(const DbmOpen* message)
{
	if (m_connection->getDatabase())
	{
		m_connection->sendReply(MsgStatus(StFailure));
		return true;
	}

	std::wstring manifest = m_configuration->getDatabaseManifest(message->getName());
	if (!manifest.empty())
	{
		Ref< IProviderDatabase > database;

		if (endsWith(toLower(manifest), L".manifest"))
		{
			Ref< LocalDatabase > localDatabase = gc_new< LocalDatabase >();
			if (!localDatabase->open(manifest))
			{
				m_connection->sendReply(MsgStatus(StFailure));
				return true;
			}
			database = localDatabase;
		}
		else if (endsWith(toLower(manifest), L".compact"))
		{
			Ref< CompactDatabase > compactDatabase = gc_new< CompactDatabase >();
			if (!compactDatabase->open(manifest))
			{
				m_connection->sendReply(MsgStatus(StFailure));
				return true;
			}
			database = compactDatabase;
		}

		T_ASSERT (database);
		m_connection->setDatabase(database);

		m_connection->sendReply(MsgStatus(StSuccess));
	}
	else
		m_connection->sendReply(MsgStatus(StFailure));

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
