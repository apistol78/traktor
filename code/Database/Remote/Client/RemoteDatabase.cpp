#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Database/ConnectionString.h"
#include "Database/Remote/Client/RemoteConnection.h"
#include "Database/Remote/Client/RemoteBus.h"
#include "Database/Remote/Client/RemoteDatabase.h"
#include "Database/Remote/Client/RemoteGroup.h"
#include "Database/Remote/Messages/DbmOpen.h"
#include "Database/Remote/Messages/DbmClose.h"
#include "Database/Remote/Messages/DbmGetBus.h"
#include "Database/Remote/Messages/DbmGetRootGroup.h"
#include "Database/Remote/Messages/MsgHandleResult.h"
#include "Database/Remote/Messages/MsgIntResult.h"
#include "Net/Network.h"
#include "Net/SocketAddressIPv4.h"
#include "Net/TcpSocket.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.RemoteDatabase", 0, RemoteDatabase, IProviderDatabase)

bool RemoteDatabase::create(const ConnectionString& connectionString)
{
	return false;
}

bool RemoteDatabase::open(const ConnectionString& connectionString)
{
	if (!connectionString.have(L"host") || !connectionString.have(L"database"))
		return false;

	if (!net::Network::initialize())
	{
		log::error << L"Failed to open database; network initialization failed" << Endl;
		return false;
	}

	std::wstring host = connectionString.get(L"host");
	std::wstring database = connectionString.get(L"database");
	uint16_t port = 33666;

	size_t p = host.find(L':');
	if (p != host.npos)
	{
		port = parseString< uint16_t >(host.substr(p + 1));
		host = host.substr(0, p);
	}

	Ref< net::TcpSocket > socket = new net::TcpSocket();
	if (!socket->connect(net::SocketAddressIPv4(host, port)))
	{
		log::error << L"Failed to open database; unable to connect to server \"" << host << L"\" (port " << port << L")" << Endl;
		return false;
	}

	m_connection = new RemoteConnection(socket);

	Ref< MsgIntResult > result = m_connection->sendMessage< MsgIntResult >(DbmOpen(database));
	if (!result || result->get() <= 0)
	{
		log::error << L"Failed to open database; unable to open server database \"" << database << L"\"" << Endl;
		return false;
	}

	m_connection->setStreamServerAddr(net::SocketAddressIPv4(host, result->get()));

	return true;
}

void RemoteDatabase::close()
{
	if (m_connection)
	{
		Ref< MsgStatus > result = m_connection->sendMessage< MsgStatus >(DbmClose());
		if (!result || result->getStatus() != StSuccess)
			log::warning << L"Unable to close server database" << Endl;

		m_connection->destroy();
		m_connection = 0;

		net::Network::finalize();
	}
}

Ref< IProviderBus > RemoteDatabase::getBus()
{
	if (!m_connection)
		return 0;

	Ref< MsgHandleResult > result = m_connection->sendMessage< MsgHandleResult >(DbmGetBus());
	return result ? new RemoteBus(m_connection, result->get()) : 0;
}

Ref< IProviderGroup > RemoteDatabase::getRootGroup()
{
	if (!m_connection)
		return 0;

	Ref< MsgHandleResult > result = m_connection->sendMessage< MsgHandleResult >(DbmGetRootGroup());
	return result ? new RemoteGroup(m_connection, result->get()) : 0;
}

	}
}
