#include "Core/Thread/Atomic.h"
#include "Flash/FlashTypes.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

int32_t s_tag = 0;

		}

int32_t allocateCacheTag()
{
	return Atomic::increment(s_tag);
}

	}
}
