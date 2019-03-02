#include <Windows.h>
#include "Input/Win32/TypesWin32.h"

namespace traktor
{
	namespace input
	{

uint32_t translateFromVk(uint32_t vk)
{
	for (uint32_t i = 0; i < sizeof_array(c_vkControlKeys); ++i)
	{
		if (c_vkControlKeys[i] == vk)
			return i;
	}
	return 0;
}

	}
}
