#include "Avalanche/Dictionary.h"
#include "Avalanche/Server/Connection.h"
#include "Avalanche/Server/Server.h"
#include "Avalanche/Server/Peer.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Net/SocketAddressIPv4.h"
#include "Net/TcpSocket.h"
#include "Net/Discovery/DiscoveryManager.h"
#include "Net/Discovery/NetworkService.h"

namespace traktor
{
	namespace avalanche
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.avalanche.Server", Server, Object)

bool Server::create(const PropertyGroup* settings)
{
	// Create server listening socket.
	const int32_t listenPort = settings->getProperty< int32_t >(L"Avalanche.Port", 40001);

	m_serverSocket = new net::TcpSocket();
	if (!m_serverSocket->bind(net::SocketAddressIPv4(listenPort)))
	{
		log::error << L"Unable to bind server socket." << Endl;
		return false;
	}

	if (!m_serverSocket->listen())
	{
		log::error << L"Unable to listen on server socket." << Endl;
		return false;
	}

	// Get our best external interface.
    net::SocketAddressIPv4::Interface itf;
    if (!net::SocketAddressIPv4::getBestInterface(itf))
    {
        log::error << L"Unable to get interfaces." << Endl;
        return false;
    }

	// Broadcast our self on the network.
	Ref< PropertyGroup > publishSettings = DeepClone(settings).create< PropertyGroup >();
	publishSettings->setProperty< PropertyString >(L"Avalanche.Host", itf.addr->getHostName());

	m_discoveryManager = new net::DiscoveryManager();
	m_discoveryManager->create(net::MdFindServices | net::MdPublishServices);
	m_discoveryManager->addService(new net::NetworkService(
		L"Traktor.Avalanche",
		publishSettings
	));

	// Create our dictionary.
	m_dictionary = new Dictionary();
	return true;
}

void Server::destroy()
{
	safeClose(m_serverSocket);
	safeDestroy(m_discoveryManager);
	m_dictionary = nullptr;
}

bool Server::update()
{
	// Accept new connections.
	if (m_serverSocket->select(true, false, false, 500) > 0)
	{
		Ref< net::TcpSocket > clientSocket = m_serverSocket->accept();
		if (clientSocket)
		{
			Ref< Connection > connection = new Connection(m_dictionary);
			if (connection->create(clientSocket))
				m_connections.push_back(connection);
		}
	}

	// Cleanup terminated connections.
	{
		auto it = std::remove_if(m_connections.begin(), m_connections.end(), [](Connection* connection) {
			return !connection->update();
		});
		if (it != m_connections.end())
		{
			m_connections.erase(it, m_connections.end());
			log::info << L"Connections removed." << Endl;
		}
	}

	// Search for peers.
	RefArray< Peer > peers;
	RefArray< net::NetworkService > services;
	m_discoveryManager->findServices< net::NetworkService >(services);
	for (auto service : services)
	{
		if (service->getType() == L"Traktor.Avalanche")
		{
			auto settings = service->getProperties();

			net::SocketAddressIPv4 peerAddress(
				settings->getProperty< std::wstring >(L"Avalanche.Host"),
				settings->getProperty< int32_t >(L"Avalanche.Port")
			);
			if (!peerAddress.valid())
				continue;

			auto it = std::find_if(m_peers.begin(), m_peers.end(), [&](Peer* peer) {
				return peer->getServerAddress() == peerAddress;
			});
			if (it != m_peers.end())
				peers.push_back(*it);
			else
			{
				log::info << L"Found new peer at " << peerAddress.getHostName() << L":" << peerAddress.getPort() << Endl;
				peers.push_back(new Peer(peerAddress, m_dictionary));
			}
		}
	}
	for (auto peer : m_peers)
	{
		if (std::find(peers.begin(), peers.end(), peer) == peers.end())
		{
			const net::SocketAddressIPv4& peerAddress = peer->getServerAddress();
			log::info << L"Peer at " << peerAddress.getHostName() << L":" << peerAddress.getPort() << L" disconnected." << Endl;
		}
	}
	m_peers = peers;

	return true;
}

	}
}
