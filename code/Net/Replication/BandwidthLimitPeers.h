#ifndef traktor_net_BandwidthLimitPeers_H
#define traktor_net_BandwidthLimitPeers_H

#include "Core/Timer/Timer.h"
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

class T_DLLCLASS BandwidthLimitPeers : public IReplicatorPeers
{
	T_RTTI_CLASS;

public:
	BandwidthLimitPeers(IReplicatorPeers* peers, int32_t maxSendRate, bool silentDiscard, bool alwaysSendReliable);

	virtual ~BandwidthLimitPeers();

	virtual void destroy();

	virtual bool update();

	virtual void setStatus(uint8_t status);

	virtual void setConnectionState(uint64_t connectionState);

	virtual handle_t getHandle() const;

	virtual std::wstring getName() const;

	virtual handle_t getPrimaryPeerHandle() const;

	virtual bool setPrimaryPeerHandle(handle_t handle);

	virtual uint32_t getPeers(std::vector< PeerInfo >& outPeers) const;

	virtual int32_t receive(void* data, int32_t size, handle_t& outFromHandle);

	virtual bool send(handle_t handle, const void* data, int32_t size, bool reliable);

private:
	Timer m_timer;
	Ref< IReplicatorPeers > m_peers;
	int32_t m_maxSendRate;
	bool m_silentDiscard;
	bool m_alwaysSendReliable;
	double m_lastTime;
	int32_t m_sent;
	int32_t m_currentRate;
};

	}
}

#endif	// traktor_net_BandwidthLimitPeers_H
