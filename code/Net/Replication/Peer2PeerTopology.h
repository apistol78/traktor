#ifndef traktor_net_Peer2PeerTopology_H
#define traktor_net_Peer2PeerTopology_H

#include <map>
#include <list>
#include <vector>
#include "Core/Containers/CircularVector.h"
#include "Core/Math/Random.h"
#include "Net/Replication/INetworkTopology.h"
#include "Net/Replication/IPeer2PeerProvider.h"

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

class T_DLLCLASS Peer2PeerTopology : public INetworkTopology
{
	T_RTTI_CLASS;

public:
	struct Peer
	{
		net_handle_t handle;
		net_handle_t send;
		std::wstring name;
		Ref< Object > user;
		bool established;
		uint8_t sequence;
		std::vector< net_handle_t > connections;
		double whenIAm;
		uint32_t sentIAm;
		double whenPropagate;

		Peer()
		:	handle(0)
		,	send(0)
		,	established(false)
		,	sequence(0)
		,	whenIAm(0.0)
		,	sentIAm(0)
		,	whenPropagate(0.0)
		{
		}
	};

	Peer2PeerTopology(IPeer2PeerProvider* provider);

	virtual void setCallback(INetworkCallback* callback);

	virtual net_handle_t getLocalHandle() const;

	virtual bool setPrimaryHandle(net_handle_t node);

	virtual net_handle_t getPrimaryHandle() const;

	virtual int32_t getNodeCount() const;

	virtual net_handle_t getNodeHandle(int32_t index) const;

	virtual std::wstring getNodeName(int32_t index) const;

	virtual Object* getNodeUser(int32_t index) const;

	virtual bool isNodeRelayed(int32_t index) const;

	virtual bool send(net_handle_t node, const void* data, int32_t size);

	virtual int32_t recv(void* data, int32_t size, net_handle_t& outNode);

	virtual bool update(double dT);

	const std::vector< Peer >& getPeers() const { return m_peers; }

private:
	struct Recv
	{
		net_handle_t from;
		int32_t size;
		uint8_t data[MaxDataSize];
	};

	Ref< IPeer2PeerProvider > m_provider;
	std::vector< net_handle_t > m_providerPeers;
	INetworkCallback* m_callback;
	Random m_random;
	double m_time;
	std::vector< Peer > m_peers;
	std::vector< int32_t > m_nodes;
	CircularVector< Recv, 512 > m_recvQueue;

	bool findOptimalRoute(net_handle_t from, net_handle_t to, net_handle_t& outNext) const;

	int32_t indexOf(net_handle_t handle) const;

	std::wstring getLogPrefix() const;
};

	}
}

#endif	// traktor_net_Peer2PeerTopology_H
