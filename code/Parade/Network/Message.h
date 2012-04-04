#ifndef traktor_parade_Message_H
#define traktor_parade_Message_H

#include "Core/Config.h"

namespace traktor
{
	namespace parade
	{

enum MessageType
{
	MtWho = 0xf0,
	MtIAm = 0xf1,
	MtState	= 0x11,
	MtEvent = 0x12
};

#pragma pack(1)
struct Message
{
	uint8_t type;
	uint32_t time;
	uint8_t data[1200 - 1 - sizeof(float)];
};
#pragma pack()

	}
}

#endif	// traktor_parade_Message_H
