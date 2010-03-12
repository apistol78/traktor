#include <algorithm>
#include "Database/Remote/Server/ConnectionManager.h"
#include "Database/Remote/Server/Connection.h"
#include "Database/Remote/Server/Configuration.h"
#include "Net/TcpSocket.h"
#include "Net/SocketAddressIPv4.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.ConnectionManager", ConnectionManager, Object)

bool ConnectionManager::create(const Configuration* configuration)
{
	if (!net::Network::initialize())
	{
		log::error << L"Failed to create remote database connection managar; unable to initialize network" << Endl;
		return false;
	}

	m_configuration = configuration;
	if (!m_configuration)
	{
		log::error << L"Failed to create remote database connection manager; no configuration" << Endl;
		return false;
	}

	m_serverSocket = new net::TcpSocket();

	if (!m_serverSocket->bind(net::SocketAddressIPv4(m_configuration->getListenPort())))
	{
		log::error << L"Failed to create remote database connection manager; unable to bind server socket to port " << m_configuration->getListenPort() << Endl;
		return false;
	}

	if (!m_serverSocket->listen())
	{
		log::error << L"Failed to create remote database connection manager; unable to listenen to port " << m_configuration->getListenPort() << Endl;
		return false;
	}

	log::info << L"Remote database connection manager created" << Endl;
	return true;
}

void ConnectionManager::destroy()
{
	for (RefArray< Connection >::iterator i = m_connections.begin(); i != m_connections.end(); ++i)
		(*i)->destroy();

	m_connections.resize(0);

	if (m_serverSocket)
	{
		m_serverSocket->close();
		m_serverSocket = 0;
	}

	net::Network::finalize();
}

bool ConnectionManager::update(int32_t waitTimeout)
{
	if (!m_serverSocket)
		return false;

	if (!acceptConnections(waitTimeout))
		return false;

	if (!cleanupConnections())
		return false;

	return true;
}

bool ConnectionManager::acceptConnections(int32_t waitTimeout)
{
	if (m_serverSocket->select(true, false, false, waitTimeout) <= 0)
		return true;

	Ref< net::TcpSocket > clientSocket = m_serverSocket->accept();
	if (!clientSocket)
		return true;

	Ref< Connection > connection = new Connection(m_configuration, clientSocket);
	m_connections.push_back(connection);

	log::info << L"Remote database connection accepted" << Endl;
	return true;
}

bool ConnectionManager::cleanupConnections()
{
	uint32_t count = uint32_t(m_connections.size());

	for (RefArray< Connection >::iterator i = m_connections.begin(); i != m_connections.end(); )
	{
		if (!(*i)->alive())
			i = m_connections.erase(i);
		else
			++i;
	}

	count -= uint32_t(m_connections.size());

	if (count)
		log::info << count << L" remote database connection(s) removed" << Endl;

	return true;
}

	}
}
