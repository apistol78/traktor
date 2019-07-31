#include "Core/Thread/Atomic.h"
#include "Spark/Types.h"

namespace traktor
{
	namespace spark
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
