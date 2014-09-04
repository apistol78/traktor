#ifndef traktor_net_INetworkTopology_H
#define traktor_net_INetworkTopology_H

#include "Core/Object.h"
#include "Net/Replication/NetworkTypes.h"

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

class T_DLLCLASS INetworkTopology : public Object
{
	T_RTTI_CLASS;

public:
	struct INetworkCallback
	{
		virtual bool nodeConnected(INetworkTopology* topology, net_handle_t node) = 0;

		virtual bool nodeDisconnected(INetworkTopology* topology, net_handle_t node) = 0;
	};

	virtual void setCallback(INetworkCallback* callback) = 0;

	virtual net_handle_t getLocalHandle() const = 0;

	virtual bool setPrimaryHandle(net_handle_t node) = 0;

	virtual net_handle_t getPrimaryHandle() const = 0;

	virtual int32_t getNodeCount() const = 0;

	virtual net_handle_t getNodeHandle(int32_t index) const = 0;

	virtual std::wstring getNodeName(int32_t index) const = 0;

	virtual bool isNodeRelayed(int32_t index) const = 0;

	virtual bool send(net_handle_t node, const void* data, int32_t size) = 0;

	virtual int32_t recv(void* data, int32_t size, net_handle_t& outNode) = 0;

	virtual bool update(double dT) = 0;
};

	}
}

#endif	// traktor_net_INetworkTopology_H
