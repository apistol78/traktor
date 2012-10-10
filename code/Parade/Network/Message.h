#ifndef traktor_parade_Message_H
#define traktor_parade_Message_H

#include "Core/Config.h"

namespace traktor
{
	namespace parade
	{

enum MessageType
{
	MtIAm = 0xf1,
	MtBye = 0xf2,
	MtPing = 0xf3,
	MtPong = 0xf4,
	MtState	= 0x11,
	MtEvent = 0x12
};

#pragma pack(1)
struct Message
{
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

		uint8_t data[1200 - 1 - sizeof(uint32_t)];
	};
};
#pragma pack()

	}
}

#endif	// traktor_parade_Message_H
