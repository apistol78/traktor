#include <cstring>
#include <ctime>
#include "Core/Containers/StaticVector.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Timer/Measure.h"
#include "Core/Timer/Timer.h"
#include "Net/Replication/Peer2PeerTopology.h"

namespace traktor
{
	namespace net
	{
		namespace
		{

enum P2PMessageId
{
	MsgDirect	= 0x00,
	MsgRelay	= 0x01,
	MsgIAm_0	= 0x10,
	MsgIAm_1	= 0x11,
	MsgCMask	= 0x20
};

#define MsgDirect_NetSize(dataSize)		( sizeof(uint8_t) + dataSize )
#define MsgDirect_DataSize(netSize)		( netSize - sizeof(uint8_t) )
#define MsgRelay_NetSize(dataSize)		( sizeof(uint8_t) + sizeof(net_handle_t) + sizeof(net_handle_t) + dataSize )
#define MsgRelay_DataSize(netSize)		( netSize - sizeof(uint8_t) - sizeof(net_handle_t) - sizeof(net_handle_t) )
#define MsgIAm_NetSize()				( sizeof(uint8_t) + sizeof(uint8_t) )
#define MsgCMask_NetSize(connections)	( sizeof(uint8_t) + sizeof(net_handle_t) + sizeof(uint8_t) + sizeof(net_handle_t) * connections )
#define MsgCMask_Connections(netSize)	( (netSize - (sizeof(uint8_t) + sizeof(net_handle_t) + sizeof(uint8_t))) / sizeof(net_handle_t) )

#pragma pack(1)
struct P2PMessage
{
	uint8_t id;
	union
	{
		struct
		{
			uint8_t data[1];
		}
		direct;

		struct
		{
			net_handle_t from;
			net_handle_t target;
			uint8_t data[1];
		}
		relay;

		struct 
		{
			uint8_t sequence;
		}
		iam;

		struct
		{
			net_handle_t of;
			uint8_t sequence;
			net_handle_t connections[1];
		}
		cmask;

