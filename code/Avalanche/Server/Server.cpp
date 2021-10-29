#include "Avalanche/Dictionary.h"
#include "Avalanche/IBlob.h"
#include "Avalanche/Server/Connection.h"
#include "Avalanche/Server/Server.h"
#include "Avalanche/Server/Peer.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Core/System/OS.h"
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
	m_instanceId = Guid::create();
	T_ASSERT(m_instanceId.isValid());

	// Create server listening socket.
	const int32_t listenPort = settings->getProperty< int32_t >(L"Avalanche.Port", 40001);

	m_serverSocket = new net::TcpSocket();
	if (!m_serverSocket->bind(net::SocketAddressIPv4(listenPort), true))
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

	// Create our dictionary.
	m_dictionary = new Dictionary();
	if (!m_dictionary->create(
		settings->getProperty< std::wstring >(L"Avalanche.Path", L"")
	))
	{
		log::error << L"Unable to create dictionary." << Endl;
		return false;		
	}

	m_master = settings->getProperty< bool >(L"Avalanche.Master", false);
	m_memoryBudget = settings->getProperty< int32_t >(L"Avalanche.MemoryBudget", 8);

	// Broadcast our self on the network.
	Ref< PropertyGroup > publishSettings = DeepClone(settings).create< PropertyGroup >();
	publishSettings->setProperty< PropertyInteger >(L"Avalanche.Version.Major", c_majorVersion);
	publishSettings->setProperty< PropertyInteger >(L"Avalanche.Version.Minor", c_minorVersion);
	publishSettings->setProperty< PropertyString >(L"Avalanche.OS.Name", OS::getInstance().getName());
	publishSettings->setProperty< PropertyString >(L"Avalanche.OS.Identifier", OS::getInstance().getIdentifier());
	publishSettings->setProperty< PropertyString >(L"Avalanche.OS.ComputerName", OS::getInstance().getComputerName());
	publishSettings->setProperty< PropertyString >(L"Avalanche.Host", itf.addr->getHostName());
	publishSettings->setProperty< PropertyString >(L"Avalanche.InstanceID", m_instanceId.format());

	m_discoveryManager = new net::DiscoveryManager();
	m_discoveryManager->create(net::MdFindServices | net::MdPublishServices);
	m_discoveryManager->addService(new net::NetworkService(
		L"Traktor.Avalanche",
		publishSettings
	));

	log::info << L"Server started successfully (" << (m_master ? L"Master" : L"Slave") << L")." << Endl;
	return true;
}

void Server::destroy()
{
	m_connections.clear();
	m_peers.clear();
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
			size_t n = std::distance(it, m_connections.end());
			m_connections.erase(it, m_connections.end());
		}
	}

	// Search for master peers.
	RefArray< Peer > peers;
	RefArray< net::NetworkService > services;
	m_discoveryManager->findServices< net::NetworkService >(services);
	for (auto service : services)
	{
		if (service->getType() == L"Traktor.Avalanche")
		{
			auto settings = service->getProperties();
			if (!settings)
				continue;

			// If I'm no master and peer is not a master then we ignore this peer.
			bool peerMaster = settings->getProperty< bool >(L"Avalanche.Master", false);
			if (!m_master && !peerMaster)
				continue;

			Guid peerInstanceId = Guid(settings->getProperty< std::wstring >(L"Avalanche.InstanceID", L""));
			if (!peerInstanceId.isValid())
				continue;

			net::SocketAddressIPv4 peerAddress(
				settings->getProperty< std::wstring >(L"Avalanche.Host"),
				settings->getProperty< int32_t >(L"Avalanche.Port")
			);
			if (!peerAddress.valid())
				continue;

			auto it = std::find_if(m_peers.begin(), m_peers.end(), [&](Peer* peer) {
				return peer->getInstanceId() == peerInstanceId;
			});
			if (it != m_peers.end())
				peers.push_back(*it);
			else
			{
				int32_t majorVersion = settings->getProperty< int32_t >(L"Avalanche.Version.Major", 0);
				int32_t minorVersion = settings->getProperty< int32_t >(L"Avalanche.Version.Minor", 0);
				if (
					majorVersion == c_majorVersion &&
					minorVersion >= c_minorVersion
				)
				{
					std::wstring peerName = settings->getProperty< std::wstring >(L"Avalanche.OS.Name", L"");
					std::wstring peerIdentifier = settings->getProperty< std::wstring >(L"Avalanche.OS.Identifier", L"");
					std::wstring peerComputerName = settings->getProperty< std::wstring >(L"Avalanche.OS.ComputerName", L"");

					if (peerMaster)
						log::info << L"Found master peer at " << peerAddress.getHostName() << L":" << peerAddress.getPort() << Endl;
					else
						log::info << L"Found slave peer at " << peerAddress.getHostName() << L":" << peerAddress.getPort() << Endl;

					log::info << L"  Name          : " << peerName << Endl;
					log::info << L"  Identifier    : " << peerIdentifier << Endl;
					log::info << L"  Computer name : " << peerComputerName << Endl;
					log::info << L"  Instance ID   : " << peerInstanceId.format() << Endl;
					log::info << L"  Protocol      : " << majorVersion << L"." << minorVersion << Endl;

					std::wstring name = !peerComputerName.empty() ? peerComputerName : str(L"%s:%d", peerAddress.getHostName().c_str(), peerAddress.getPort());
					peers.push_back(new Peer(peerAddress, peerInstanceId, peerMaster, name, m_dictionary));
				}
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

	// Execute eviction policy.
	if (m_master)
	{
		const uint64_t maxMemoryUsage = (uint64_t)m_memoryBudget * 1024UL * 1024UL * 1024UL;

		Dictionary::Stats stats;
		if (m_dictionary->getStats(stats))
		{
			if (stats.memoryUsage >= maxMemoryUsage)
			{
				AlignedVector< std::pair< Key, DateTime > > queue;

				// Get snapshot of keys and last access time stamps.
				AlignedVector< Key > keys;
				m_dictionary->snapshotKeys(keys);
				for (const auto& key : keys)
				{
					Ref< IBlob > blob = m_dictionary->get(key, true);
					if (!blob)
						continue;
					queue.push_back({
						key,
						blob->lastAccessed()
					});
				}

				// Sort eviction candidates on longest since access first.
				std::sort(queue.begin(), queue.end(), [](const std::pair< Key, DateTime >& lh, const std::pair< Key, DateTime >& rh) {
					return lh.second < rh.second;
				});

				// Evict blobs until memory budget is meet.
				auto it = queue.begin();
				while (it != queue.end() && stats.memoryUsage >= maxMemoryUsage)
				{
					Ref< IBlob > blob = m_dictionary->get(it->first, true);
					if (!blob)
						continue;

					stats.memoryUsage -= blob->size();
					m_dictionary->remove(it->first);

					++it;
				}
			}
		}
	}

	return true;
}

	}
}
