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
	Peer2PeerTopology(IPeer2PeerProvider* provider);

	virtual void setCallback(INetworkCallback* callback);

	virtual net_handle_t getLocalHandle() const;

	virtual bool setPrimaryHandle(net_handle_t node);

	virtual net_handle_t getPrimaryHandle() const;

	virtual int32_t getNodeCount() const;

	virtual net_handle_t getNodeHandle(int32_t index) const;

	virtual std::wstring getNodeName(int32_t index) const;

	virtual bool send(net_handle_t node, const void* data, int32_t size);

	virtual int32_t recv(void* data, int32_t size, net_handle_t& outNode);

	virtual bool update(double dT);

private:
	struct Peer
	{
		net_handle_t handle;
		net_handle_t send;
		std::wstring name;
		bool established;
		uint8_t sequence;
		std::vector< net_handle_t > connections;
		double whenIAm;
		uint32_t sentIAm;

		Peer()
		:	handle(0)
		,	send(0)
		,	established(false)
		,	sequence(0)
		,	whenIAm(0.0)
		,	sentIAm(0)
		{
		}
	};

	struct Recv
	{
		net_handle_t from;
		uint8_t data[MaxDataSize];
		int32_t size;
	};

	Ref< IPeer2PeerProvider > m_provider;
	INetworkCallback* m_callback;
	Random m_random;
	double m_time;
	std::vector< Peer > m_peers;
	std::vector< int32_t > m_nodes;
	CircularVector< Recv, 128 > m_recvQueue;
	double m_whenPropagate;

	bool findOptimalRoute(net_handle_t from, net_handle_t to, net_handle_t& outNext) const;

	void traverseRoute(net_handle_t from, net_handle_t to, const std::vector< net_handle_t >& chain, std::vector< net_handle_t >& outChain) const;

	int32_t indexOf(net_handle_t handle) const;

	std::wstring getLogPrefix() const;
};

	}
}

#endif	// traktor_net_Peer2PeerTopology_H