		uint8_t data[MaxDataSize - 1];
	};
};
#pragma pack()

const double c_IAmInterval = 1.0;
const uint32_t c_maxPendingIAm = 16;
const double c_propagateInterval = 4.0;
const double c_timeRandomFlux = 0.5;
const int32_t c_maxReceiveMessages = 128;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.Peer2PeerTopology", Peer2PeerTopology, INetworkTopology)

Peer2PeerTopology::Peer2PeerTopology(IPeer2PeerProvider* provider)
:	m_provider(provider)
,	m_callback(0)
#if !defined(WINCE)
,	m_random(std::clock())
#endif
,	m_time(0.0)
{
}

void Peer2PeerTopology::setCallback(INetworkCallback* callback)
{
	m_callback = callback;
}

net_handle_t Peer2PeerTopology::getLocalHandle() const
{
	return m_provider->getLocalHandle();
}

bool Peer2PeerTopology::setPrimaryHandle(net_handle_t node)
{
	return m_provider->setPrimaryPeerHandle(node);
}

net_handle_t Peer2PeerTopology::getPrimaryHandle() const
{
	return m_provider->getPrimaryPeerHandle();
}

int32_t Peer2PeerTopology::getNodeCount() const
{
	return int32_t(m_nodes.size());
}

net_handle_t Peer2PeerTopology::getNodeHandle(int32_t index) const
{
	int32_t peerIndex = m_nodes[index];
	return m_peers[peerIndex].handle;
}

std::wstring Peer2PeerTopology::getNodeName(int32_t index) const
{
	int32_t peerIndex = m_nodes[index];
	return m_peers[peerIndex].name;
}

bool Peer2PeerTopology::isNodeRelayed(int32_t index) const
{
	int32_t peerIndex = m_nodes[index];
	return m_peers[peerIndex].handle != m_peers[peerIndex].send;
}

bool Peer2PeerTopology::send(net_handle_t node, const void* data, int32_t size)
{
	int32_t nodeIndex = indexOf(node);
	if (nodeIndex < 0)
		return false;

	Peer& nodePeer = m_peers[nodeIndex];

	if (nodePeer.send == nodePeer.handle)
	{
		// Direct connection is established; send directly to target peer.
		P2PMessage msg;
		std::memset(&msg, 0, sizeof(msg));

		msg.id = MsgDirect;
		std::memcpy(msg.direct.data, data, size);

		if (m_provider->send(nodePeer.send, &msg, MsgDirect_NetSize(size)))
			return true;
	}
	else
	{
		// No direct connection is established; relay message across peers.
		P2PMessage msg;
		std::memset(&msg, 0, sizeof(msg));

		msg.id = MsgRelay;
		msg.relay.from = m_provider->getLocalHandle();
		msg.relay.target = node;
		std::memcpy(msg.relay.data, data, size);

		if (!m_provider->send(nodePeer.send, &msg, MsgRelay_NetSize(size)))
			return false;
	}

	return true;
}

int32_t Peer2PeerTopology::recv(void* data, int32_t size, net_handle_t& outNode)
{
	if (m_recvQueue.empty())
		return 0;

	const Recv& r = m_recvQueue.front();

	size = std::min(size, r.size);
	std::memcpy(data, r.data, size);

	outNode = r.from;

	m_recvQueue.pop_front();
	return size;
}

bool Peer2PeerTopology::update(double dT)
{
	P2PMessage msg;
	int32_t updateRouting = 0;

	T_MEASURE_BEGIN();

	// Update provider first.
	if (!m_provider->update())
		return false;

	T_MEASURE_UNTIL(0.002);

	// Get peers from provider.
	int32_t providerPeerCount = m_provider->getPeerCount();
	m_providerPeers.resize(providerPeerCount);
	for (int32_t i = 0; i < providerPeerCount; ++i)
		m_providerPeers[i] = m_provider->getPeerHandle(i);

	T_MEASURE_UNTIL(0.00025);

	// Add new peers.
	for (int32_t i = 0; i < providerPeerCount; ++i)
	{
		if (indexOf(m_providerPeers[i]) < 0)
		{
			Peer peer;
			peer.handle = m_providerPeers[i];
			peer.name = m_provider->getPeerName(i);
			m_peers.push_back(peer);
			updateRouting = 1;
		}
	}

	T_MEASURE_UNTIL(0.00025);

	int32_t myIndex = indexOf(m_provider->getLocalHandle());
	Peer& myPeer = m_peers[myIndex];

	// Remove peers.
	for (int32_t i = 0; i < int32_t(m_peers.size()); ++i)
	{
		Peer& peer = m_peers[i];

		if (std::find(m_providerPeers.begin(), m_providerPeers.end(), peer.handle) == m_providerPeers.end())
		{
			std::vector< net_handle_t >::iterator it = std::find(myPeer.connections.begin(), myPeer.connections.end(), peer.handle);
			if (it != myPeer.connections.end())
			{
				myPeer.connections.erase(it);
				myPeer.sequence++;
				myPeer.whenPropagate = m_time;
				updateRouting = 2;
			}

			if (!peer.connections.empty())
				updateRouting = 2;

			peer.sequence = 0;
			peer.connections.clear();
			peer.whenIAm = 0.0;
			peer.sentIAm = 0;
		}
	}

	T_MEASURE_UNTIL(0.00025);

	// Send direct connection handshake messages.
	for (int32_t i = 0; i < int32_t(m_peers.size()); ++i)
	{
		if (i == myIndex)
			continue;

		Peer& peer = m_peers[i];

		if (m_time < peer.whenIAm)
			continue;

		// Check if peer doesn't respond to "I am" messages first.
		if (peer.sentIAm >= c_maxPendingIAm)
		{
			std::vector< net_handle_t >::iterator it = std::find(myPeer.connections.begin(), myPeer.connections.end(), peer.handle);
			if (it != myPeer.connections.end())
			{
				log::info << getLogPrefix() << L"Peer " << peer.handle << L" no longer respond to \"I am\" messages." << Endl;

				myPeer.connections.erase(it);
				myPeer.whenPropagate = m_time;
				myPeer.sequence++;

				peer.sequence = 0;
				peer.connections.clear();
				peer.whenIAm = 0.0;
				peer.sentIAm = 0;

				updateRouting = 3;
			}
		}

		// Keep sending "I am" messages as long as peer exist from provider; ie is in lobby.
		msg.id = MsgIAm_0;
		msg.iam.sequence = 0;

		if (m_provider->send(peer.handle, &msg, MsgIAm_NetSize()))
		{
			peer.sentIAm++;
		}
		else
		{
			std::vector< net_handle_t >::iterator it = std::find(myPeer.connections.begin(), myPeer.connections.end(), peer.handle);
			if (it != myPeer.connections.end())
			{
				myPeer.connections.erase(it);
				myPeer.whenPropagate = m_time;
				myPeer.sequence++;

				peer.sequence = 0;
				peer.connections.clear();
				peer.whenIAm = 0.0;
				peer.sentIAm = 0;

				updateRouting = 4;
			}
		}

		peer.whenIAm = m_time + c_IAmInterval + m_random.nextDouble() * c_timeRandomFlux;
	}

	T_MEASURE_UNTIL(0.001);

	// If I am alone then clear every other peer.
	if (myPeer.connections.empty())
	{
		for (int32_t i = 0; i < int32_t(m_peers.size()); ++i)
		{
			if (i != myIndex && !m_peers[i].connections.empty())
			{
				m_peers[i].sequence = 0;
				m_peers[i].connections.clear();
				m_peers[i].whenIAm = 0.0;
				m_peers[i].sentIAm = 0;
				updateRouting = 5;
			}
		}
		myPeer.sentIAm = 0;
	}

	T_MEASURE_UNTIL(0.0001);

	// Non two-way connected peers.
	for (int32_t i = 0; i < int32_t(m_peers.size()); ++i)
	{
		if (i == myIndex)
			continue;

		Peer& thisPeer = m_peers[i];

		if (thisPeer.connections.empty())
			continue;

		bool thisReachable = false;

		for (int32_t j = 0; j < int32_t(m_peers.size()); ++j)
		{
			if (i == j)
				continue;

			const Peer& otherPeer = m_peers[j];
			if (std::find(otherPeer.connections.begin(), otherPeer.connections.end(), thisPeer.handle) != otherPeer.connections.end())
			{
				thisReachable = true;
				break;
			}
		}

		if (!thisReachable)
		{
			thisPeer.sequence = 0;
			thisPeer.connections.clear();
			thisPeer.whenIAm = 0.0;
			thisPeer.sentIAm = 0;

			updateRouting = 6;
		}
	}

	T_MEASURE_UNTIL(0.0001);

	// Propagate connections to my neighbor peers.
	int32_t errors = 0;
	for (int32_t i = 0; i < int32_t(m_peers.size()); ++i)
	{
		Peer& peer = m_peers[i];

		if (m_time < peer.whenPropagate || peer.connections.empty())
			continue;

		msg.id = MsgCMask;
		msg.cmask.of = peer.handle;
		msg.cmask.sequence = peer.sequence;

		for (int32_t j = 0; j < int32_t(peer.connections.size()); ++j)
			msg.cmask.connections[j] = peer.connections[j];

		for (int32_t j = 0; j < int32_t(myPeer.connections.size()); ++j)
		{
			if (myPeer.connections[j] == 0 || myPeer.connections[j] == peer.handle)
				continue;

			if (!m_provider->send(myPeer.connections[j], &msg, MsgCMask_NetSize(peer.connections.size())))
				++errors;
		}

		peer.whenPropagate = m_time + c_propagateInterval + m_random.nextDouble() * c_timeRandomFlux;
	}

	if (errors > 0)
		log::warning << getLogPrefix() << L"Unable to propagate " << errors << L" connection mask(s)." << Endl;

	T_MEASURE_UNTIL(0.001);

	// Receive messages.
	{
		net_handle_t from;
		P2PMessage reply;
		int32_t nrecv;
		int32_t i;

		for (i = 0; i < c_maxReceiveMessages; ++i)
		{
			T_MEASURE_STATEMENT(nrecv = m_provider->recv(&msg, MaxDataSize, from), 0.001);
			if (nrecv <= 0)
				break;

			if (msg.id == MsgIAm_0)
			{
				reply.id = MsgIAm_1;
				reply.iam.sequence = msg.iam.sequence;
				T_MEASURE_STATEMENT(m_provider->send(from, &reply, MsgIAm_NetSize()), 0.001);
			}
			else if (msg.id == MsgIAm_1)
			{
				int32_t peerIndex = indexOf(from);
				if (peerIndex >= 0)
				{
					Peer& peer = m_peers[peerIndex];
				
					peer.sentIAm = 0;

					std::vector< net_handle_t >::iterator it = std::find(myPeer.connections.begin(), myPeer.connections.end(), from);
					if (it == myPeer.connections.end())
					{
						myPeer.connections.push_back(from);
						std::sort(myPeer.connections.begin(), myPeer.connections.end());

						myPeer.whenPropagate = m_time;
						myPeer.sequence++;

						updateRouting = 7;
					}
				}
			}
			else if (msg.id == MsgCMask)
			{
				T_ASSERT (msg.cmask.of != myPeer.handle);

				int32_t ofPeerIndex = indexOf(msg.cmask.of);
				if (ofPeerIndex >= 0 && ofPeerIndex < int32_t(m_peers.size()))
				{
					Peer& ofPeer = m_peers[ofPeerIndex];
					if (msg.cmask.sequence >= ofPeer.sequence)
					{
						int32_t nconnections = MsgCMask_Connections(nrecv);

						std::vector< net_handle_t > connections(nconnections);
						for (int32_t j = 0; j < nconnections; ++j)
							connections[j] = msg.cmask.connections[j];

						std::sort(connections.begin(), connections.end());

						bool equal = false;
						if (connections.size() == ofPeer.connections.size())
							equal = std::equal(connections.begin(), connections.end(), ofPeer.connections.begin());

						if (!equal)
							ofPeer.connections = connections;

						if (!equal || msg.cmask.sequence > ofPeer.sequence)
						{
							ofPeer.whenPropagate = m_time;
							if (!equal)
								updateRouting = 8;
						}

						ofPeer.sequence = msg.cmask.sequence;
					}
				}
			}
			else if (msg.id == MsgDirect)
			{
				// Received a direct message to myself.
				Recv& r = m_recvQueue.push_back();
				r.from = from;
				std::memcpy(r.data, msg.direct.data, MsgDirect_DataSize(nrecv));
				r.size = MsgDirect_DataSize(nrecv);
			}
			else if (msg.id == MsgRelay)
			{
				if (msg.relay.target == myPeer.handle)
				{
					// Received a relayed message to myself.
					Recv& r = m_recvQueue.push_back();
					r.from = msg.relay.from;
					std::memcpy(r.data, msg.relay.data, MsgRelay_DataSize(nrecv));
					r.size = MsgRelay_DataSize(nrecv);
				}
				else
				{
					// Received a relayed message for someone else; send further.
					int32_t targetIndex = indexOf(msg.relay.target);
					if (targetIndex >= 0 && m_peers[targetIndex].send != 0)
					{
						T_ASSERT (targetIndex != myIndex);
						bool result;
						T_MEASURE_STATEMENT(result = m_provider->send(m_peers[targetIndex].send, &msg, nrecv), 0.001);
						if (!result)
							log::info << getLogPrefix() << L"Unable to relay message to peer " << msg.relay.target << L" through " << m_peers[targetIndex].send << L"; message discarded." << Endl;
					}
				}
			}
		}
	}

	T_MEASURE_UNTIL(0.010);

#if defined(_DEBUG)
	// Update local routing information.
	if (updateRouting)
		log::info << getLogPrefix() << L"Updating optimal routes (" << updateRouting << L")..." << Endl;
#endif

	m_nodes.resize(0);
	m_nodes.reserve(m_peers.size());
	for (int32_t i = 0; i < int32_t(m_peers.size()); ++i)
	{
		if (i != myIndex)
		{
			if (updateRouting)
			{
				bool result = false;
				T_MEASURE_STATEMENT(result = findOptimalRoute(myPeer.handle, m_peers[i].handle, m_peers[i].send), 0.001);
				if (!result)
					m_peers[i].send = 0;
			}

			if (m_peers[i].send != 0)
			{
				m_nodes.push_back(i);

				if (!m_peers[i].established)
				{
					log::info << getLogPrefix() << L"Peer " << m_peers[i].handle << L" connected." << Endl;

					if (m_callback)
						T_MEASURE_STATEMENT(m_callback->nodeConnected(this, m_peers[i].handle), 0.001);

					m_peers[i].established = true;
				}
			}
			else
			{
				if (m_peers[i].established)
				{
					log::info << getLogPrefix() << L"Peer " << m_peers[i].handle << L" disconnected." << Endl;

					if (m_callback)
						T_MEASURE_STATEMENT(m_callback->nodeDisconnected(this, m_peers[i].handle), 0.001);

					m_peers[i].established = false;
				}
			}
		}
		else
		{
			m_peers[i].send = m_peers[i].handle;
			m_nodes.push_back(i);

			if (!m_peers[i].established)
			{
				log::info << getLogPrefix() << L"Peer " << m_peers[i].handle << L" connected." << Endl;

				if (m_callback)
					T_MEASURE_STATEMENT(m_callback->nodeConnected(this, m_peers[i].handle), 0.001);

				m_peers[i].established = true;
			}
		}
	}

	T_MEASURE_UNTIL(0.001);

	m_time += dT;
	return true;
}

bool Peer2PeerTopology::findOptimalRoute(net_handle_t from, net_handle_t to, net_handle_t& outNext) const
{
	if (from == to)
		return false;

	int32_t fromPeerId = indexOf(from);
	if (fromPeerId < 0)
		return false;

	int32_t toPeerId = indexOf(to);
	if (toPeerId < 0)
		return false;

	// Check if "from" can send directly to "to".
	const Peer& fromPeer = m_peers[fromPeerId];
	if (std::find(fromPeer.connections.begin(), fromPeer.connections.end(), to) != fromPeer.connections.end())
	{
		outNext = to;
		return true;
	}

	// Check if anyone "from" knows can send to "to".
	int32_t throughPeerId = -1;
	for (int32_t i = 0; i < fromPeer.connections.size(); ++i)
	{
		net_handle_t throughPeer = fromPeer.connections[i];
		if (throughPeer == from || throughPeer == to)
			continue;

		int32_t id = indexOf(throughPeer);
		if (id < 0)
			continue;

		const Peer& checkPeer = m_peers[id];
		if (std::find(checkPeer.connections.begin(), checkPeer.connections.end(), to) != checkPeer.connections.end())
		{
			throughPeerId = id;
			break;
		}
	}
	if (throughPeerId < 0)
		return false;

	outNext = m_peers[throughPeerId].handle;
	return true;
}

int32_t Peer2PeerTopology::indexOf(net_handle_t handle) const
{
	for (int32_t i = 0; i < int32_t(m_peers.size()); ++i)
	{
		if (m_peers[i].handle == handle)
			return i;
	}
	return -1;
}

std::wstring Peer2PeerTopology::getLogPrefix() const
{
	return L"P2P: [" + toString(m_provider->getLocalHandle()) + L"] ";
}

	}
}
