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
	MtThrottle = 0xf5,
	MtDisconnect = 0xf6,
	MtFullState	= 0x11,
	MtDeltaState = 0x12,
	MtEvent = 0x13
};

#pragma pack(1)
struct Message
{
	enum
	{
		HeaderSize = sizeof(uint8_t) + sizeof(uint32_t),
		MessageSize = 1200,
		StateSize = MessageSize - HeaderSize,
		EventSize = MessageSize - HeaderSize
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
			uint8_t data[StateSize];
		} state;

		struct
		{
			uint8_t data[EventSize];
		} event;

		uint8_t reserved[MessageSize - HeaderSize];
	};
};
#pragma pack()

	}
}

#endif	// traktor_net_Message_H
