#ifndef traktor_net_Message_H
#define traktor_net_Message_H

#include "Core/Config.h"

namespace traktor
{
	namespace net
	{

enum MessageType
{
	MtIAm = 0xf1,
	MtBye = 0xf2,
	MtPing = 0xf3,
	MtPong = 0xf4,
	
	MtFullState	= 0x11,
	MtDeltaState = 0x12,

	MtEvent0 = 0x20,	//!< Never transmitted
	MtEvent1 = 0x21,
	MtEvent2 = 0x22,
	MtEvent3 = 0x23,
	MtEvent4 = 0x24,

	MtRelayUnreliable1 = 0x81,
	MtRelayUnreliable2 = 0x82,

	MtRelayReliable1 = 0x83,
	MtRelayReliable2 = 0x84,

	MtMasquerade = 0x90
};

#pragma pack(1)
struct Message
{
	enum
	{
		HeaderSize = sizeof(uint8_t) + sizeof(uint32_t),
		MessageSize = 510,
		DataSize = MessageSize - HeaderSize - 2 * sizeof(uint64_t)
	};

	uint8_t type;
	uint32_t time;
	union
	{
		struct
		{
			uint8_t sequence;
			uint32_t id;
		} iam;

		struct 
		{
			uint32_t time0;
			uint32_t latency;
		} pong;

		struct
		{
			uint64_t globalId;
		} disconnect;

		struct
		{
			uint8_t data[DataSize];
		} state;

		struct
		{
			uint8_t data[DataSize];
		} event;

		struct 
		{
			uint64_t fromGlobalId;
			uint64_t targetGlobalId;
			uint8_t data[DataSize];
		} relay;

		struct 
		{
			uint64_t fromGlobalId;
			uint8_t data[DataSize];
		}
		masquerade;

		uint8_t reserved[MessageSize - HeaderSize];
	};
};
#pragma pack()

	}
}

#endif	// traktor_net_Message_H
