#ifndef traktor_net_DiagnosePeers_H
#define traktor_net_DiagnosePeers_H

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

class T_DLLCLASS DiagnosePeers : public IReplicatorPeers
{
	T_RTTI_CLASS;

public:
	DiagnosePeers(IReplicatorPeers* peers);

	virtual ~DiagnosePeers();

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
	Ref< IReplicatorPeers > m_peers;
	Timer m_timer;
	double m_lastT;
	uint32_t m_sent;
	uint32_t m_received;
};

	}
}

#endif	// traktor_net_DiagnosePeers_H
