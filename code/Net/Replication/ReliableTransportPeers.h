#ifndef traktor_net_ReliableTransportPeers_H
#define traktor_net_ReliableTransportPeers_H

#include <list>
#include <map>
#include "Core/Containers/CircularVector.h"
#include "Core/Timer//Timer.h"
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

class T_DLLCLASS ReliableTransportPeers : public IReplicatorPeers
{
	T_RTTI_CLASS;

public:
	ReliableTransportPeers(IReplicatorPeers* peers);

	virtual ~ReliableTransportPeers();

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
	enum EnvelopeType
	{
		EtUnreliable = 0x01,
		EtReliable = 0x02,
		EtAck = 0x03
	};

#pragma pack(1)
	struct Envelope
	{
		uint8_t type;
		uint8_t sequence;
		uint8_t payload[510];
	};
#pragma pack()

	struct ControlEnvelope
	{
		double time0;
		double time;
		bool resent;
		uint32_t size;
		Envelope envelope;
	};

	struct Control
	{
		uint8_t sequence0;
		uint8_t sequence1;
		std::list< ControlEnvelope > sent;
		CircularVector< uint8_t, 16 > last0;
		CircularVector< uint8_t, 16 > last1;
		bool alive;
		bool faulty;

		Control()
		:	sequence0(0)
		,	sequence1(0)
		,	alive(false)
		,	faulty(false)
		{
		}
	};

	Ref< IReplicatorPeers > m_peers;
	Timer m_timer;
	std::vector< PeerInfo > m_info;
	std::map< handle_t, Control > m_control;
};

	}
}

#endif	// traktor_net_ReliableTransportPeers_H
