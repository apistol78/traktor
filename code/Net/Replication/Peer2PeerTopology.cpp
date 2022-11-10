/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include <cstring>
#include <ctime>
#include "Core/Containers/StaticVector.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
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

struct P2PMessageAny
{
	uint8_t data[MaxDataSize - 1];
};

struct P2PMessageDirect
{
	uint8_t data[MaxDataSize - 1];
};

struct P2PMessageRelay
{
	net_handle_t from;
	net_handle_t target;
	uint8_t data[MaxDataSize - 1 - 8 - 8];
};

struct P2PMessageIAm
{
	uint8_t sequence;
};

struct P2PMessageCMask
{
	net_handle_t of;
	uint8_t sequence;
	net_handle_t connections[(MaxDataSize - 1 - 8 - 1) / 8];
};

struct P2PMessage
{
	uint8_t id;
	union
	{
		P2PMessageDirect direct;
		P2PMessageRelay relay;
		P2PMessageIAm iam;
		P2PMessageCMask cmask;
		uint8_t data[MaxDataSize - 1];
	};
};

#pragma pack()

const uint32_t c_maxPendingIAm = 32;
const int32_t c_maxReceiveMessages = 128;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.Peer2PeerTopology", Peer2PeerTopology, INetworkTopology)

Peer2PeerTopology::Peer2PeerTopology(IPeer2PeerProvider* provider)
:	m_provider(provider)
,	m_callback(nullptr)
,	m_random(std::clock())
,	m_iAmInterval(2.0)
,	m_iAmRandomFlux(0.3)
,	m_propagateInterval(1.0)
,	m_propagateRandomFlux(0.3)
{
}

void Peer2PeerTopology::setIAmInterval(double interval, double flux)
{
	T_FATAL_ASSERT (interval > 0.0);
	m_iAmInterval = interval;
	m_iAmRandomFlux = flux;
}

void Peer2PeerTopology::setPropagateCMaskInterval(double interval, double flux)
{
	T_FATAL_ASSERT (interval > 0.0);
	T_FATAL_ASSERT (flux > 0.0);
	T_FATAL_ASSERT (flux < interval);
	m_propagateInterval = interval;
	m_propagateRandomFlux = flux;
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

Object* Peer2PeerTopology::getNodeUser(int32_t index) const
{
	int32_t peerIndex = m_nodes[index];
	return m_peers[peerIndex].user;
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

		log::info << getLogPrefix() << L"Failed to send data to peer " << nodePeer.send << L" (1)." << Endl;
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

		if (m_provider->send(nodePeer.send, &msg, MsgRelay_NetSize(size)))
			return true;

		log::info << getLogPrefix() << L"Failed to send data to peer " << nodePeer.send << L" (2)." << Endl;
	}

	return false;
}

int32_t Peer2PeerTopology::recv(void* data, int32_t size, net_handle_t& outNode)
{
	while (!m_recvQueue.empty())
	{
		const Recv& r = m_recvQueue.front();

		// Only permit receive from established peers; all
		// other messages are discarded.
		int32_t peerIndex = indexOf(r.from);
		if (peerIndex >= 0)
		{
			Peer& peer = m_peers[peerIndex];
			if (peer.established)
			{
				size = std::min(size, r.size);
				std::memcpy(data, r.data, size);
				outNode = r.from;
				m_recvQueue.pop_front();
				return size;
			}
			else
			{
				log::warning << getLogPrefix() << L"Received data from non-established peer " << r.from << Endl;
				peer.whenIAm = 0.0;
			}
		}
		else
			log::warning << getLogPrefix() << L"Received data from unknown peer " << r.from << Endl;

		m_recvQueue.pop_front();
	}
	return 0;
}

