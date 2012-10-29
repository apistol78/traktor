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
	MtFullState	= 0x11,
	MtDeltaState = 0x12,
	MtEvent = 0x13
};

#pragma pack(1)
struct Message
{
	enum { DataSize = 1200 - 1 - sizeof(uint32_t) };

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

		uint8_t data[DataSize];
	};
};
#pragma pack()

	}
}

#endif	// traktor_parade_Message_H
