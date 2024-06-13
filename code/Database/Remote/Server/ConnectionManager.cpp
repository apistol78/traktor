/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include "Core/Log/Log.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Database/Remote/Server/Connection.h"
#include "Database/Remote/Server/ConnectionManager.h"
#include "Net/SocketAddressIPv4.h"
#include "Net/SocketSet.h"
#include "Net/TcpSocket.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.ConnectionManager", ConnectionManager, Object)

ConnectionManager::ConnectionManager(net::StreamServer* streamServer)
:	m_streamServer(streamServer)
,	m_listenPort(0)
,	m_serverThread(nullptr)
{
}

bool ConnectionManager::create()
{
	m_listenSocket = new net::TcpSocket();
	if (!m_listenSocket->bind(net::SocketAddressIPv4(0)))
	{
		log::error << L"Failed to create remote database connection manager; unable to bind server socket." << Endl;
		return false;
	}
	if (!m_listenSocket->listen())
	{
		log::error << L"Failed to create remote database connection manager; unable to listenen." << Endl;
		return false;
	}

	m_listenPort = dynamic_type_cast< net::SocketAddressIPv4* >(m_listenSocket->getLocalAddress())->getPort();

	m_serverThread = ThreadManager::getInstance().create(
		[=, this](){ threadServer(); },
		L"Database server"
	);
	if (!m_serverThread)
		return false;

	m_serverThread->start();

	log::info << L"Remote database connection manager @" << m_listenPort << L" created." << Endl;
	return true;
}

void ConnectionManager::destroy()
{
	if (m_serverThread)
	{
		m_serverThread->stop();
		ThreadManager::getInstance().destroy(m_serverThread);
		m_serverThread = nullptr;
	}

	if (m_listenSocket)
	{
		m_listenSocket->close();
		m_listenSocket = nullptr;
	}

	for (auto connection : m_connections)
		connection->destroy();

	m_connections.clear();

	m_streamServer = nullptr;
}

void ConnectionManager::setConnectionString(const std::wstring& name, const std::wstring& connectionString)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_connectionStringsLock);
	m_connectionStrings[name] = connectionString;
}

void ConnectionManager::removeConnectionString(const std::wstring& name)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_connectionStringsLock);
	m_connectionStrings.remove(name);
}

uint16_t ConnectionManager::getListenPort() const
{
	return m_listenPort;
}

void ConnectionManager::threadServer()
{
	while (!m_serverThread->stopped())
	{
		net::SocketSet ss;
		ss.add(m_listenSocket);
		for (const auto& connection : m_connections)
			ss.add(connection->getSocket());

		net::SocketSet result;
		if (ss.select(true, false, false, 100, result) <= 0)
			continue;
	
		// Accept new connections.
		if (result.contain(m_listenSocket))
		{
			Ref< net::TcpSocket > clientSocket = m_listenSocket->accept();
			if (!clientSocket)
				continue;

			Ref< Connection > connection = new Connection(m_connectionStringsLock, m_connectionStrings, m_streamServer, clientSocket);
			m_connections.push_back(connection);

			Ref< net::SocketAddress > remoteAddress = clientSocket->getRemoteAddress();
			if (remoteAddress)
				log::info << L"Remote database connection accepted from " << remoteAddress->getHostName() << L"." << Endl;
			else
				log::info << L"Remote database connection accepted." << Endl;
		}

		// Serve connections.
		uint32_t closed = 0;
		for (int32_t i = 0; i < result.count(); ++i)
		{
			Ref< net::Socket > socket = result.get(i);
			if (socket == m_listenSocket)
				continue;			

			auto it = std::find_if(m_connections.begin(), m_connections.end(), [&](const Connection* connection) {
				return connection->getSocket() == socket;
			});
			if (it == m_connections.end())
				continue;

			auto connection = *it;
			if (!connection->process())
			{
				m_connections.erase(it);
				closed++;
			}
		}
		if (closed)
			log::info << closed << L" remote database connection(s) removed." << Endl;
	}
}

	}
}