bool Peer2PeerTopology::update(double dT)
{
	P2PMessage msg;
	int32_t updateRouting = 0;

	// Update provider first.
	if (!m_provider->update())
		return false;

	// Get peers from provider.
	int32_t providerPeerCount = m_provider->getPeerCount();
	m_providerPeers.resize(providerPeerCount);
	for (int32_t i = 0; i < providerPeerCount; ++i)
	{
		m_providerPeers[i] = m_provider->getPeerHandle(i);
		if (m_providerPeers[i] == 0)
			log::error << L"Provider peer handle " << i << L" null." << Endl;
	}

	// Add new peers.
	for (int32_t i = 0; i < providerPeerCount; ++i)
	{
		if (indexOf(m_providerPeers[i]) < 0)
		{
			Peer peer;
			peer.handle = m_providerPeers[i];
			peer.name = m_provider->getPeerName(i);
			peer.user = m_provider->getPeerUser(i);
			m_peers.push_back(peer);
			updateRouting = 1;
		}
	}

	int32_t myIndex = indexOf(m_provider->getLocalHandle());
	Peer& myPeer = m_peers[myIndex];

	// Remove peers.
	for (int32_t i = 0; i < int32_t(m_peers.size()); )
	{
		Peer& peer = m_peers[i];

		if (std::find(m_providerPeers.begin(), m_providerPeers.end(), peer.handle) == m_providerPeers.end())
		{
			// Remove from my list of direct connections.
			auto it = myPeer.connections.find(peer.handle);
			if (it != myPeer.connections.end())
			{
				myPeer.connections.erase(it);
				myPeer.sequence++;
				myPeer.whenPropagate = 0.0;
				updateRouting = 2;
			}

			// Issue disconnect callback.
			if (peer.established)
			{
				log::info << getLogPrefix() << L"Peer " << m_peers[i].handle << L" disconnected (no provider)." << Endl;

				if (m_callback)
					m_callback->nodeDisconnected(this, peer.handle);

				peer.established = false;
			}

			m_peers.erase(m_peers.begin() + i);
		}
		else
			++i;
	}

	double time = m_timer.getElapsedTime();

	// Send direct connection handshake messages, send for every new peer first.
	for (int32_t i = 0; i < int32_t(m_peers.size()); ++i)
	{
		if (i == myIndex)
			continue;

		Peer& peer = m_peers[i];
		if (peer.whenIAm > 0.0)
			continue;

		// Keep sending "I am" messages as long as peer exist from provider; ie is in lobby.
		msg.id = MsgIAm_0;
		msg.iam.sequence = 0;

		if (m_provider->send(peer.handle, &msg, MsgIAm_NetSize()))
		{
			peer.sentIAm++;
		}
		else
		{
			log::info << getLogPrefix() << L"Failed to send \"I am\" to peer " << peer.handle << L" (1)." << Endl;

			auto it = myPeer.connections.find(peer.handle);
			if (it != myPeer.connections.end())
			{
				myPeer.connections.erase(it);
				myPeer.whenPropagate = 0.0;
				myPeer.sequence++;

				peer.sequence = 0;
				peer.connections.clear();
				peer.whenIAm = 0.0;
				peer.sentIAm = 0;

				updateRouting = 4;
			}
		}

		peer.whenIAm = time + m_iAmInterval + m_random.nextDouble() * m_iAmRandomFlux;
	}

	// Send direct connection handshake messages, periodically thus only one message per update.
	for (int32_t i = 0; i < int32_t(m_peers.size()); ++i)
	{
		if (i == myIndex)
			continue;

		Peer& peer = m_peers[i];
		if (peer.whenIAm <= 0.0 || time < peer.whenIAm)
			continue;

		// Keep sending "I am" messages as long as peer exist from provider; ie is in lobby.
		msg.id = MsgIAm_0;
		msg.iam.sequence = 0;

		if (m_provider->send(peer.handle, &msg, MsgIAm_NetSize()))
		{
			peer.sentIAm++;
		}
		else
		{
			log::info << getLogPrefix() << L"Failed to send \"I am\" to peer " << peer.handle << L" (2)." << Endl;

			auto it = myPeer.connections.find(peer.handle);
			if (it != myPeer.connections.end())
			{
				myPeer.connections.erase(it);
				myPeer.whenPropagate = 0.0;
				myPeer.sequence++;

				peer.sequence = 0;
				peer.connections.clear();
				peer.whenIAm = 0.0;
				peer.sentIAm = 0;

				updateRouting = 4;
			}
		}

		peer.whenIAm = time + m_iAmInterval + m_random.nextDouble() * m_iAmRandomFlux;

		// Only send one "I am" per update.
		break;
	}

	// Check if any peer doesn't respond to "I am".
	for (int32_t i = 0; i < int32_t(m_peers.size()); ++i)
	{
		if (i == myIndex)
			continue;

		Peer& peer = m_peers[i];

		// Check if peer doesn't respond to "I am" messages first.
		if (peer.sentIAm >= c_maxPendingIAm)
		{
			auto it = myPeer.connections.find(peer.handle);
			if (it != myPeer.connections.end())
			{
				log::info << getLogPrefix() << L"Peer " << peer.handle << L" no longer respond to \"I am\" messages." << Endl;

				myPeer.connections.erase(it);
				myPeer.whenPropagate = 0.0;
				myPeer.sequence++;

				peer.sequence = 0;
				peer.connections.clear();
				peer.whenIAm = 0.0;
				peer.sentIAm = 0;

				updateRouting = 3;
			}
		}
	}

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
			if (otherPeer.connections.count(thisPeer.handle) != 0)
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

	// Propagate connections to my neighbor peers.
	int32_t errors = 0;
	for (int32_t i = 0; i < int32_t(m_peers.size()); ++i)
	{
		Peer& peer = m_peers[i];

		if (time < peer.whenPropagate || peer.connections.empty())
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
			{
				log::info << getLogPrefix() << L"Failed to send connections to peer " << myPeer.connections[j] << L"." << Endl;
				++errors;
			}
		}

		peer.whenPropagate = time + m_propagateInterval + m_random.nextDouble() * m_propagateRandomFlux;
	}

	if (errors > 0)
		log::warning << getLogPrefix() << L"Unable to propagate " << errors << L" connection mask(s)." << Endl;

	// Receive messages.
	{
		net_handle_t from;
		P2PMessage reply;
		int32_t nrecv;
		int32_t i;

		for (i = 0; i < c_maxReceiveMessages; ++i)
		{
			from = 0;

			nrecv = m_provider->recv(&msg, MaxDataSize, from);
			if (nrecv <= 0)
				break;
			if (from == 0)
				continue;

			if (msg.id == MsgIAm_0)
			{
				reply.id = MsgIAm_1;
				reply.iam.sequence = msg.iam.sequence;
				m_provider->send(from, &reply, MsgIAm_NetSize());
			}
			else if (msg.id == MsgIAm_1)
			{
				int32_t peerIndex = indexOf(from);
				if (peerIndex >= 0)
				{
					Peer& peer = m_peers[peerIndex];
					if (peer.sentIAm > 0)
					{
						if (myPeer.connections.count(from) == 0)
						{
							myPeer.connections.insert(from);
							myPeer.whenPropagate = 0.0;
							myPeer.sequence++;
							updateRouting = 7;
						}
						peer.sentIAm = 0;
					}
#if defined(_DEBUG)
					else
						log::warning << getLogPrefix() << L"Received \"I am\" response from " << from << L" but none sent, ignored." << Endl;
#endif
				}
			}
			else if (msg.id == MsgCMask)
			{
				T_ASSERT(msg.cmask.of != myPeer.handle);

				int32_t ofPeerIndex = indexOf(msg.cmask.of);
				if (ofPeerIndex >= 0 && ofPeerIndex < int32_t(m_peers.size()))
				{
					Peer& ofPeer = m_peers[ofPeerIndex];
					if (msg.cmask.sequence >= ofPeer.sequence)
					{
						int32_t nconnections = MsgCMask_Connections(nrecv);

						StaticSet< net_handle_t, MaxPeers > connections;
						for (int32_t j = 0; j < nconnections; ++j)
							connections.insert(msg.cmask.connections[j]);

						bool equal = bool(connections == ofPeer.connections);
						if (!equal)
						{
							ofPeer.whenPropagate = 0.0;
							ofPeer.connections = connections;
							updateRouting = 8;
						}

						if (msg.cmask.sequence > ofPeer.sequence)
							ofPeer.whenPropagate = 0.0;

						ofPeer.sequence = msg.cmask.sequence;
					}
				}
			}
			else if (msg.id == MsgDirect)
			{
				// Received a direct message to myself.
				if (m_recvQueue.full())
					log::warning << L"Receive queue full; oldest message discarded (1)." << Endl;

				Recv& r = m_recvQueue.push_back();
				r.from = from;
				r.size = MsgDirect_DataSize(nrecv);
				std::memcpy(r.data, msg.direct.data, MsgDirect_DataSize(nrecv));
			}
			else if (msg.id == MsgRelay)
			{
				if (msg.relay.target == myPeer.handle)
				{
					// Received a relayed message to myself.
					if (m_recvQueue.full())
						log::warning << L"Receive queue full; oldest message discarded (2)." << Endl;

					Recv& r = m_recvQueue.push_back();
					r.from = msg.relay.from;
					r.size = MsgRelay_DataSize(nrecv);
					std::memcpy(r.data, msg.relay.data, MsgRelay_DataSize(nrecv));
				}
				else
				{
					// Received a relayed message for someone else; send further.
					int32_t targetIndex = indexOf(msg.relay.target);
					if (targetIndex >= 0 && m_peers[targetIndex].send != 0)
					{
						T_ASSERT(targetIndex != myIndex);
						bool result = m_provider->send(m_peers[targetIndex].send, &msg, nrecv);
						if (!result)
							log::info << getLogPrefix() << L"Unable to relay message to peer " << msg.relay.target << L" through " << m_peers[targetIndex].send << L"; message discarded." << Endl;
					}
				}
			}
		}
	}

