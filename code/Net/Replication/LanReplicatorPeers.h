#ifndef traktor_net_LanReplicatorPeers_H
#define traktor_net_LanReplicatorPeers_H

#include "Net/SocketAddressIPv4.h"
#include "Net/Replication/IReplicatorPeers.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_NET_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace net
	{

class DiscoveryManager;
class UdpSocket;

	}

	namespace net
	{

class T_DLLCLASS LanReplicatorPeers : public IReplicatorPeers
{
	T_RTTI_CLASS;

public:
	LanReplicatorPeers();

	bool create();

	virtual void destroy();

	virtual int32_t update();
	
	virtual std::wstring getName() const;

	virtual uint64_t getGlobalId() const;

	virtual handle_t getPrimaryPeerHandle() const;

	virtual uint32_t getPeerHandles(std::vector< handle_t >& outPeerHandles) const;

	virtual std::wstring getPeerName(handle_t handle) const;

	virtual uint64_t getPeerGlobalId(handle_t handle) const;

	virtual int32_t receive(void* data, int32_t size, handle_t& outFromHandle);

	virtual bool send(handle_t handle, const void* data, int32_t size, bool reliable);

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

#endif	// traktor_net_LanReplicatorPeers_H
