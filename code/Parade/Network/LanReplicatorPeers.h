#ifndef traktor_parade_LanReplicatorPeers_H
#define traktor_parade_LanReplicatorPeers_H

#include "Net/SocketAddressIPv4.h"
#include "Parade/Network/IReplicatorPeers.h"

namespace traktor
{
	namespace net
	{

class DiscoveryManager;
class UdpSocket;

	}

	namespace parade
	{

class LanReplicatorPeers : public IReplicatorPeers
{
	T_RTTI_CLASS;

public:
	LanReplicatorPeers();

	bool create();

	virtual void destroy();

	virtual uint32_t getPeerCount() const;

	virtual bool receiveAnyPending();

	virtual bool receive(void* data, uint32_t size, uint32_t& outFromPeer);

	virtual bool sendReady(uint32_t peerId);

	virtual bool send(uint32_t peerId, const void* data, uint32_t size, bool reliable);

private:
	struct Peer
	{
		net::SocketAddressIPv4 socketAddr;
		Ref< net::UdpSocket > socket;
	};

	Ref< net::DiscoveryManager > m_discoveryManager;
	net::SocketAddressIPv4 m_socketAddr;
	Ref< net::UdpSocket > m_socket;
	uint32_t m_networkId;
	std::vector< Peer > m_peers;
};

	}
}

#endif	// traktor_parade_LanReplicatorPeers_H
