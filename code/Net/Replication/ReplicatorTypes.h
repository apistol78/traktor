#ifndef traktor_net_ReplicatorTypes_H
#define traktor_net_ReplicatorTypes_H

#include "Core/Config.h"

namespace traktor
{
	namespace net
	{

enum RMessageId
{
	RmiPing	= 0x10,
	RmiPong = 0x11,
	RmiState = 0x20,
	RmiEvent = 0x30,
	RmiEventAck = 0x40
};

#pragma pack(1)
struct RMessage
{
	uint8_t id;
	uint32_t time;
	union
	{
		struct
		{
			uint8_t status;
		} ping;

		struct 
		{
			uint32_t time0;
			uint32_t latency;
		} pong;

		struct  
		{
			uint8_t data[1];
		} state;

		struct  
		{
			uint8_t sequence;
			uint8_t data[1];
		} event;

		struct 
		{
			uint8_t sequence;
		} eventAck;

		uint8_t reserved[1024 - sizeof(uint8_t) - sizeof(uint32_t)];
	};
};
#pragma pack()

T_FORCE_INLINE int32_t RMessage_HeaderSize()				{ return sizeof(uint8_t) + sizeof(uint32_t); }

T_FORCE_INLINE int32_t RmiPing_NetSize()					{ return RMessage_HeaderSize() + sizeof(uint8_t); }
T_FORCE_INLINE int32_t RmiPong_NetSize()					{ return RMessage_HeaderSize() + sizeof(uint32_t) + sizeof(uint32_t); }

T_FORCE_INLINE int32_t RmiState_NetSize(int32_t stateSize)	{ return RMessage_HeaderSize() + stateSize; }
T_FORCE_INLINE int32_t RmiState_StateSize(int32_t netSize)	{ return netSize - RMessage_HeaderSize(); }
T_FORCE_INLINE int32_t RmiState_MaxStateSize()				{ return RmiState_StateSize(1024); }

T_FORCE_INLINE int32_t RmiEvent_NetSize(int32_t eventSize)	{ return RMessage_HeaderSize() + sizeof(uint8_t) + eventSize; }
T_FORCE_INLINE int32_t RmiEvent_EventSize(int32_t netSize)	{ return netSize - RMessage_HeaderSize() - sizeof(uint8_t); }
T_FORCE_INLINE int32_t RmiEvent_MaxEventSize()				{ return RmiEvent_EventSize(1024); }

T_FORCE_INLINE int32_t RmiEventAck_NetSize()				{ return RMessage_HeaderSize() + sizeof(uint8_t); }

T_FORCE_INLINE uint32_t time2net(double time)	{ return uint32_t(time * 1000.0 + 0.5); }
T_FORCE_INLINE double net2time(uint32_t time)	{ return time / 1000.0; }

	}
}

#endif	// traktor_net_ReplicatorTypes_H
