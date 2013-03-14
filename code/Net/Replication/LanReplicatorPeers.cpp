#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyInteger.h"
#include "Net/UdpSocket.h"
#include "Net/Discovery/DiscoveryManager.h"
#include "Net/Discovery/NetworkService.h"
#include "Net/Replication/LanReplicatorPeers.h"

namespace traktor
{
	namespace net
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.LanReplicatorPeers", LanReplicatorPeers, IReplicatorPeers)

LanReplicatorPeers::LanReplicatorPeers()
:	m_networkId(0)
{
}

bool LanReplicatorPeers::create()
{
	// Create discovery manager.
	m_discoveryManager = new net::DiscoveryManager();
	m_discoveryManager->create(false);

	// Find peers and connect to them.
	RefArray< net::NetworkService > peerServices;
	m_discoveryManager->findServices< net::NetworkService >(peerServices);

	// Determine safe port and id; need to be exclusive as we want to be able to run
	// multiple clients on same computer.
	std::set< uint16_t > occupiedPorts, occupiedIds;
	for (RefArray< net::NetworkService >::const_iterator i = peerServices.begin(); i != peerServices.end(); ++i)
	{
		if ((*i)->getType() != L"Parade/Replicator")
			continue;

		const PropertyGroup* properties = (*i)->getProperties();
		if (!properties)
			continue;

		occupiedPorts.insert(properties->getProperty< PropertyInteger >(L"Port"));
		occupiedIds.insert(properties->getProperty< PropertyInteger >(L"Id"));
	}

	// Create incoming transport.
	net::SocketAddressIPv4::Interface itf;
	if (!net::SocketAddressIPv4::getBestInterface(itf))
		return false;

	uint32_t addr = itf.addr->getAddr();
	uint16_t port = 33000;
	for (; port < 34000; ++port)
	{
		if (occupiedPorts.find(port) != occupiedPorts.end())
			continue;

		m_socketAddr = net::SocketAddressIPv4(addr, port);

		Ref< net::UdpSocket > socket = new net::UdpSocket();
		if (!socket->bind(m_socketAddr))
		{
			socket = 0;
			continue;
		}

		m_socket = socket;
		break;
	}

	if (!m_socket)
		return false;

	// Determine our network id.
	m_networkId = 1;
	for (; m_networkId; ++m_networkId)
	{
		if (occupiedIds.find(m_networkId) == occupiedIds.end())
			break;
	}

	// Publish ourself as discoverable.
	Ref< PropertyGroup > properties = new PropertyGroup();
	properties->setProperty< PropertyInteger >(L"Host", addr);
	properties->setProperty< PropertyInteger >(L"Port", port);
	properties->setProperty< PropertyInteger >(L"Id", m_networkId);
	m_discoveryManager->addService(new net::NetworkService(L"Parade/Replicator", properties));

	// Initialize all peer transports.
	for (RefArray< net::NetworkService >::const_iterator i = peerServices.begin(); i != peerServices.end(); ++i)
	{
		if ((*i)->getType() != L"Parade/Replicator")
			continue;

		const PropertyGroup* properties = (*i)->getProperties();
		if (!properties)
			continue;

		int32_t host = properties->getProperty< PropertyInteger >(L"Host");
		int32_t port = properties->getProperty< PropertyInteger >(L"Port");
		
		net::SocketAddressIPv4 socketAddr(host, port);

		Ref< net::UdpSocket > socket = new net::UdpSocket();
		if (!socket->connect(socketAddr))
			continue;

		Peer p;
		p.socketAddr = socketAddr;
		p.socket = socket;
		m_peers.push_back(p);
	}

	return true;
}

void LanReplicatorPeers::destroy()
{
	safeDestroy(m_discoveryManager);
}

void LanReplicatorPeers::update()
{
}

std::wstring LanReplicatorPeers::getName() const
{
	return L"";
}

uint64_t LanReplicatorPeers::getGlobalId() const
{
	return 0;
}

handle_t LanReplicatorPeers::getPrimaryPeerHandle() const
{
	return 0;
}

uint32_t LanReplicatorPeers::getPeerHandles(std::vector< handle_t >& outPeerHandles) const
{
	return 0;
}

std::wstring LanReplicatorPeers::getPeerName(handle_t handle) const
{
	return L""; // m_peers[peerId].socketAddr.getHostName();
}

uint64_t LanReplicatorPeers::getPeerGlobalId(handle_t handle) const
{
	return 0;
}

int32_t LanReplicatorPeers::receive(void* data, int32_t size, handle_t& outFromHandle)
{
	/*
	net::SocketAddressIPv4 fromAddr;
	if (m_socket->recvFrom(data, size, &fromAddr) <= 0)
		return false;

	// Find out from which peer we received from.
	for (uint32_t i = 0; i < m_peers.size(); ++i)
	{
		if (
			fromAddr.getAddr() == m_peers[i].socketAddr.getAddr() &&
			fromAddr.getPort() == m_peers[i].socketAddr.getPort()
		)
		{
			outFromPeer = i;
			return true;
		}
	}

	// Unknown peer; setup another struct.
	Ref< net::UdpSocket > socket = new net::UdpSocket();
	if (!socket->connect(fromAddr))
		return false;

	Peer p;
	p.socketAddr = fromAddr;
	p.socket = socket;
	m_peers.push_back(p);

	outFromPeer = m_peers.size() - 1;
	return true;
	*/
	return -1;
}

bool LanReplicatorPeers::send(handle_t handle, const void* data, int32_t size, bool reliable)
{
	//return m_peers[peerId].socket->send(data, size) > 0;
	return false;
}

	}
}
