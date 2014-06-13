#ifndef traktor_net_RecordPeers_H
#define traktor_net_RecordPeers_H

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

class IStream;

	namespace net
	{

class T_DLLCLASS RecordPeers : public IReplicatorPeers
{
	T_RTTI_CLASS;

public:
	RecordPeers(IReplicatorPeers* peers, IStream* stream);

	virtual ~RecordPeers();

	virtual void destroy();

	virtual bool update();

	virtual handle_t getHandle() const;

	virtual std::wstring getName() const;

	virtual handle_t getPrimaryPeerHandle() const;

	virtual bool setPrimaryPeerHandle(handle_t handle);

	virtual uint32_t getPeers(std::vector< PeerInfo >& outPeers) const;

	virtual int32_t receive(void* data, int32_t size, handle_t& outFromHandle);

	virtual bool send(handle_t handle, const void* data, int32_t size, bool reliable);

private:
	struct Entry 
	{
		float time;
		uint8_t direction;
		handle_t handle;
		int32_t size;
		uint8_t data[1500];
	};

	Ref< IReplicatorPeers > m_peers;
	Ref< IStream > m_stream;
	Timer m_timer;
	handle_t m_handle;
	std::wstring m_name;
	std::vector< PeerInfo > m_info;
};

	}
}

#endif	// traktor_net_RecordPeers_H
