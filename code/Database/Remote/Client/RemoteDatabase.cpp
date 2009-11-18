#include "Database/Remote/Client/RemoteDatabase.h"
#include "Database/Remote/Client/RemoteBus.h"
#include "Database/Remote/Client/RemoteGroup.h"
#include "Database/Remote/Client/Connection.h"
#include "Database/Remote/Messages/DbmOpen.h"
#include "Database/Remote/Messages/DbmClose.h"
#include "Database/Remote/Messages/DbmGetBus.h"
#include "Database/Remote/Messages/DbmGetRootGroup.h"
#include "Database/Remote/Messages/MsgStatus.h"
#include "Database/Remote/Messages/MsgHandleResult.h"
#include "Net/Network.h"
#include "Net/TcpSocket.h"
#include "Net/SocketAddressIPv4.h"
#include "Core/Misc/WildCompare.h"
#include "Core/Misc/String.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.RemoteDatabase", RemoteDatabase, IProviderDatabase)

bool RemoteDatabase::open(const std::wstring& connectionString)
{
	if (!net::Network::initialize())
	{
		log::error << L"Failed to open database; network initialization failed" << Endl;
		return false;
	}

	std::vector< std::wstring > parts;
	if (!WildCompare(L"*:*/*").match(connectionString, WildCompare::CmIgnoreCase, &parts))
	{
		log::error << L"Failed to open database; invalid connection string" << Endl;
		return false;
	}

	std::wstring host = parts[0];
	uint16_t port = parseString< uint16_t >(parts[1]);

	Ref< net::TcpSocket > socket = new net::TcpSocket();
	if (!socket->connect(net::SocketAddressIPv4(host, port)))
	{
		log::error << L"Failed to open database; unable to connect to server at \"" << host << L"\" port " << port << Endl;
		return false;
	}

	m_connection = new Connection(socket);

	std::wstring name = parts[2];

	Ref< MsgStatus > result = m_connection->sendMessage< MsgStatus >(DbmOpen(name));
	if (!result || result->getStatus() != StSuccess)
	{
		log::error << L"Failed to open database; unable to open server database \"" << name << L"\"" << Endl;
		return false;
	}

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

		//Heap::getInstance().invalidateRefs(m_connection);
		T_ASSERT (m_connection == 0);

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