#if defined(_DEBUG)
	// Update local routing information.
	if (updateRouting)
		log::info << getLogPrefix() << L"Updating optimal routes (" << updateRouting << L")..." << Endl;
#endif

	m_nodes.resize(0);
	for (int32_t i = 0; i < int32_t(m_peers.size()); ++i)
	{
		if (i != myIndex)
		{
			if (updateRouting)
			{
				if (!findOptimalRoute(myPeer.handle, m_peers[i].handle, m_peers[i].send))
					m_peers[i].send = 0;

				if (!findOptimalRoute(m_peers[i].handle, myPeer.handle, m_peers[i].reverseSend))
					m_peers[i].reverseSend = 0;
			}

			if (m_peers[i].send != 0 && m_peers[i].reverseSend != 0)
			{
				m_nodes.push_back(i);

				if (!m_peers[i].established)
				{
					log::info << getLogPrefix() << L"Peer " << m_peers[i].handle << L" connected (found route) [" << updateRouting << L"]." << Endl;

					if (m_callback)
						m_callback->nodeConnected(this, m_peers[i].handle);

					m_peers[i].established = true;
				}
			}
			else
			{
				if (m_peers[i].established)
				{
					log::info << getLogPrefix() << L"Peer " << m_peers[i].handle << L" disconnected (no route) [" << updateRouting << L"]." << Endl;

					if (m_callback)
						m_callback->nodeDisconnected(this, m_peers[i].handle);

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
				log::info << getLogPrefix() << L"Peer " << m_peers[i].handle << L" connected (local)." << Endl;

				if (m_callback)
					m_callback->nodeConnected(this, m_peers[i].handle);

				m_peers[i].established = true;
			}
		}
	}

	return true;
}

bool Peer2PeerTopology::findOptimalRoute(net_handle_t from, net_handle_t to, net_handle_t& outNext) const
{
	if (from == to)
	{
		log::warning << getLogPrefix() << L"findOptimalRoute; no route, same source and target." << Endl;
		return false;
	}

	int32_t fromPeerId = indexOf(from);
	if (fromPeerId < 0)
	{
		log::warning << getLogPrefix() << L"findOptimalRoute; no route, unknown source." << Endl;
		return false;
	}

	int32_t toPeerId = indexOf(to);
	if (toPeerId < 0)
	{
		log::warning << getLogPrefix() << L"findOptimalRoute; no route, unknown target." << Endl;
		return false;
	}

	// Check if "from" can send directly to "to".
	const Peer& fromPeer = m_peers[fromPeerId];
	if (fromPeer.connections.count(to) != 0)
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
		if (checkPeer.connections.count(to) != 0)
		{
			throughPeerId = id;
			break;
		}
	}
	if (throughPeerId < 0)
	{
		log::warning << getLogPrefix() << L"findOptimalRoute; no route, no re-direct peer found." << Endl;
		return false;
	}

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
