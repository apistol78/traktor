#ifndef traktor_net_MeasureP2PProvider_H
#define traktor_net_MeasureP2PProvider_H

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

class T_DLLCLASS MeasureP2PProvider : public IPeer2PeerProvider
{
	T_RTTI_CLASS;

public:
	MeasureP2PProvider(IPeer2PeerProvider* provider);

	virtual bool update();

	virtual net_handle_t getLocalHandle() const;

	virtual int32_t getPeerCount() const;

	virtual net_handle_t getPeerHandle(int32_t index) const;

	virtual std::wstring getPeerName(int32_t index) const;

	virtual Object* getPeerUser(int32_t index) const;

	virtual bool setPrimaryPeerHandle(net_handle_t node);

	virtual net_handle_t getPrimaryPeerHandle() const;

	virtual bool send(net_handle_t node, const void* data, int32_t size);

	virtual int32_t recv(void* data, int32_t size, net_handle_t& outNode);

	float getSendBitsPerSecond() const;

	float getRecvBitsPerSecond() const;

private:
	Ref< IPeer2PeerProvider > m_provider;
	double m_time;
	int32_t m_sentBytes;
	int32_t m_recvBytes;
	double m_sentBps;
	double m_recvBps;
};

	}
}

#endif	// traktor_net_MeasureP2PProvider_H
